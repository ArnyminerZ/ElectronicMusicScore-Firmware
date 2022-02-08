#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "mbedtls/md.h"
#include "webpages.h"
#include "logger_levels.h"

#define FIRMWARE_VERSION "v0.0.1"

#define DEBUG_LEVEL DEBUG_LOG

#include "logger.h"

/**
 * @brief The amount of time that will take a token to expire, in seconds.
 */
#define SESSION_EXPIRATION_TIME_SECONDS 60 * 60

const String default_ssid = "DIGIFIBRA-dZSX";
const String default_wifipassword = "t2SPCE7yHs";
const String default_httpuser = "admin";
const String default_httppassword = "admin";
const int default_webserverporthttp = 80;

/**
 * @author Arnau Mora
 * @since 20220208
 * @brief The name of the preferences storage.
 */
const char *preferencesName = "elec-score";
/**
 * @brief The preferences key for storing the amount of sessions stored.
 */
const char *pref_sessionCount = "sess-count";
/**
 * @brief The preferences key for storing the prefix for all the sessions.
 */
const String pref_sessionPrefix = "sess-u";
/**
 * @brief The preferences key for storing the prefix for all the sessions' creation date.
 */
const String pref_sessionExpPrefix = "sess-e";

/**
 * @brief The NTP server for getting the current time.
 */
const char *ntpServer = "time.google.com";

// configuration structure
struct Config
{
  String ssid;           // wifi ssid
  String wifipassword;   // wifi password
  String httpuser;       // username to access web admin
  String httppassword;   // password to access web admin
  int webserverporthttp; // http port number for web admin
};

// For storing key-value data
Preferences preferences;

// variables
Config config;             // configuration
bool shouldReboot = false; // schedule a reboot
AsyncWebServer *server;    // initialise webserver

// function defaults
String listFiles(bool ishtml = false);

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes)
{
  if (bytes < 1024)
    return String(bytes) + " B";
  else if (bytes < (1024 * 1024))
    return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024))
    return String(bytes / 1024.0 / 1024.0) + " MB";
  else
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String hash(const char *payload)
{
  const size_t payloadLength = strlen(payload);
  byte shaResult[payloadLength];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  String builder = "";
  for (int i = 0; i < sizeof(shaResult); i++)
  {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    builder += str;
  }

  return builder;
}

/**
 * @brief Reboots the device.
 *
 * @param message The reboot message to send through serial before rebooting. AKA Reboot reason.
 */
void rebootESP(String message)
{
  warn("Rebooting ESP32: ");
  warnln(message);
  ESP.restart();
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
/**
 * @brief Gets some data according to [var].
 *
 * @param var The data to get. Can be:
 * - FIRMWARE: Returns the Firmware version
 * - FREESPIFFS: Returns the free SPIFFS memory
 * - USEDSPIFFS: Returns the used SPIFFS memory
 * - TOTALSPIFFS: Returns the total available SPIFFS memory
 * @return String
 */
String processor(const String &var)
{
  String result = "N/A";

  if (var == "FIRMWARE")
    result = FIRMWARE_VERSION;
  else if (var == "FREESPIFFS")
    result = humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
  else if (var == "USEDSPIFFS")
    result = humanReadableSize(SPIFFS.usedBytes());
  else if (var == "TOTALSPIFFS")
    result = humanReadableSize(SPIFFS.totalBytes());

  return result;
}

/**
 * @brief Send the 404 error page through [request].
 *
 * @param request The [AsyncWebServerRequest] to give the answer.
 */
void notFound(AsyncWebServerRequest *request)
{
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  infoln(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest *request)
{
  debugln("Checking if user is authenticated...");
  debug("  Checking if there's cookie header...");
  if (request->hasHeader("Cookie"))
  {
    // The request has cookies stored.
    debugln("ok");
    debug("  Getting cookie header...");

    // Get the cookies header
    AsyncWebHeader *cookie = request->getHeader("Cookie");
    String cookieValue = cookie->value();
    debugln("ok");
    debug("  - Cookie header: ");
    debugln(cookieValue);

    // Find the SESSIONID value
    debug("  Checking if cookie has SESSIONID...");
    if (cookieValue.indexOf("SESSIONID=") != -1) // There's an stored session id
    {
      debugln("ok");

      // Get the user's date for expired session ids
      debug("  Cookie has SESSIONID. Getting browser User-Agent...");
      AsyncWebHeader *agentHeader = request->getHeader("User-Agent");
      String agent = agentHeader->value();
      debugln("ok");
      debug("  - User-Agent header: ");
      debugln(agent);
      debug("  Hashing user agent...");
      String agentHash = hash(agent.c_str()); // Create hash for the User Agent
      debugln("ok");
      debug("  - User Hash: ");
      debugln(agentHash);

      // Get the current time for token expiration
      debug("  Getting device time...");
      struct tm time;
      unsigned long currentTime = -1;
      if (getLocalTime(&time))
      {
        debugln("ok");
        currentTime = time.tm_sec + time.tm_min * (60) + time.tm_hour * (60 * 60) + time.tm_mday * (60 * 60 * 24) + time.tm_mon * (60 * 60 * 24 * 30) + time.tm_year * (60 * 60 * 24 * 365);
        debug("  - Current device time: ");
        debugln(String(currentTime));
      }
      else
      {
        debugln("error!");
        errln("ERROR! Could not get time info. Tokens won't expire.");
      }

      // Get the amount of stored sessions
      debug("  Getting sessions count...");
      unsigned int sessionsCount = preferences.getUInt(pref_sessionCount, 0U);
      debugln("ok");
      debug("  - Sessions count: ");
      debugln(String(sessionsCount));

      // Iterate that much of times
      debugln("  Iterating sessions...");
      for (int c = 0; c < sessionsCount; c++)
      {
        debug("    Getting session at ");
        debug(String(c));
        debug("...");

        // Get the stored session id on that index
        String sessionId = preferences.getString((pref_sessionPrefix + String(c)).c_str());
        debugln("ok");
        debug("    - Session id: ");
        debugln(sessionId);

        // Ignore all empty sets
        if (sessionId.length() <= 0)
        {
          debugln("    Session id is empty. Jumping to next session.");
          continue;
        }

        // Check if the found sessionId matches the User Agent hash
        debug("    Checking if agent hash is correct...");
        if (sessionId == agentHash) // Hash is correct
        {
          debugln("ok");

          // There's no valid stored time, skip expiration check
          if (currentTime < 0)
          {
            debugln("      System time could not be loaded. Expiration is disabled.");
            // TODO: Return result codes instead of boolean
            return true;
          }

          // Check if not expired
          debugln("      Checking if session has expired.");
          debug("      Getting session creation date...");
          unsigned int sessionCreation = preferences.getUInt((pref_sessionExpPrefix + String(c)).c_str());
          debugln("ok");

          unsigned int difference = currentTime - sessionCreation;
          if (difference > SESSION_EXPIRATION_TIME_SECONDS)
          {
            // Token has expired
            debugln("      Session is expired.");
            debug("      Session age is: ");
            debugln(String(sessionCreation));
            debug("      And current time is: ");
            debugln(String(currentTime));
            debug("      Which difference is: ");
            debugln(String(difference));
            debug("      That is greater from the configured max age: ");
            debugln(String(SESSION_EXPIRATION_TIME_SECONDS));

            debugln("      Removing expired session...");
            debug("        Reducing sessions count to ");
            sessionsCount--;
            debug(String(sessionsCount));
            debug("...");
            preferences.putUInt(pref_sessionCount, sessionsCount);
            debugln("ok");
            // Move all remaining sessions one position to the left
            for (int i = c; i < sessionsCount; i++)
            {
              debug("        - Displacing session id at ");
              debug(String(i + 1));
              debug("...");
              String oldSessionId = preferences.getString((pref_sessionPrefix + String(i + 1)).c_str());
              preferences.putString((pref_sessionPrefix + String(i)).c_str(), oldSessionId);
              debugln("ok");

              debug("        - Displacing session expiration at ");
              debug(String(i + 1));
              debug("...");
              unsigned int sessionCreation = preferences.getUInt((pref_sessionExpPrefix + String(i + 1)).c_str());
              preferences.putUInt((pref_sessionExpPrefix + String(i)).c_str(), sessionCreation);
              debugln("ok");
            }
            continue;
          }

          debugln("      Session not expired, returning ok.");
          // TODO: Return result codes instead of boolean
          return true;
        }
        else
          debugln("no");
      }
    }
    else
      debugln("no");
  }
  else
    debugln("no");

  return false;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request))
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index)
    {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SPIFFS.open("/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len)
    {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final)
    {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  }
  else
  {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}

void configureWebServer()
{
  // configure web server

  // if url isn't found
  server->onNotFound(notFound);

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);

  // visiting this page will cause you to be logged out
  server->on(
      "/logout",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        // Clears cookies
        AsyncWebServerResponse *response = request->beginResponse(401);
        response->addHeader("Cookie", String());
        request->send(response);
      });

  // presents a "you are now logged out webpage
  server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request)
             {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    info(logmessage);
    request->send_P(401, "text/html", logout_html, processor); });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               String logmessage = "Client:" + request->client()->remoteIP().toString() + +" " + request->url();

               if (checkUserWebAuth(request))
               {
                 logmessage += " Auth: Success";
                 info(logmessage);
                 request->send_P(200, "text/html", index_html, processor);
               }
               else
               {
                 logmessage += " Auth: Failed";
                 info(logmessage);
                 request->send_P(200, "text/html", login_html, processor);
               } });

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
             {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (checkUserWebAuth(request)) {
      request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      info(logmessage);
      shouldReboot = true;
    } else {
      logmessage += " Auth: Failed";
      info(logmessage);
                 request->send_P(200, "text/html", login_html, processor);
    } });

  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
             {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      info(logmessage);
      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      info(logmessage);
                 request->send_P(200, "text/html", login_html, processor);
    } });

  server->on("/file", HTTP_GET, [](AsyncWebServerRequest *request)
             {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      info(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();

        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!SPIFFS.exists(fileName)) {
          info(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        } else {
          info(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(SPIFFS, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            SPIFFS.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          } else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          info(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      info(logmessage);
      request->send_P(200, "text/html", login_html, processor);
    } });

  // Process a login request
  server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request)
             {
               String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
               int paramsCount = request->params();
               logmessage += " Params (" + String(paramsCount) + "){";
               for (int i = 0; i < paramsCount; i++)
               {
                 AsyncWebParameter *p = request->getParam(i);
                 if (p->isFile())
                 {
                   const char *name = p->name().c_str();
                   const char *value = p->value().c_str();
                   const char *size = String(p->size()).c_str();
                   char buffer[24 + strlen(name) + strlen(value) + strlen(size)];
                   sprintf(buffer, "_FILE[%s]: %s, size: %u, ", p->name().c_str(), p->value().c_str(), p->size());
                   logmessage += String(buffer);
                 }
                 else if (p->isPost())
                 {
                   const char *name = p->name().c_str();
                   const char *value = p->value().c_str();
                   char buffer[24 + strlen(name) + strlen(value)];
                   sprintf(buffer,"%s: %s, ", p->name().c_str(), p->value().c_str());
                   logmessage += String(buffer);
                 }
                 else
                 {
                   const char *name = p->name().c_str();
                   const char *value = p->value().c_str();
                   char buffer[24 + strlen(name) + strlen(value)];
                   sprintf(buffer,"_GET[%s]: %s, ", p->name().c_str(), p->value().c_str());
                   logmessage += String(buffer);
                 }
               }
               info(logmessage);
             });
}

void setup()
{
  Serial.begin(115200);

  Serial.print("Firmware: ");
  Serial.println(FIRMWARE_VERSION);

  Serial.println("Booting ...");

  Serial.println("Initializing preferences...");
  preferences.begin(preferencesName, false);

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true))
  {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    Serial.println("ERROR: Cannot mount SPIFFS, Rebooting");
    rebootESP("ERROR: Cannot mount SPIFFS, Rebooting");
  }

  Serial.print("SPIFFS Free: ");
  Serial.println(humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  Serial.print("SPIFFS Used: ");
  Serial.println(humanReadableSize(SPIFFS.usedBytes()));
  Serial.print("SPIFFS Total: ");
  Serial.println(humanReadableSize(SPIFFS.totalBytes()));

  Serial.println(listFiles());

  Serial.println("Loading Configuration ...");
  config.ssid = default_ssid;
  config.wifipassword = default_wifipassword;
  config.httpuser = default_httpuser;
  config.httppassword = default_httppassword;
  config.webserverporthttp = default_webserverporthttp;

  Serial.print("\nConnecting to Wifi: ");
  WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("ok");

  Serial.println("\nNetwork Configuration:");
  Serial.println("----------------------");
  Serial.print("         SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("  Wifi Status: ");
  Serial.println(WiFi.status());
  Serial.print("Wifi Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("          MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("           IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("       Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("      Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("        DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("        DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
  Serial.print("        DNS 3: ");
  Serial.println(WiFi.dnsIP(2));
  Serial.println();

  // Update time
  // 3600 is for Spain timezone (+1h=3600s)
  Serial.print("Configuring time...");
  configTime(0, 3600, ntpServer);
  Serial.println("ok");

  // configure web server
  Serial.print("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();
  Serial.println("ok");

  // startup web server
  Serial.print("Starting Webserver ...");
  server->begin();
  Serial.println("ok");
}

void loop()
{
  // reboot if we've told it to reboot
  if (shouldReboot)
  {
    rebootESP("Web Admin Initiated Reboot");
  }
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml)
{
  String returnText = "";
  Serial.println("Listing files stored on SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml)
  {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile)
  {
    if (ishtml)
    {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    }
    else
    {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml)
  {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

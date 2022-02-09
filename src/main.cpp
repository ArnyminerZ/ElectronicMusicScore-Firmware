// External libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>

// Internal utilities files
#include "hash.h"

// Constants files
#include "logger_levels.h"
#include "pref_consts.h"
#include "consts_wifi.h"
#include "consts_net.h"

// General definitions
#define FIRMWARE_VERSION "v0.0.1"

#define DEBUG_LEVEL DEBUG_LOG

// For logging messages into serial
#include "logger.h"

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

void setup()
{
  Serial.begin(115200);

  info("Firmware: ");
  infoln(FIRMWARE_VERSION);

  infoln("Booting ...");

  info("Initializing preferences...");
  preferences.begin(preferencesName, false);
  infoln("ok");

  info("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true))
  {
    infoln("error!");
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    errln("ERROR: Cannot mount SPIFFS, Rebooting");
    rebootESP("ERROR: Cannot mount SPIFFS, Rebooting");
    return;
  }
  infoln("ok");

  info("SPIFFS Free: ");
  infoln(humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  info("SPIFFS Used: ");
  infoln(humanReadableSize(SPIFFS.usedBytes()));
  info("SPIFFS Total: ");
  infoln(humanReadableSize(SPIFFS.totalBytes()));

  infoln(listFiles());

  infoln("Loading Configuration ...");
  config.ssid = preferences.getString(pref_wifiSsid, WIFI_DEFAULT_SSID);
  config.wifipassword = preferences.getString(pref_wifiPass, WIFI_DEFAULT_PASS);
  config.httpuser = preferences.getString(pref_authUser, AUTH_DEFAULT_USER);
  config.httppassword = preferences.getString(pref_authPass, AUTH_DEFAULT_PASS);
  config.webserverporthttp = WEB_PORT;

  info("\nConnecting to Wifi (");
  info(config.ssid);
  info(")");
  WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    info(".");
  }
  infoln("ok");

  infoln("\nNetwork Configuration:");
  infoln("----------------------");
  info("         SSID: ");
  infoln(WiFi.SSID());
  info("  Wifi Status: ");
  infoln(String(WiFi.status()));
  info("Wifi Strength: ");
  info(String(WiFi.RSSI()));
  infoln(" dBm");
  info("          MAC: ");
  infoln(String(WiFi.macAddress()));
  info("           IP: ");
  infoln(String(WiFi.localIP()));
  info("       Subnet: ");
  infoln(String(WiFi.subnetMask()));
  info("      Gateway: ");
  infoln(String(WiFi.gatewayIP()));
  info("        DNS 1: ");
  infoln(String(WiFi.dnsIP(0)));
  info("        DNS 2: ");
  infoln(String(WiFi.dnsIP(1)));
  info("        DNS 3: ");
  infoln(String(WiFi.dnsIP(2)));
  infoln(String());

  // Update time
  // Defaults to 3600  for Spain timezone (+1h=3600s)
  info("Configuring time...");
  int daylightOffset = preferences.getInt(pref_timezone, 3600);
  configTime(0, daylightOffset, ntpServer);
  infoln("ok");

  // configure web server
  info("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer(server);
  infoln("ok");

  // startup web server
  info("Starting Webserver ...");
  server->begin();
  infoln("ok");
}

void loop()
{
  // reboot if we've told it to reboot
  if (shouldReboot)
    rebootESP("Web Admin Initiated Reboot");

  delay(10);
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

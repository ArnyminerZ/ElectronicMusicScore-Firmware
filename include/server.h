
#ifndef SERVER_H
#define SERVER_H

// Include libraries
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Include utils file
#include "logger.h"
#include "auth.h"
#include "utils.h"
#include "filesystem.h"
#include "hash.h"
#include "musicxml.h"

// Include webpages data
#include "webpages.h"

// Include constants
#include "consts_net.h"

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
    String result = "";

    if (var == "FIRMWARE")
        result = FIRMWARE_VERSION;
    else if (var == "FREESPIFFS")
        result = humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
    else if (var == "USEDSPIFFS")
        result = humanReadableSize(SPIFFS.usedBytes());
    else if (var == "TOTALSPIFFS")
        result = humanReadableSize(SPIFFS.totalBytes());
    else if (var == "USEDSPIFFS_INT")
        result = String(SPIFFS.usedBytes());
    else if (var == "TOTALSPIFFS_INT")
        result = String(SPIFFS.totalBytes());
    else if (var == "AUTH_SESSIONS")
    {
        unsigned int sessionsCount = preferences.getUShort(pref_sessionCount, 0U);
        result = String(sessionsCount) + "^";
        for (int c = 0; c < sessionsCount; c++)
        {
            String sessionId = preferences.getString((String(pref_sessionPrefix) + String(c)).c_str());
            unsigned int sessionCreation = preferences.getULong((String(pref_sessionExpPrefix) + String(c)).c_str());
            result += sessionId + "," + String(sessionCreation) + ";";
        }
    }

    return result;
}

/**
 * @brief Handles uploading to the server.
 *
 * @param request The AsyncWebServerRequest for giving answer.
 * @param filename The filename for uploading.
 * @param index The index of the file.
 * @param data All the bytes of data to write.
 * @param len The length of the files.
 */
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

/**
 * @brief Adds all the handlers for the server.
 */
void configureWebServer(AsyncWebServer *server, boolean *shouldReboot)
{
    // configure web server

    // if url isn't found
    server->onNotFound(notFound);

    // run handleUpload function when any file is uploaded
    server->onFileUpload(handleUpload);

    // The file for styles
    server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send_P(200, "text/css", styles_css, processor); });

    // The file for scripts
    server->on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send_P(200, "application/javascript", scripts_js, processor); });

    // visiting this page will cause you to be logged out
    server->on(
        "/logout",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            // Clears cookies
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html", logout_html);
            response->addHeader("Set-Cookie", "SESSIONID=; Max-Age=-1");
            request->send(response);
        });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + +" " + request->url();

        if (checkUserWebAuth(request)) {
            logmessage += " Auth: Success";
            infoln(logmessage);
            request->send_P(200, "text/html", index_html, processor);
        } else {
            logmessage += " Auth: Failed";
            infoln(logmessage);
            request->send_P(200, "text/html", login_html, processor);
        } });

    server->on("/reboot", HTTP_GET, [shouldReboot](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

        if (checkUserWebAuth(request)) {
            request->send(200, "text/html", reboot_html);
            logmessage += " Auth: Success";
            infoln(logmessage);
            *shouldReboot = true;
        } else {
            logmessage += " Auth: Failed";
            infoln(logmessage);
            request->send_P(200, "text/html", login_html, processor);
        } });

    server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
        if (checkUserWebAuth(request)) {
            logmessage += " Auth: Success";
            infoln(logmessage);
            request->send(200, "text/plain", listFiles(true));
        } else {
            logmessage += " Auth: Failed";
            infoln(logmessage);
            request->send_P(200, "text/html", login_html, processor);
        } });

    server->on("/loadxml", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
        if (checkUserWebAuth(request)) {
            logmessage += " Auth: Success";
            infoln(logmessage);
            if (request->hasParam("path"))
            {
                AsyncWebParameter *path = request->getParam("path");
                loadMusic(path->value());
                request->send(200, "text/plain", "See log");
            } else
                request->send(500, "text/plain", "Path parameter not found.");
        } else {
            logmessage += " Auth: Failed";
            infoln(logmessage);
            request->send_P(200, "text/html", login_html, processor);
        } });

    server->on("/file", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
        if (checkUserWebAuth(request)) {
            logmessage += " Auth: Success";
            infoln(logmessage);

            if (request->hasParam("name") && request->hasParam("action")) {
                const char *fileName = request->getParam("name")->value().c_str();
                const char *fileAction = request->getParam("action")->value().c_str();

                logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" +
                             String(fileName) + "&action=" + String(fileAction);

                if (!SPIFFS.exists(fileName)) {
                    infoln(logmessage + " ERROR: file does not exist");
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
                    infoln(logmessage);
                }
            } else {
                request->send(400, "text/plain", "ERROR: name and action params required");
            }
        } else {
            logmessage += " Auth: Failed";
            infoln(logmessage);
            request->send_P(200, "text/html", login_html, processor);
        } });

    // Process a login request
    server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
        int paramsCount = request->params();
        logmessage += " Params (" + String(paramsCount) + "){";

        String username = String();
        String password = String();

        for (int i = 0; i < paramsCount; i++) {
            AsyncWebParameter *p = request->getParam(i);

            String strName = p->name();
            String strValue = p->value();
            const char *name = strName.c_str();
            const char *value = strValue.c_str();

            if (strName == "username")
                username = strValue;
            else if (strName == "password")
                password = strValue;

            logmessage += strName + "=" + strValue + ", ";
        }
        logmessage += "}";

        if (username.length() > 0 && password.length() > 0)
        {
            String correctUsername = preferences.getString(pref_authUser, AUTH_DEFAULT_USER);
            String correctPassword = preferences.getString(pref_authPass, AUTH_DEFAULT_PASS);

            if (username == correctUsername && password == correctPassword)
            {
                logmessage += ". Auth OK.";

                // Create session
                unsigned int sessionsCount = preferences.getUShort(pref_sessionCount, 0);

                struct tm time;
                unsigned long currentTime = -1;
                if (getLocalTime(&time))
                    currentTime = time.tm_sec + time.tm_min * (60) + time.tm_hour * (60 * 60) + time.tm_mday * (60 * 60 * 24) + time.tm_mon * (60 * 60 * 24 * 30) + time.tm_year * (60 * 60 * 24 * 365);

                AsyncWebHeader* agentHeader = request->getHeader("User-Agent");
                String userAgent = agentHeader->value();
                String userHash = hash(userAgent.c_str());
                preferences.putString((String(pref_sessionPrefix) + String(sessionsCount)).c_str(), userHash);
                preferences.putULong((String(pref_sessionExpPrefix) + String(sessionsCount)).c_str(), currentTime);

                preferences.putUShort(pref_sessionCount, ++sessionsCount);

                AsyncWebServerResponse *response = request->beginResponse(303);
                response->addHeader("Set-Cookie", "SESSIONID=" + userHash + "; Max-Age=" + String(SESSION_EXPIRATION_TIME_SECONDS / 1000));
                response->addHeader("Location", "/");
                request->send(response);
            }
            else
            {
                logmessage += ". Auth NO.";
                // TODO: Failed login should give the user a message
                request->redirect("/");
            }
            infoln(logmessage);
            return;
        }
        else logmessage += ". No auth parameters.";

        // TODO: Failed login should give the user a message
        request->redirect("/");
        infoln(logmessage); });

// Requests for debug mode
#ifdef DEBUG_MODE
    server->on(
        "/clear-auth",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            debugln("Clearing auth sessions...");
            debug("Setting session count to 0...");
            preferences.putUShort(pref_sessionCount, 0U);
            debugln("ok");

            request->redirect("/");
        });
#endif
}

#endif
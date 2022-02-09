
#ifndef SERVER_H
#define SERVER_H

// Include libraries
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Include utils file
#include "logger.h"
#include "auth.h"

// Include webpages data
#include "webpages.h"

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
void configureWebServer(AsyncWebServer *server)
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
               info(logmessage); });
}

#endif
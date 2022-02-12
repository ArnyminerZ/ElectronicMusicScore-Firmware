
// General definitions
#define FIRMWARE_VERSION "v0.0.1"

#define DEBUG_LEVEL DEBUG_LOG

// Comment to disable debug mode
#define DEBUG_MODE

// External libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <AsyncElegantOTA.h>

// Internal utilities files
#include "hash.h"
#include "filesystem.h"

// Constants files
#include "logger_levels.h"
#include "pref_consts.h"
#include "consts_wifi.h"
#include "consts_net.h"

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
unsigned long blockingRequestTime = 0;
IPAddress apIP(8, 8, 4, 4); // The default android DNS
DNSServer dnsServer;

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
  blockingRequestTime = millis();
  int timeout = preferences.getInt(pref_wifiTimeout, WIFI_TIMEOUT_DEFAULT);
  while (WiFi.status() != WL_CONNECTED)
  {
    // If timed-out, break from while.
    if (millis() - blockingRequestTime >= timeout)
      break;

    delay(500);
    info(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
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
    infoln(WiFi.macAddress());
    info("           IP: ");
    infoln(WiFi.localIP());
    info("       Subnet: ");
    infoln(WiFi.subnetMask());
    info("      Gateway: ");
    infoln(WiFi.gatewayIP());
    info("        DNS 1: ");
    infoln(WiFi.dnsIP(0));
    info("        DNS 2: ");
    infoln(WiFi.dnsIP(1));
    info("        DNS 3: ");
    infoln(WiFi.dnsIP(2));
    infoln(String());
  }
  else
  {
    infoln("timeout!");

    info("Setting up AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-DNSServer");
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    infoln("ok");

    info("Setting up DNS server...");
    dnsServer.start(DNS_PORT, "*", apIP);
    infoln("ok");
  }

  // Update time
  // Defaults to 3600  for Spain timezone (+1h=3600s)
  info("Configuring time...");
  int daylightOffset = preferences.getInt(pref_timezone, 3600);
  configTime(0, daylightOffset, ntpServer);
  infoln("ok");

  // configure web server
  info("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer(server, &shouldReboot);
  infoln("ok");

  info("Starting OTA...");
  AsyncElegantOTA.begin(server, config.httpuser.c_str(), config.httppassword.c_str());
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

  if (WiFi.getMode() == WIFI_AP)
    dnsServer.processNextRequest();

  delay(10);
}

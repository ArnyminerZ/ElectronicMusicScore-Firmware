/**
 * @file auth.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Provides the functions required for authenticating the user.
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef AUTH_H
#define AUTH_H

// Include libraries
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

// Include utils files
#include "logger.h"
#include "pref_consts.h"
#include "hash.h"

/**
 * @brief The amount of time that will take a token to expire, in seconds.
 */
#define SESSION_EXPIRATION_TIME_SECONDS 60 * 60

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

#endif

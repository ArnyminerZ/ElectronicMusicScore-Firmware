/**
 * @file config.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Provides functions for helping out on configuration storage.
 * @version 0.1
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

// Include libraries
#include <Arduino.h>
#include <Preferences.h>

// Include cpp headers
#include <iostream>
#include <sstream>

// Include header files
#include "pref_consts.h"
#include "consts_err.h"
#include "logger.h"

// Define config keys
/**
 * @brief Used to delete auth sessions. The value must be a numeric value specifying the index of the session to remove.
 */
#define CONFIG_KEY_REMOVE_SESSION "delSession"

bool isNumber(const std::string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

/**
 * @brief Sets the specified [key] to value [value].
 *
 * @param key The config key to update.
 * @param value The value to set
 */
const char *configure(std::string key, std::string value)
{
    // Check if key is CONFIG_KEY_REMOVE_SESSION
    if (key.rfind(CONFIG_KEY_REMOVE_SESSION) != std::string::npos)
    {
        debugln("Received configure parameter CONFIG_KEY_REMOVE_SESSION.");

        // CONFIG_KEY_REMOVE_SESSION requires value to be numeric
        if (!isNumber(value))
        {
            debug("The received value \"");
            debug(value.c_str());
            debugln("\" is not a numeric value");
            return ERR_CONFIG_NUMERIC;
        }

        debug("Converting value to unsigned short...");
        // Convert value to short to use as index
        std::stringstream strVal;
        strVal << value;
        unsigned short index;
        strVal >> index;
        debugln("ok");

        // Get the amount of stored sessions
        debug("Getting sessionsCount...");
        unsigned short sessionsCount = preferences.getUShort(pref_sessionCount, 0U);
        debugln("ok");
        debug("  sessionsCount=");
        debugln(String(sessionsCount));

        if (index >= sessionsCount)
        {
            debugln("The index specified is greater than the sessionsCount.");
            return ERR_CONFIG_BOUNDS;
        }

        // Iterate all sessions following the set index, and move them one position left
        for (unsigned short i = index; i < sessionsCount; i++)
        {
            debug("Getting sessionId and sessionCreation at ");
            debug(String(i + 1));
            debug("...");
            // Get the session id
            String sessionId = preferences.getString((String(pref_sessionPrefix) + String(i + 1)).c_str());
            // Get the session creation id
            unsigned int sessionCreation = preferences.getULong((String(pref_sessionExpPrefix) + String(i + 1)).c_str());
            debugln("ok");

            debug("Writing both elements at ");
            debug(String(i));
            debug("...");
            preferences.putString((String(pref_sessionPrefix) + String(i)).c_str(), sessionId);
            preferences.putULong((String(pref_sessionExpPrefix) + String(i)).c_str(), sessionCreation);
            debugln("ok");
        }

        // Reduce by 1 the count at sessionsCount
        sessionsCount--;

        // Update the value for sessionsCount
        debug("Writing ");
        debug(String(sessionsCount));
        debug(" for sessionsCount...");
        preferences.putUShort(pref_sessionCount, sessionsCount);
        debugln("ok");

        return CONFIG_OK;
    }

    debug("Got invalid key for config: ");
    debugln(key.c_str());

    return ERR_CONFIG_KEY;
}

#endif
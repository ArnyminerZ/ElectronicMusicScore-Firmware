/**
 * @file preferences.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Contains constants regarding the preferences storage.
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef PREF_CONSTS_H
#define PREF_CONSTS_H

// For storing key-value data
Preferences preferences;

/**
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
const char *pref_sessionPrefix = "sess-u";

/**
 * @brief The preferences key for storing the prefix for all the sessions' creation date.
 */
const char *pref_sessionExpPrefix = "sess-e";

/**
 * @brief The preferences key for storing the SSID of the WiFi network to connect to.
 */
const char *pref_wifiSsid = "wifi-ssid";
/**
 * @brief The preferences key for storing the Password of the WiFi network to connect to.
 * @see pref_wifiSsid
 */
const char *pref_wifiPass = "wifi-pass";

/**
 * @brief The preferences key for storing the Username that is allowed to access the web UI.
 * @see pref_authPass
 */
const char *pref_authUser = "auth-user";

/**
 * @brief The preferences key for storing the Password of the user that is allowed to access the web UI.
 */
const char *pref_authPass = "auth-pass";

/**
 * @brief the preferences key for storing the Timezone preference of the user.
 */
const char *pref_timezone = "tz";

#endif
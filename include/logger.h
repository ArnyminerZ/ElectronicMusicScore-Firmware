#include <Arduino.h>
#include "logger_levels.h"

#ifndef LOGGER_H
#define LOGGER_H

#ifndef DEBUG_LEVEL
#warning "DEBUG_LEVEL not defined, defaulting to DEBUG_LOG."

#define DEBUG_LEVEL DEBUG_LOG
#endif

#pragma region "Debug functions"

/**
 * @brief Sends a message through Serial with Debug (DEBUG_LOG) log level.
 *
 * @param message The message to send
 */
void debug(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_LOG
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Debug (DEBUG_LOG) log level.
 *
 * @param message The message to send
 */
void debug(String message)
{
#if DEBUG_LEVEL <= DEBUG_LOG
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Debug (DEBUG_LOG) log level.
 *
 * @param message The message to send
 */
void debugln(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_LOG
    Serial.println(message);
#endif
}

/**
 * @brief Sends a message through Serial with Debug (DEBUG_LOG) log level.
 *
 * @param message The message to send
 */
void debugln(String message)
{
#if DEBUG_LEVEL <= DEBUG_LOG
    Serial.println(message);
#endif
}

#pragma endregion "Debug functions"

#pragma region "Info functions"

/**
 * @brief Sends a message through Serial with Info (DEBUG_INFO) log level.
 *
 * @param message The message to send
 */
void info(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_INFO
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Info (DEBUG_INFO) log level.
 *
 * @param message The message to send
 */
void info(String message)
{
#if DEBUG_LEVEL <= DEBUG_INFO
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Info (DEBUG_INFO) log level.
 *
 * @param message The message to send
 */
void infoln(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_INFO
    Serial.println(message);
#endif
}

/**
 * @brief Sends a message through Serial with Info (DEBUG_INFO) log level.
 *
 * @param message The message to send
 */
void infoln(String message)
{
#if DEBUG_LEVEL <= DEBUG_INFO
    Serial.println(message);
#endif
}

/**
 * @brief Sends a message through Serial with Info (DEBUG_INFO) log level.
 *
 * @param message The message to send
 */
void infoln(IPAddress message)
{
#if DEBUG_LEVEL <= DEBUG_INFO
    Serial.println(message);
#endif
}

#pragma endregion "Info functions"

#pragma region "Warning functions"

/**
 * @brief Sends a message through Serial with Warning (DEBUG_WARN) log level.
 *
 * @param message The message to send
 */
void warn(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_WARN
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Warning (DEBUG_WARN) log level.
 *
 * @param message The message to send
 */
void warn(String message)
{
#if DEBUG_LEVEL <= DEBUG_WARN
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Warning (DEBUG_WARN) log level.
 *
 * @param message The message to send
 */
void warnln(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_WARN
    Serial.println(message);
#endif
}

/**
 * @brief Sends a message through Serial with Warning (DEBUG_WARN) log level.
 *
 * @param message The message to send
 */
void warnln(String message)
{
#if DEBUG_LEVEL <= DEBUG_WARN
    Serial.println(message);
#endif
}

#pragma endregion "Warning functions"

#pragma region "Error functions"

/**
 * @brief Sends a message through Serial with Error (DEBUG_ERR) log level.
 *
 * @param message The message to send
 */
void err(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_ERR
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Error (DEBUG_ERR) log level.
 *
 * @param message The message to send
 */
void err(String message)
{
#if DEBUG_LEVEL <= DEBUG_ERR
    Serial.print(message);
#endif
}

/**
 * @brief Sends a message through Serial with Error (DEBUG_ERR) log level.
 *
 * @param message The message to send
 */
void errln(const char *message)
{
#if DEBUG_LEVEL <= DEBUG_ERR
    Serial.println(message);
#endif
}

/**
 * @brief Sends a message through Serial with Error (DEBUG_ERR) log level.
 *
 * @param message The message to send
 */
void errln(String message)
{
#if DEBUG_LEVEL <= DEBUG_ERR
    Serial.println(message);
#endif
}

#pragma endregion "Error functions"

#endif
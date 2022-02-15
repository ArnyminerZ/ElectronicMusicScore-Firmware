/**
 * @file consts.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Used for defining general usage constants. See each line for an explanation.
 * @version 0.1
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */

// General definitions
#define FIRMWARE_VERSION "v1.0.0-SNAPSHOT"

#include "logger_levels.h"

// Can be one of the following: DEBUG_LOG, DEBUG_INFO, DEBUG_WARN, DEBUG_ERR
#define DEBUG_LEVEL DEBUG_LOG

// Define DEBUG_MODE if DEBUG_LEVEL is lower or equal than DEBUG_LOG
#if DEBUG_LEVEL <= DEBUG_LOG
#define DEBUG_MODE
#endif

// Disabled OTA until working with 16MB of flash
// #define ENABLE_OTA
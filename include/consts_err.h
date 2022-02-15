/**
 * @file consts_err.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Used for defining error codes.
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CONSTS_ERR_H
#define CONSTS_ERR_h

// When the user is not authenticated
#define ERR_AUTH "no-auth"

/**
 * ERRORS OF CONFIGURATION PARAMETERS
 */

#define ERR_CONFIG_PARAMS "missing-param"
// When the value was expected to be numeric and it isn't
#define ERR_CONFIG_NUMERIC "value-numeric"
// When the key is not valid
#define ERR_CONFIG_KEY "invalid-key"
// When the set value integer is out of the bounds
#define ERR_CONFIG_BOUNDS "out-of-bounds"

#define CONFIG_OK "ok"

#endif

/**
 * @file hash.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Provides some hasing functions.
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef HASH_H
#define HASH_H

// Include dependencies
#include "mbedtls/md.h"
#include <Arduino.h>

/**
 * @brief Returns a hash of the [payload] as a [String].
 * 
 * @param payload The text to hash.
 * @return String The hashed text.
 */
String hash(const char *payload)
{
  const size_t payloadLength = strlen(payload);
  byte shaResult[payloadLength];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  String builder = "";
  for (int i = 0; i < sizeof(shaResult); i++)
  {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    builder += str;
  }

  return builder;
}

#endif
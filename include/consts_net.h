/**
 * @file consts_net.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief Provides constants for network interactions.
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CONSTS_NET_H
#define CONSTS_NET_H

#define AUTH_DEFAULT_USER "admin"
#define AUTH_DEFAULT_PASS "admin"

#define WEB_PORT 80

#define DNS_PORT 53

// HTTP MIME types
#define MIME_PLAIN "text/plain"
#define MIME_HTML "text/html"
#define MIME_JSON "application/json"

// HTTP result codes, see https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400

#endif
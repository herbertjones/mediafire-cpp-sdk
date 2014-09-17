/**
 * @file default_http_headers.hpp
 * @author Herbert Jones
 * @brief Overridable default headers.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace http {

#ifndef HTTP_DEFAULT_HEADERS
#define HTTP_DEFAULT_HEADERS
/**
 * Default headers for the application. Can be overridden by setting
 * HTTP_DEFAULT_HEADERS.
 */
char const * const default_headers[][2] = {
    {"Accept", "*/*"},
    {"TE", "trailers"},
    {"Accept-Encoding", "gzip"},
    {"User-Agent", "HttpRequester"},
    {"Connection", "close"},
};
#endif

}  // namespace http
}  // namespace mf

/**
 * @file headers.hpp
 * @author Herbert Jones
 * @brief Headers struct
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace mf {
namespace http {

/**
 * @class Headers
 * @brief Wrapper for header data.
 */
struct Headers
{
    /** String containing header portion of HTTP response. */
    std::string raw_headers;

    /** HTTP status message such as "200" in "HTTP/1.1 200 OK" */
    uint16_t status_code;

    /** HTTP status message such as "OK" in "HTTP/1.1 200 OK" */
    std::string status_message;

    /**
     * HTTP header names are case insensitive, so the key value here is
     * converted to lowercase for ease of use.
     */
    std::map<std::string, std::string> headers;
};

}  // namespace http
}  // namespace mf

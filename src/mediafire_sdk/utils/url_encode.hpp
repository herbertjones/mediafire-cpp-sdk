/**
 * @file url_encode.hpp
 * @author Herbert Jones
 * @brief Url encoding utility
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/optional.hpp"

namespace mf {
namespace utils {

    /**
     * @brief Encode data to be used within a URL.
     *
     * See: http://en.wikipedia.org/wiki/Percent-encoding
     *
     * @param[in] str Data to be percent encoded by uri specification.
     *
     * @return Encoded string
     */
    std::string UrlEncode(const std::string & str);

    /**
     * @brief Unencode data used within a URL.
     *
     * See: http://en.wikipedia.org/wiki/Percent-encoding
     *
     * @param[in] str Data to be percent unencoded
     *
     * @return Unencoded string or nothing if encoding is invalid.
     */
    boost::optional<std::string> UrlUnencode(const std::string & str);

    /**
     * @brief Encode data to be used within POST form data.
     *
     * See: http://en.wikipedia.org/wiki/Percent-encoding
     *
     * @param[in] str Data to be percent encoded by uri specification.
     *
     * @return Encoded string
     */
    std::string UrlPostEncode(const std::string & str);

    /**
     * @brief Unencode data used within POST form data.
     *
     * See: http://en.wikipedia.org/wiki/Percent-encoding
     *
     * @param[in] str Data to be percent unencoded
     *
     * @return Unencoded string or nothing if encoding is invalid.
     */
    boost::optional<std::string> UrlPostUnencode(const std::string & str);


}  // namespace utils
}  // namespace mf

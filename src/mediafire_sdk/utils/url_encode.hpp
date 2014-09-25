/**
 * @file url_encode.hpp
 * @author Herbert Jones
 * @brief Url encoding utility
 *
 * See http://tools.ietf.org/html/rfc3986
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/optional.hpp"

namespace mf {
namespace utils {
namespace url {

namespace percent {
/**
 * @brief Percent encode a string
 *
 * See: http://en.wikipedia.org/wiki/Percent-encoding
 *
 * @param[in] str Data to be percent encoded by uri specification.
 *
 * @return Encoded string
 */
std::string Encode(const std::string & str);

/**
 * @brief Unencode data used within a URL.
 *
 * See: http://en.wikipedia.org/wiki/Percent-encoding
 *
 * @param[in] str Data to be percent unencoded
 *
 * @return Unencoded string or nothing if encoding is invalid.
 */
boost::optional<std::string> Decode(const std::string & str);
}  // namespace percent

namespace percent_plus {
/**
 * @brief Encode data to be used within POST form data.
 *
 * See: http://en.wikipedia.org/wiki/Percent-encoding
 *
 * @param[in] str Data to be percent encoded by uri specification.
 *
 * @return Encoded string
 */
std::string Encode(const std::string & str);

/**
 * @brief Unencode data used within POST form data.
 *
 * See: http://en.wikipedia.org/wiki/Percent-encoding
 *
 * @param[in] str Data to be percent unencoded
 *
 * @return Unencoded string or nothing if encoding is invalid.
 */
boost::optional<std::string> Decode(const std::string & str);
}  // namespace percent_plus

namespace get_parameter = percent;
namespace post_parameter = percent_plus;

}  // namespace url
}  // namespace utils
}  // namespace mf

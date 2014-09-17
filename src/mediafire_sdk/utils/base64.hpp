/**
 * @file base64.hpp
 * @author Herbert Jones
 * @brief Base64 encoding
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "boost/optional.hpp"

namespace mf {
namespace utils {

/**
 * @brief Encode bytes to base64.
 *
 * @param[in] data Pointer to the bytes to encode.
 * @param[in] size Size of the data at pointer.
 *
 * @return bas64 encoded string
 */
std::string Base64Encode( void const * data, std::size_t size );

/**
 * @brief Decode a base64 string.
 *
 * String must be properly base 64 encoded for this to return data.
 *
 * @param[in] input Base64 string
 *
 * @return Decoded bytes if string was properly encoded.
 */
boost::optional<std::vector<uint8_t>> Base64Decode(const std::string & input);

}  // namespace utils
}  // namespace mf

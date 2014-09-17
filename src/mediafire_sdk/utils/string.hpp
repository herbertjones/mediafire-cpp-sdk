/**
 * @file string.hpp
 * @author Herbert Jones
 * @brief Cross platform string funtions
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/filesystem/path.hpp"

#if defined(__MINGW32__)
#include "boost/lexical_cast.hpp"
#endif

namespace mf {
namespace utils {

/**
 * @brief Convert string to uint64_t.
 *
 * Same functionality as stoll, which is not available on all target platforms.
 *
 * @param[in] str String to convert
 * @param[in] base  The base of str.
 *
 * @return Number from string.
 */
uint64_t str_to_uint64(
        const std::string & str,
        int base
    );

/**
 * @brief Convert to string
 *
 * Wrapper for std::to_string or lexical cast if not available on platform.
 *
 * @param[in] numeric_value Value to convert
 *
 * @return Converted string
 */
template<typename T>
std::string to_string(T numeric_value)
{
#if defined(__MINGW32__)
    return boost::lexical_cast< std::string >( numeric_value );
#else
    return std::to_string(numeric_value);
#endif
}

/**
 * @brief Convert unicode encoded wide string to a UTF-8 encoded string.
 *
 * @param[in] str Wide character unicode string.
 *
 * @return UTF-8 encoded string
 */
std::string wide_to_bytes(const std::wstring & str);

/**
 * @brief Convert utf-8 encoded string to a wstring.
 *
 * @param[in] str UTF-8 encoded string
 *
 * @return Wide character string
 */
std::wstring bytes_to_wide(const std::string & str);

/**
 * @brief Convert a path to a utf8 encoded string.
 *
 * @param[in] path Path to convert
 *
 * @return UTF-8 string
 */
std::string path_to_utf8(const boost::filesystem::path & path);


}  // namespace utils
}  // namespace mf

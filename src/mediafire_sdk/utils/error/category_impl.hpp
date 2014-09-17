/**
 * @file category_impl.hpp
 * @author Herbert Jones
 * @brief std::error_code local implementation http_category().
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "mediafire_sdk/utils/error/codes.hpp"
#include "mediafire_sdk/utils/noexcept.hpp"

namespace mf {
namespace utils {

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& error_category();

}  // End namespace utils
}  // namespace mf

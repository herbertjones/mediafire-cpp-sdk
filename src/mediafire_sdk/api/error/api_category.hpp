/**
 * @file api_category.hpp
 * @author Herbert Jones
 * @brief Error category for API errors
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace mf {
namespace api {

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& api_category();

}  // End namespace api
}  // namespace mf

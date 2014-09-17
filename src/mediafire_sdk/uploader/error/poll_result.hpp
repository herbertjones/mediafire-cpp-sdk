/**
 * @file poll_result.hpp
 * @author Herbert Jones
 * @brief Generic uploader category
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace mf {
namespace uploader {

/** uploader error code values */

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& poll_result_category();

}  // End namespace uploader
}  // namespace mf

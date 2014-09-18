/**
 * @file upload_response.hpp
 * @author Herbert Jones
 * @brief Upload response error codes
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

namespace mf {
namespace uploader {

/**
 * @brief Create/get the instance of the error category.
 *
 * Contains the error code returned from the api response in the JSON
 * /response/error field.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& upload_response_category();

}  // End namespace uploader
}  // namespace mf

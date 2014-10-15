/**
 * @file error.hpp
 * @author Herbert Jones
 * @brief Error codes for the api_library module.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

//#include "mediafire_sdk/api/error/api_category.hpp"
//#include "mediafire_sdk/api/error/api_condition.hpp"

#include "mediafire_sdk/api/error/codes/api_code.hpp"
#include "mediafire_sdk/api/error/codes/http_status.hpp"
#include "mediafire_sdk/api/error/codes/result_code.hpp"
#include "mediafire_sdk/api/error/conditions/generic.hpp"

namespace mf {
namespace api {

/**
 * @brief Returns whether an error returned from the API should invalidate the
 *        session token used for the request that returned that error.
 *
 * This is used to determine when a session token can be reused or must be
 * thrown away.
 *
 * @param[in] error The error to check.
 */
bool IsInvalidSessionTokenError(std::error_code error);

/**
 * @brief Returns whether an error returned from the API should invalidate the
 *        credentials used to acquire session tokens.
 *
 * This is used to determine when to force a user to provide new credentials.
 *
 * @param[in] error The error to check.
 */
bool IsInvalidCredentialsError(std::error_code error);

}  // End namespace api
}  // namespace mf

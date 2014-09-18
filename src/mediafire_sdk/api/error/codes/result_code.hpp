/**
 * @file result_code.hpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

// Api returns error codes has generic conditions to compare against.
// See http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-4.html

namespace mf {
namespace api {

// This only contains the result codes we specifically care about.  As the API
// returns multiple codes for the same error, we prefer to record the error code
// in this class, and override default_error_condition to return the generic api
// condition.

/** Contains the error code returned from the api response in the JSON
 * /response/error field. */
enum class result_code
{
    SessionTokenInvalid      = 105,
    CredentialsInvalid       = 107,
    SignatureInvalid         = 127,
    AsyncOperationInProgress = 208,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * Contains the error code returned from the api response in the JSON
 * /response/error field.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& result_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(result_code e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(result_code e);

}  // End namespace api
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::api::result_code> : public true_type {};

}  // namespace std

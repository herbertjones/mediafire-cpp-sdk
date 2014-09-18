/**
 * @file http_status.hpp
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

enum class http_status
{
    BadRequest = 400,
    Forbidden = 403,
    NotFound = 404,
    InternalServerError = 500,
    ApiInternalServerError = 900,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& http_status_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(http_status e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(http_status e);

}  // End namespace api
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::api::http_status> : public true_type {};

}  // namespace std

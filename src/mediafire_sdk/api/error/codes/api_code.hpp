/**
 * @file api_code.hpp
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

enum class api_code
{
    UnknownApiError = 1,  // Can not start at 0
    ContentInvalidData,
    ContentInvalidFormat,
    SessionTokenUnavailableTimeout,
    ConnectionUnavailableTimeout,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& api_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(api_code e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(api_code e);

}  // End namespace api
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::api::api_code> : public true_type {};

}  // namespace std

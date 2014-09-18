/**
 * @file poll_result.hpp
 * @author Herbert Jones
 * @brief Poll upload result error codes
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

namespace mf {
namespace uploader {

enum class poll_result
{
    UploadKeyNotFound = -80,
    InvalidUploadKey = -20,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * Contains the error code returned from the api response in the JSON
 * /response/error field.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& poll_result_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(poll_result e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(poll_result e);

}  // End namespace uploader
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::uploader::poll_result> : public true_type {};

}  // namespace std

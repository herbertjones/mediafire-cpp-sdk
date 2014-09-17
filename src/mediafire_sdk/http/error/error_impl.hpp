/**
 * @file error_impl.hpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

#include "mediafire_sdk/http/error/codes.hpp"
#include "mediafire_sdk/http/error/category_impl.hpp"

namespace std
{
    template <>
    struct is_error_condition_enum<mf::http::errc>
        : public true_type {};
}  // End namespace std

namespace mf {
namespace http
{
    // Argument-dependent name lookup rules says to put make_error_condition in
    // the same namespace as the data type, so the std templates choose
    // mf::http::errc over std::errc.

    /**
     * @brief Create an error condition for std::error_code usage.
     *
     * @param[in] e Error code
     *
     * @return Error condition
     */
    std::error_condition make_error_condition(errc e);

    /**
     * @brief Create an error code for std::error_code usage.
     *
     * @param[in] e Error code
     *
     * @return Error code
     */
    std::error_code make_error_code(errc e);

}  // End namespace http
}  // namespace mf


/**
 * @file error_impl.hpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

// Http returns error codes
// See http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-4.html

namespace mf {
namespace http {

enum class http_error
{
    InvalidUrl,
    InvalidRedirectUrl,
    RedirectPermissionDenied,
    UnableToResolve,
    UnableToConnect,
    UnableToConnectToProxy,
    ProxyProtocolFailure,
    SslHandshakeFailure,
    WriteFailure,
    ReadFailure,
    CompressionFailure,
    UnparsableHeaders,
    UnsupportedEncoding,
    PostInterfaceReadFailure,
    Cancelled,
    IoTimeout,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& http_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(http_error e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(http_error e);

}  // End namespace http
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::http::http_error> : public true_type {};

}  // namespace std

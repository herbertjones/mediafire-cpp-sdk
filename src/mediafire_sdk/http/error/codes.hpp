/**
 * @file codes.hpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace http
{
    /** Module error codes. */
    enum class errc
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
        VariablePostInterfaceFailure,
        Cancelled,
        IoTimeout,
    };

}  // End namespace http
}  // namespace mf

/**
 * @file category_impl.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "category_impl.hpp"

#include <sstream>
#include <string>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace {
class DeviceCategoryImpl : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char* name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;

    /**
     * @brief Compare other error codes to your error conditions/values, or
     *        pass them to other comparers.
     *
     * Any of your error codes that correspond to std::errc types should be
     * matched up here, and return true if so.
     *
     * See: http://en.cppreference.com/w/cpp/error/errc
     */
    virtual bool equivalent(
            const std::error_code& code,
            int condition
        ) const NOEXCEPT;
};

const char* DeviceCategoryImpl::name() const NOEXCEPT
{
    return "http";
}

std::string DeviceCategoryImpl::message(int ev) const
{
    using mf::http::errc;

    switch (static_cast<errc>(ev))
    {
        case errc::InvalidUrl:
            return "invalid url";
        case errc::InvalidRedirectUrl:
            return "invalid redirect url";
        case errc::RedirectPermissionDenied:
            return "redirect permission denied";
        case errc::UnableToResolve:
            return "failure to resolve host";
        case errc::UnableToConnect:
            return "failure to connect to host";
        case errc::UnableToConnectToProxy:
            return "failure to connect to UnableToConnectToProxy";
        case errc::ProxyProtocolFailure:
            return "proxy protocol failure";
        case errc::SslHandshakeFailure:
            return "SSL handshake failure";
        case errc::WriteFailure:
            return "connection failure while writing";
        case errc::ReadFailure:
            return "connection failure while reading";
        case errc::CompressionFailure:
            return "failed to uncompress content";
        case errc::UnparsableHeaders:
            return "unable to parse http headers";
        case errc::UnsupportedEncoding:
            return "unsupported http encoding";
        case errc::VariablePostInterfaceFailure:
            return "variable post interface failure";
        case errc::Cancelled:
            return "request cancelled";
        case errc::IoTimeout:
            return "request timed out";
        default:
        {
            std::stringstream ss;
            ss << "Unknown error: " << ev;
            return ss.str();
        }
    }
}

bool DeviceCategoryImpl::equivalent(
        const std::error_code& /*code*/,
        int condition
    ) const NOEXCEPT
{
    using mf::http::errc;

    switch (static_cast<errc>(condition))
    {
        // case errc::PermissionDenied:
        // {
        //     // This is our own impl of permission denied, which matches the
        //     // one from std::errc
        //     return code == std::errc::permission_denied;
        // }
        default:
            return false;
    }
}
}  // namespace

const std::error_category& mf::http::error_category()
{
    static DeviceCategoryImpl instance;
    return instance;
}


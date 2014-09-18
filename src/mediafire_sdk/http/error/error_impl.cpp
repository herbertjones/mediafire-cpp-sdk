/**
 * @file error_impl.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "error_impl.hpp"

#include <string>
#include <sstream>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace {

class CategoryImpl
    : public std::error_category
{
public:
  virtual const char* name() const NOEXCEPT;
  virtual std::string message(int ev) const;
  virtual std::error_condition default_error_condition(int ev) const NOEXCEPT;
};

const char* CategoryImpl::name() const NOEXCEPT
{
    return "mediafire http";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::http::http_error;

    switch (static_cast<http_error>(ev))
    {
        case http_error::InvalidUrl:
            return "invalid url";
        case http_error::InvalidRedirectUrl:
            return "invalid redirect url";
        case http_error::RedirectPermissionDenied:
            return "redirect permission denied";
        case http_error::UnableToResolve:
            return "failure to resolve host";
        case http_error::UnableToConnect:
            return "failure to connect to host";
        case http_error::UnableToConnectToProxy:
            return "failure to connect to UnableToConnectToProxy";
        case http_error::ProxyProtocolFailure:
            return "proxy protocol failure";
        case http_error::SslHandshakeFailure:
            return "SSL handshake failure";
        case http_error::WriteFailure:
            return "connection failure while writing";
        case http_error::ReadFailure:
            return "connection failure while reading";
        case http_error::CompressionFailure:
            return "failed to uncompress content";
        case http_error::UnparsableHeaders:
            return "unable to parse http headers";
        case http_error::UnsupportedEncoding:
            return "unsupported http encoding";
        case http_error::VariablePostInterfaceFailure:
            return "variable post interface failure";
        case http_error::Cancelled:
            return "request cancelled";
        case http_error::IoTimeout:
            return "request timed out";
        default:
        {
            std::ostringstream ss;
            ss << "Unknown error: " << ev;
            return ss.str();
        }
    }
}

std::error_condition CategoryImpl::default_error_condition(
        int ev
    ) const NOEXCEPT
{
    using mf::http::http_error;

    switch (static_cast<http_error>(ev))
    {
        case http_error::InvalidUrl:
            return std::errc::invalid_argument;
        case http_error::InvalidRedirectUrl:
            return std::errc::invalid_argument;
        case http_error::RedirectPermissionDenied:
            return std::errc::permission_denied;
        case http_error::UnableToResolve:
            return std::errc::host_unreachable;
        case http_error::UnableToConnect:
            return std::errc::connection_refused;
        case http_error::UnableToConnectToProxy:
            return std::errc::connection_refused;
        case http_error::ProxyProtocolFailure:
            return std::errc::protocol_error;
        case http_error::SslHandshakeFailure:
            return std::errc::protocol_error;
        case http_error::WriteFailure:
            return std::errc::io_error;
        case http_error::ReadFailure:
            return std::errc::io_error;
        case http_error::CompressionFailure:
            return std::errc::io_error;
        case http_error::UnparsableHeaders:
            return std::errc::protocol_error;
        case http_error::UnsupportedEncoding:
            return std::errc::protocol_error;
        case http_error::VariablePostInterfaceFailure:
            return std::errc::broken_pipe;
        case http_error::Cancelled:
            return std::errc::operation_canceled;
        case http_error::IoTimeout:
            return std::errc::timed_out;
        default:
            return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace http {

const std::error_category& http_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(http_error e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::http::http_category()
            );
}

std::error_condition make_error_condition(http_error e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::http::http_category()
            );
}

}  // End namespace http
}  // namespace mf

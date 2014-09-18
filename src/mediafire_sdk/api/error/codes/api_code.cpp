/**
 * @file api_code.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "api_code.hpp"

#include <string>
#include <sstream>

#include "mediafire_sdk/utils/noexcept.hpp"
#include "mediafire_sdk/api/error/conditions/generic.hpp"

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
    return "mediafire api sdk";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::api::api_code;

    switch (static_cast<api_code>(ev))
    {
        case api_code::UnknownApiError:
            return "unknown api error";
        case api_code::ContentInvalidData:
            return "content invalid data";
        case api_code::ContentInvalidFormat:
            return "content invalid format";
        case api_code::SessionTokenUnavailableTimeout:
            return "session token unavailable timeout";
        case api_code::ConnectionUnavailableTimeout:
            return "connection unavailable timeout";
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
    using mf::api::api_code;
    using mf::api::errc;

    switch (static_cast<api_code>(ev))
    {
        case api_code::UnknownApiError:
            return errc::UnknownApiError;
        case api_code::ContentInvalidData:
            return errc::ContentInvalidData;
        case api_code::ContentInvalidFormat:
            return errc::ContentInvalidFormat;
        case api_code::SessionTokenUnavailableTimeout:
            return errc::SessionTokenUnavailableTimeout;
        case api_code::ConnectionUnavailableTimeout:
            return errc::ConnectionUnavailableTimeout;
        default:
            return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace api {

const std::error_category& api_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(api_code e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::api::api_category()
            );
}

std::error_condition make_error_condition(api_code e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::api::api_category()
            );
}

}  // End namespace api
}  // namespace mf

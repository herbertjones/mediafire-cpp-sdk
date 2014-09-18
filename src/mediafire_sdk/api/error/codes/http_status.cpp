/**
 * @file http_status.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "http_status.hpp"

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
    return "mediafire api http status";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::api::http_status;

    switch (static_cast<http_status>(ev))
    {
        case http_status::BadRequest:
            return "bad request";
        case http_status::Forbidden:
            return "forbidden";
        case http_status::NotFound:
            return "not found";
        case http_status::InternalServerError:
            return "internal server error";
        case http_status::ApiInternalServerError:
            return "api internal server error";
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
    using mf::api::http_status;

    switch (static_cast<http_status>(ev))
    {
        //case http_status::AsyncOperationInProgress:
        //    return std::errc::
        default:
            return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace api {

const std::error_category& http_status_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(http_status e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::api::http_status_category()
            );
}

std::error_condition make_error_condition(http_status e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::api::http_status_category()
            );
}

}  // End namespace api
}  // namespace mf

/**
 * @file poll_result.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "poll_result.hpp"

#include <string>
#include <sstream>

#include "mediafire_sdk/uploader/error/conditions/generic.hpp"
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
    return "pollupload file error";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::uploader::poll_result;

    switch (static_cast<poll_result>(ev))
    {
        case poll_result::UploadKeyNotFound:
            return "upload key not found";
        case poll_result::InvalidUploadKey:
            return "invalid upload key";
        default:
        {
            std::ostringstream ss;
            ss << "API result code: " << ev;
            return ss.str();
        }
    }
}

std::error_condition CategoryImpl::default_error_condition(
        int ev
    ) const NOEXCEPT
{
    switch (ev)
    {
        default: return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace uploader {

const std::error_category& poll_result_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(poll_result e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::uploader::poll_result_category()
            );
}

std::error_condition make_error_condition(poll_result e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::uploader::poll_result_category()
            );
}

}  // End namespace uploader
}  // namespace mf

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
     * See: utils://en.cppreference.com/w/cpp/error/errc
     */
    virtual bool equivalent(
        const std::error_code& code,
        int condition
    ) const NOEXCEPT;
};

const char* DeviceCategoryImpl::name() const NOEXCEPT
{
    return "mf::utils";
}

std::string DeviceCategoryImpl::message(int ev) const
{
    using mf::utils::errc;

    switch (static_cast<errc>(ev))
    {
        case errc::UnknownError:
            return "unknown error";
        case errc::EndOfFile:
            return "end of file";
        case errc::StreamError:
            return "stream error";
        case errc::LineTooLong:
            return "line too long";
        case errc::FileModified:
            return "file modified";
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
    using mf::utils::errc;

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

const std::error_category& mf::utils::error_category()
{
    static DeviceCategoryImpl instance;
    return instance;
}


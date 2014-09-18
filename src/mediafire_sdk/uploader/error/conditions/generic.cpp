/**
 * @file generic.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "generic.hpp"

#include <cassert>
#include <sstream>
#include <string>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace {
/**
 * @class ApiConditionImpl
 * @brief std::error_category implementation for api namespace
 */
class ApiConditionImpl : public std::error_category
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

const char* ApiConditionImpl::name() const NOEXCEPT
{
    return "mediafire uploader";
}

std::string ApiConditionImpl::message(int ev) const
{
    using mf::uploader::errc;

    switch (static_cast<errc>(ev))
    {
        case errc::BadUploadResponse:
            return "bad upload response";
        case errc::Cancelled:
            return "cancelled";
        case errc::FileExistInFolder:
            return "file exists in folder";
        case errc::FiledropKeyInvalid:
            return "filedrop key invalid";
        case errc::FiledropMisconfigured:
            return "filedrop misconfigured";
        case errc::FilenameMissing:
            return "filename missing";
        case errc::FilesizeInvalid:
            return "filesize invalid";
        case errc::HashIncorrect:
            return "hash incorrect";
        case errc::IncompleteUpload:
            return "incomplete upload";
        case errc::InsufficientCloudStorage:
            return "insufficient cloud storage";
        case errc::InternalUploadError:
            return "internal upload error";
        case errc::LogicError:
            return "logic error";
        case errc::MaxFilesizeExceeded:
            return "max filesize exceeded";
        case errc::MissingFileData:
            return "missing file data";
        case errc::Paused:
            return "paused";
        case errc::Permissions:
            return "permissions";
        case errc::SessionInvalid:
            return "session invalid";
        case errc::TargetFolderMissing:
            return "target folder missing";
        case errc::ZeroByteFile:
            return "zero byte file";
        default:
            {
                assert(!"Unimplemented condition");
                std::stringstream ss;
                ss << "Unknown uploader category: " << ev;
                return ss.str();
            }
    }
}

bool ApiConditionImpl::equivalent(
        const std::error_code& ec,
        int condition_code
    ) const NOEXCEPT
{
    using mf::uploader::errc;

    switch (static_cast<errc>(condition_code))
    {
        default:
            return false;
    }
}
}  // namespace

namespace mf {
namespace uploader {

std::error_condition make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            generic_upload_category()
            );
}

std::error_code make_error_code(errc e)
{
    return std::error_code(
            static_cast<int>(e),
            generic_upload_category()
            );
}

const std::error_category& generic_upload_category()
{
    static ApiConditionImpl instance;
    return instance;
}


}  // namespace uploader
}  // namespace mf

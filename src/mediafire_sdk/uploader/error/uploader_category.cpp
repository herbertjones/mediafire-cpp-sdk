/**
 * @file uploader_category.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "uploader_category.hpp"

#include <sstream>
#include <string>

#include "upload_response.hpp"

namespace mf {
namespace uploader {

std::error_condition make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            error_category()
            );
}

std::error_code make_error_code(errc e)
{
    return std::error_code(
            static_cast<int>(e),
            error_category()
            );
}

/**
 * @class UploadCategoryImpl
 * @brief std::error_category implementation for uploader namespace
 */
class UploadCategoryImpl : public std::error_category
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

const std::error_category& error_category()
{
    static UploadCategoryImpl instance;
    return instance;
}

const char* UploadCategoryImpl::name() const NOEXCEPT
{
    return "uploader";
}

std::string UploadCategoryImpl::message(int ev) const
{
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
            std::stringstream ss;
            ss << "Unknown error: " << ev;
            return ss.str();
        }
    }
}

bool UploadCategoryImpl::equivalent(
        const std::error_code& ec,
        int condition_code
    ) const NOEXCEPT
{
    switch (static_cast<errc>(condition_code))
    {
        case errc::FiledropKeyInvalid:
            return ec == std::error_code(-1, upload_response_category())
                || ec == std::error_code(-8, upload_response_category())
                || ec == std::error_code(-11, upload_response_category());
        case errc::FiledropMisconfigured:
            return ec == std::error_code(-21, upload_response_category())
                || ec == std::error_code(-22, upload_response_category());
        case errc::FilenameMissing:
            return ec == std::error_code(-49, upload_response_category());
        case errc::FilesizeInvalid:
            return ec == std::error_code(-48, upload_response_category());
        case errc::HashIncorrect:
            return ec == std::error_code(-90, upload_response_category());
        case errc::IncompleteUpload:
            return ec == std::error_code(-43, upload_response_category())
                || ec == std::error_code(-44, upload_response_category());
        case errc::InternalUploadError:
            return ec == std::error_code(-10, upload_response_category())
                || ec == std::error_code(-12, upload_response_category())
                || ec == std::error_code(-26, upload_response_category())
                || ec == std::error_code(-31, upload_response_category())
                || ec == std::error_code(-40, upload_response_category())
                || ec == std::error_code(-45, upload_response_category())
                || ec == std::error_code(-46, upload_response_category())
                || ec == std::error_code(-47, upload_response_category())
                || ec == std::error_code(-47, upload_response_category())
                || ec == std::error_code(-50, upload_response_category())
                || ec == std::error_code(-51, upload_response_category())
                || ec == std::error_code(-52, upload_response_category())
                || ec == std::error_code(-53, upload_response_category())
                || ec == std::error_code(-54, upload_response_category())
                || ec == std::error_code(-70, upload_response_category())
                || ec == std::error_code(-71, upload_response_category())
                || ec == std::error_code(-80, upload_response_category())
                || ec == std::error_code(-120, upload_response_category())
                || ec == std::error_code(-122, upload_response_category())
                || ec == std::error_code(-124, upload_response_category())
                || ec == std::error_code(-140, upload_response_category())
                || ec == std::error_code(-200, upload_response_category());
        case errc::MaxFilesizeExceeded:
            return ec == std::error_code(-41, upload_response_category())
                || ec == std::error_code(-42, upload_response_category())
                || ec == std::error_code(-700, upload_response_category())
                || ec == std::error_code(-701, upload_response_category())
                || ec == std::error_code(-881, upload_response_category())
                || ec == std::error_code(-882, upload_response_category());
        case errc::MissingFileData:
            return ec == std::error_code(-32, upload_response_category());
        case errc::Permissions:
            return ec == std::error_code(-99, upload_response_category());
        case errc::SessionInvalid:
            return ec == std::error_code(-99, upload_response_category());
        case errc::TargetFolderInTrash:
            return ec == std::error_code(-206, upload_response_category());
        case errc::TargetFolderMissing:
            return ec == std::error_code(-14, upload_response_category());
        default:
            return false;
    }
}

}  // namespace uploader
}  // namespace mf

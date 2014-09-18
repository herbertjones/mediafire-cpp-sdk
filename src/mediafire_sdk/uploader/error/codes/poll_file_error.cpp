/**
 * @file poll_file_error.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "poll_file_error.hpp"

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
    using mf::uploader::poll_file_error;

    switch (static_cast<poll_file_error>(ev))
    {
        case poll_file_error::FileSizeLargerThanMax:
            return "file size larger than maximum";
        case poll_file_error::FileSizeZero:
            return "file size is zero";
        case poll_file_error::BadRar1:
            return "bad rar(1)";
        case poll_file_error::BadRar2:
            return "bad rar(2)";
        case poll_file_error::VirusFound:
            return "virus found";
        case poll_file_error::FileLost:
            return "file lost";
        case poll_file_error::FileHashOrSizeMismatch:
            return "hash or size mismatch";
        case poll_file_error::UnknownInternalError1:
            return "internal error(1)";
        case poll_file_error::BadRar3:
            return "bad rar(3)";
        case poll_file_error::UnknownInternalError2:
            return "internal error(2)";
        case poll_file_error::DatabaseFailure:
            return "database failure";
        case poll_file_error::FilenameCollision:
            return "filename collision";
        case poll_file_error::DestinationMissing:
            return "destination missing";
        case poll_file_error::AccountStorageLimitReached:
            return "account storage limit reached";
        case poll_file_error::RevisionConflict:
            return "revision conflict";
        case poll_file_error::PatchFailed:
            return "patch failed";
        case poll_file_error::AccountBlocked:
            return "account blocked";
        case poll_file_error::PathCreationFailure:
            return "path creation failure";
        default:
        {
            std::ostringstream ss;
            ss << "Poll file error: " << ev;
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

const std::error_category& poll_upload_file_error_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(poll_file_error e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::uploader::poll_upload_file_error_category()
            );
}

std::error_condition make_error_condition(poll_file_error e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::uploader::poll_upload_file_error_category()
            );
}

}  // End namespace uploader
}  // namespace mf

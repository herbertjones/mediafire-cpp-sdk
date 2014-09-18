/**
 * @file upload_response.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_response.hpp"

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
    return "upload response";
}

std::string CategoryImpl::message(int ev) const
{
    switch (ev)
    {
        default:
        {
            std::ostringstream ss;
            ss << "Upload response code: " << ev;
            return ss.str();
        }
    }
}

std::error_condition CategoryImpl::default_error_condition(
        int ev
    ) const NOEXCEPT
{
    namespace upload = mf::uploader;

    switch (ev)
    {
        case -1:   return upload::errc::FiledropKeyInvalid;
        case -8:   return upload::errc::FiledropKeyInvalid;
        case -10:  return upload::errc::InternalUploadError;
        case -11:  return upload::errc::FiledropKeyInvalid;
        case -12:  return upload::errc::InternalUploadError;
        case -14:  return upload::errc::TargetFolderMissing;
        case -21:  return upload::errc::FiledropMisconfigured;
        case -22:  return upload::errc::FiledropMisconfigured;
        case -26:  return upload::errc::InternalUploadError;
        case -31:  return upload::errc::InternalUploadError;
        case -32:  return upload::errc::MissingFileData;
        case -40:  return upload::errc::InternalUploadError;
        case -41:  return upload::errc::MaxFilesizeExceeded;
        case -42:  return upload::errc::MaxFilesizeExceeded;
        case -43:  return upload::errc::IncompleteUpload;
        case -44:  return upload::errc::IncompleteUpload;
        case -45:  return upload::errc::InternalUploadError;
        case -46:  return upload::errc::InternalUploadError;
        case -47:  return upload::errc::InternalUploadError;
        case -48:  return upload::errc::FilesizeInvalid;
        case -49:  return upload::errc::FilenameMissing;
        case -50:  return upload::errc::InternalUploadError;
        case -51:  return upload::errc::InternalUploadError;
        case -52:  return upload::errc::InternalUploadError;
        case -53:  return upload::errc::InternalUploadError;
        case -54:  return upload::errc::InternalUploadError;
        case -70:  return upload::errc::InternalUploadError;
        case -71:  return upload::errc::InternalUploadError;
        case -80:  return upload::errc::InternalUploadError;
        case -90:  return upload::errc::HashIncorrect;
        case -99:  return upload::errc::SessionInvalid;
        case -120: return upload::errc::InternalUploadError;
        case -122: return upload::errc::InternalUploadError;
        case -124: return upload::errc::InternalUploadError;
        case -140: return upload::errc::InternalUploadError;
        case -200: return upload::errc::InternalUploadError;
        case -206: return upload::errc::TargetFolderInTrash;
        case -700: return upload::errc::MaxFilesizeExceeded;
        case -701: return upload::errc::MaxFilesizeExceeded;
        case -881: return upload::errc::MaxFilesizeExceeded;
        case -882: return upload::errc::MaxFilesizeExceeded;
        default:   return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace uploader {

const std::error_category& upload_response_category()
{
    static CategoryImpl instance;
    return instance;
}

}  // End namespace uploader
}  // namespace mf

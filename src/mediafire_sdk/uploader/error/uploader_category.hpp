/**
 * @file uploader_category.hpp
 * @author Herbert Jones
 * @brief Generic uploader category
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace mf {
namespace uploader {

/** uploader error code values */
enum class errc
{
    BadUploadResponse,
    Cancelled,
    FileExistInFolder,
    FiledropKeyInvalid,
    FiledropMisconfigured,
    FilenameMissing,
    FilesizeInvalid,
    HashIncorrect,
    IncompleteUpload,
    InsufficientCloudStorage,
    InternalUploadError,
    LogicError,
    MaxFilesizeExceeded,
    MissingFileData,
    Paused,
    Permissions,
    SessionInvalid,
    TargetFolderInTrash,
    TargetFolderMissing,
    ZeroByteFile,
};

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(errc e);

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(errc e);

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& error_category();

}  // End namespace uploader
}  // namespace mf

namespace std
{
    template <>
    struct is_error_condition_enum<mf::uploader::errc>
        : public true_type {};
}  // End namespace std


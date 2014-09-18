/**
 * @file generic.hpp
 * @author Herbert Jones
 * @brief Uploader error conditions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

namespace mf {
namespace uploader {

/**
 * Uploader error conditions
 *
 * You may compare these to API errors and they may be used as generic errors.
 */
enum class errc
{
    BadUploadResponse = 1,
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
const std::error_category& generic_upload_category();

}  // End namespace uploader
}  // namespace mf

namespace std
{
    template <>
    struct is_error_condition_enum<mf::uploader::errc>
        : public true_type {};
}  // End namespace std

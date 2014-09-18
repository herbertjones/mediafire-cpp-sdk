/**
 * @file poll_file_error.hpp
 * @author Herbert Jones
 * @brief Poll upload file error codes
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

namespace mf {
namespace uploader {

enum class poll_file_error
{
    FileSizeLargerThanMax      = 1,
    FileSizeZero               = 2,
    BadRar1                    = 3,
    BadRar2                    = 4,
    VirusFound                 = 5,
    FileLost                   = 6,
    FileHashOrSizeMismatch     = 7,
    UnknownInternalError1      = 8,
    BadRar3                    = 9,
    UnknownInternalError2      = 10,
    DatabaseFailure            = 12,
    FilenameCollision          = 13,
    DestinationMissing         = 14,
    AccountStorageLimitReached = 15,
    RevisionConflict           = 16,
    PatchFailed                = 17,
    AccountBlocked             = 18,
    PathCreationFailure        = 19,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * Contains the error code returned from the api response in the JSON
 * /response/error field.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& poll_upload_file_error_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(poll_file_error e);

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(poll_file_error e);

}  // End namespace uploader
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::uploader::poll_file_error> : public true_type {};

}  // namespace std

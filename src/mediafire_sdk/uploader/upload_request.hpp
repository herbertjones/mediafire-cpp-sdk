/**
 * @file upload_request.hpp
 * @author Herbert Jones
 * @brief Container for upload requests
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/filesystem/path.hpp"
#include "boost/optional.hpp"

#include "./detail/upload_target.hpp"

namespace mf {
namespace uploader {

// Forward declaration
namespace detail { class UploadManagerImpl; }

/**
 * Action to take if the file already exists at the requested cloud path.
 */
enum class OnDuplicateAction
{
    /** Stop the upload if the file already exists. */
    Fail,

    /** Upload the file and replace the existing file. */
    Replace
};

/**
 * @class UploadRequest
 * @brief Request and configuration for a file upload.
 */
class UploadRequest
{
public:
    /**
     * @brief Create a file upload.
     *
     * @param[in] local_filepath The path to the file to upload on the local
     * system.
     */
    UploadRequest( std::string local_filepath );

    /**
     * @brief Create a file upload.
     *
     * @param[in] local_filepath The path to the file to upload on the local
     * system.
     */
    UploadRequest( std::wstring local_filepath );

    /**
     * @brief Create a file upload.
     *
     * @param[in] local_filepath The path to the file to upload on the local
     * system.
     */
    UploadRequest( boost::filesystem::path local_filepath );

    /**
     * @brief Upload the file with a different name than the one on the local
     * filesystem
     *
     * @param[in] cloud_filename The name the file will have in the cloud.
     */
    void SetTargetFilename( std::string cloud_filename );

    /**
     * @brief Set the upload target folder by folderkey.
     *
     * If you do not have the folderkey available, the path in the cloud can be
     * used as the target folder by setting the folder path instead.  See
     * SetTargetFolderPath.
     *
     * @param[in] folderkey The id of the folder.
     */
    void SetTargetFolderkey( std::string folderkey );

    /**
     * @brief Set the upload target folder by cloud path.
     *
     * @param[in] cloud_upload_path The path of the target folder in the cloud.
     */
    void SetTargetFolderPath( std::string cloud_upload_path );

    /**
     * @brief Set the action to take if the upload location already exists in
     * the cloud.
     *
     * If the target file already exists in the cloud, the file will only be
     * replaced if Replace is set here.
     *
     * @param[in] on_duplicate The action to take.
     */
    void SetOnDuplicateAction( OnDuplicateAction on_duplicate );

private:
    friend class detail::UploadManagerImpl;

    /** Path to the file on the local system */
    const boost::filesystem::path local_file_path_;

    /** Name of file in cloud if different from local_path. */
    boost::optional<std::string> utf8_target_name_;

    /** Location to upload the file */
    detail::UploadTarget upload_target_folder_;

    /** What should happen if filename already exists in folder? */
    OnDuplicateAction on_duplicate_action_;
};

}  // namespace uploader
}  // namespace mf

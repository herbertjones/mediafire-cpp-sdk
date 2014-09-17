/**
 * @file api/upload_manager.hpp
 * @author Herbert Jones
 * @brief Upload manager to permit easy uploading
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>
#include <string>

#include "boost/filesystem/path.hpp"

#include "upload_modification.hpp"
#include "upload_status.hpp"
#include "upload_request.hpp"

// Forward declarations
namespace boost { namespace filesystem { class path; } }
namespace mf {
namespace api { class SessionMaintainer; }
namespace uploader { namespace detail { class UploadManagerImpl; } }
}  // namespace mf
// END forward declarations

namespace mf {
namespace uploader {

/**
 * @class UploadManager
 * @brief Manage a set of uploads.
 */
class UploadManager
{
public:
    typedef std::function<void(UploadStatus)> StatusCallback;

    /**
     * @brief UploadManager constructor
     *
     * @param[in] session_maintainer Pointer to API SessionMaintainer object
     * which will be used to perform the upload.
     */
    UploadManager(
            ::mf::api::SessionMaintainer * session_maintainer
        );
    ~UploadManager();

    /**
     * @brief Request a file upload.
     *
     * @param[in] request Data on the upload to perform.
     * @param[in] callback A callback function that will receive updates on the
     * progress of the upload.
     */
    void Add(const UploadRequest & request, StatusCallback callback);

    /**
     * @brief Modify an ongoing upload.
     *
     * @param[in] filepath Local path of the file to modify.
     * @param[in] modification How to modify the upload.
     */
    void ModifyUpload(
            const std::string & filepath,
            UploadModification modification
        );

    /**
     * @brief Modify an ongoing upload.
     *
     * @param[in] filepath Local path of the file to modify.
     * @param[in] modification How to modify the upload.
     */
    void ModifyUpload(
            const std::wstring & filepath,
            UploadModification modification
        );

    /**
     * @brief Modify an ongoing upload.
     *
     * @param[in] filepath Local path of the file to modify.
     * @param[in] modification How to modify the upload.
     */
    void ModifyUpload(
            const boost::filesystem::path & filepath,
            UploadModification modification
        );

private:
    std::shared_ptr<detail::UploadManagerImpl> impl_;
};

}  // namespace uploader
}  // namespace mf

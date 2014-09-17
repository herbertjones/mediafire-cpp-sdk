/**
 * @file api/upload_manager.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_manager.hpp"

#include <string>

#include "boost/filesystem.hpp"

#include "./detail/upload_manager_impl.hpp"

namespace mf {
namespace uploader {

UploadManager::UploadManager(
        ::mf::api::SessionMaintainer * session_maintainer
    ) :
    impl_(std::make_shared<detail::UploadManagerImpl>(session_maintainer))
{
}

UploadManager::~UploadManager()
{
}

void UploadManager::Add(
        const UploadRequest & request,
        StatusCallback callback
    )
{
    impl_->Add(request, callback);
}

void UploadManager::ModifyUpload(
        const std::string & filepath,
        UploadModification modification
    )
{
    boost::filesystem::path path(filepath);
    ModifyUpload(path, modification);
}

void UploadManager::ModifyUpload(
        const std::wstring & filepath,
        UploadModification modification
    )
{
    boost::filesystem::path path(filepath);
    ModifyUpload(path, modification);
}

void UploadManager::ModifyUpload(
        const boost::filesystem::path & filepath,
        UploadModification modification
    )
{
    impl_->ModifyUpload(filepath, modification);
}

}  // namespace uploader
}  // namespace mf

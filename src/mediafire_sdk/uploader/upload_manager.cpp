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

UploadManager::UploadHandle UploadManager::Add(
        const UploadRequest & request,
        StatusCallback callback
    )
{
    return impl_->Add(request, callback);
}

void UploadManager::ModifyUpload(
        UploadHandle upload_handle,
        UploadModification modification
    )
{
    impl_->ModifyUpload(upload_handle, modification);
}

}  // namespace uploader
}  // namespace mf

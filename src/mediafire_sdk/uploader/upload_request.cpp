/**
 * @file upload_request.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_request.hpp"

using mf::uploader::OnDuplicateAction;

namespace {
const OnDuplicateAction default_on_duplicate_action = OnDuplicateAction::Fail;
}  // namespace

namespace mf {
namespace uploader {

UploadRequest::UploadRequest( std::string local_filepath ) :
    local_file_path_(local_filepath),
    on_duplicate_action_(default_on_duplicate_action)
{
}

UploadRequest::UploadRequest( std::wstring local_filepath ) :
    local_file_path_(local_filepath),
    on_duplicate_action_(default_on_duplicate_action)
{
}

UploadRequest::UploadRequest( boost::filesystem::path local_filepath ) :
    local_file_path_(local_filepath),
    on_duplicate_action_(default_on_duplicate_action)
{
}

void UploadRequest::SetTargetFilename( std::string cloud_filename )
{
    utf8_target_name_ = cloud_filename;
}

void UploadRequest::SetTargetFolderkey( std::string folderkey )
{
    upload_target_folder_ = detail::ParentFolderKey{ folderkey };
}

void UploadRequest::SetTargetFolderPath( std::string cloud_upload_path )
{
    upload_target_folder_ = detail::CloudPath{ cloud_upload_path };
}

void UploadRequest::SetOnDuplicateAction( OnDuplicateAction on_duplicate )
{
    on_duplicate_action_ = on_duplicate;
}

}  // namespace uploader
}  // namespace mf

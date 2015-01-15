/**
 * @file upload_status.hpp
 * @author Herbert Jones
 * @brief Status message from UploadManager
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/filesystem/path.hpp"
#include "boost/optional.hpp"
#include "boost/variant/variant.hpp"

#include "detail/types.hpp"

#include <system_error>

namespace mf {
namespace uploader {

namespace upload_state {
struct EnqueuedForHashing {};
struct Hashing {};
struct EnqueuedForUpload {};
struct Uploading {};
struct Polling {};
struct Error
{
    std::error_code error_code;
    std::string description;
};
struct Complete
{
    std::string quickkey;
    std::string filename;
    boost::optional<uint32_t> new_revision;
};
}  // namespace upload_state

using UploadState = boost::variant<
        upload_state::EnqueuedForHashing,
        upload_state::Hashing,
        upload_state::EnqueuedForUpload,
        upload_state::Uploading,
        upload_state::Polling,
        upload_state::Error,
        upload_state::Complete
    >;

struct UploadStatus
{
    detail::UploadHandle upload_handle;
    boost::filesystem::path path;
    UploadState state;
};

}  // namespace uploader
}  // namespace mf

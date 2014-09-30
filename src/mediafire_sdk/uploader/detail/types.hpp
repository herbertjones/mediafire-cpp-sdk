/**
 * @file types.hpp
 * @author Herbert Jones
 * @brief Types shared in upload classes.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace uploader {
namespace detail {

enum class CountState
{
    None,
    Hashing,
    Uploading
};

enum class ChunkState
{
    NeedsUpload,
    Uploading,
    Uploaded
};

struct UploadHandle
{
    uint32_t id;
};

}  // namespace detail
}  // namespace uploader
}  // namespace mf

/**
 * @file upload_events.hpp
 * @author Herbert Jones
 * @brief Events for the upload state machine.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

#include "boost/asio/io_service.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace event {

struct Error
{
    std::error_code error_code;
    std::string description;
};

struct Start {};
struct Restart {};

// Control
struct StartHash {};
struct StartUpload
{
    std::string upload_action_token;
};

// Check
struct NeedsSingleUpload
{
};

struct NeedsChunkUpload
{
};

struct InstantUpload
{
};

struct AlreadyUploaded
{
    std::string quickkey;
    std::string filename;
};

// Upload
struct ChunkSuccess
{
    uint32_t chunk_id;
    std::string upload_key;
};
struct SimpleUploadComplete
{
    std::string upload_key;
};

struct ChunkUploadComplete
{
    std::string upload_key;
};

struct InstantSuccess
{
    std::string quickkey;
    std::string filename;
};

struct PollComplete
{
    std::string quickkey;
    std::string filename;
};

}  // namespace event
}  // namespace detail
}  // namespace uploader
}  // namespace mf

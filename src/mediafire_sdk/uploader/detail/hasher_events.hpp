/**
 * @file hasher_events.hpp
 * @author Herbert Jones
 * @brief Events for the upload state machine.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

#include "boost/asio/io_service.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/date_time/posix_time/ptime.hpp"

#include "mediafire_sdk/utils/fileio.hpp"
#include "mediafire_sdk/utils/sha256_hasher.hpp"
#include "mediafire_sdk/uploader/hasher.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace hash_event {

struct HasherStateData_
{
    boost::asio::io_service * io_service;

    boost::filesystem::path filepath;
    uint64_t filesize;
    std::time_t mtime;

    ::mf::utils::Sha256Hasher primary_hasher;
    ::mf::utils::Sha256Hasher chunk_hasher;

    std::vector<std::pair<uint64_t,uint64_t>> chunk_ranges;
    std::vector<std::string> chunk_hashes;
};
using HasherStateData = std::shared_ptr<HasherStateData_>;


struct StartHash
{
    HasherStateData state;
};

struct ReadNext
{
    HasherStateData state;
    mf::utils::FileIO::Pointer file_io;
    uint64_t read_byte_pos;
};

struct HashSuccess
{
    HasherStateData state;
    std::string sha256_hash;
};

struct Error
{
    HasherStateData state;

    std::error_code error_code;
    std::string description;
};

}  // namespace hash_event
}  // namespace detail
}  // namespace uploader
}  // namespace mf

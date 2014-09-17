/**
 * @file hasher.hpp
 * @author Herbert Jones
 * @brief Tool to hash file for uploading.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/signals2.hpp"

namespace mf {
namespace uploader {

/**
 * @struct FileHashes
 * @brief Hash data on a single file.
 */
struct FileHashes
{
    boost::filesystem::path path;

    uint64_t file_size;
    std::time_t mtime;

    std::string hash;

    std::vector<std::string> chunk_hashes;
    std::vector< std::pair<uint64_t,uint64_t> > chunk_ranges;
};

// Forward declaration
struct HasherImpl;

/**
 * @class Hasher
 * @brief Class to hash a file for uploading.
 */
class Hasher :
    public std::enable_shared_from_this<Hasher>
{
public:
    using Pointer = std::shared_ptr<Hasher>;
    using Callback = std::function<
        void(
                std::error_code error_code,
                boost::optional<FileHashes> file_hash
            )>;

    static Pointer Create(
            boost::asio::io_service * fileread_io_service,
            boost::filesystem::path filepath,
            uint64_t filesize,
            std::time_t mtime,
            Callback callback
        );
    ~Hasher();

    /**
     * @brief Start hashing
     */
    void Start();

    /**
     * @brief Cancel in progress hashing.
     */
    void Cancel();

private:
    Hasher(
            boost::asio::io_service * fileread_io_service,
            boost::filesystem::path filepath,
            uint64_t filesize,
            std::time_t mtime,
            Callback callback
        );

    /** Keep self alive till done or cancelled. */
    Pointer self_;

    std::shared_ptr<HasherImpl> impl_;
};

}  // namespace uploader
}  // namespace mf

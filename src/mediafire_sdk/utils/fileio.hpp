/**
 * @file fileio.hpp
 * @author Zach Marquez
 * @brief Cross platform file read and write.
 * @copyright Copyright 2014 Mediafire
 *
 * Created by Zach Marquez on 1/27/14.
 */
#pragma once

#include <cstdio>
#include <string>
#include <system_error>

#include "boost/filesystem/path.hpp"
#include "boost/shared_ptr.hpp"

namespace mf {
namespace utils {

/**
 * Indicates a base position from which to seek.
 */
enum class SeekAnchor
{
    Current,    ///< Seek from the current stream position.
    Beginning,  ///< Seek from the beginning of the stream.
    End,        ///< Seek from the end of the stream.
};

/**
 * @class FileIO
 *
 * The FileIO class is used to read and write to files in a way that supports
 * Unicode file names and 64bit file positions. It cannot be instantiated
 * directly but should be created through the use of the static member Open().
 * Once a FileIO object is instantiated, it will hold the requested file open
 * until it is destroyed.
 */
class FileIO
{
public:
    /**
     * FileIO objects cannot be instantiated directly, but should be
     * manipulated through the use of Pointer objects.
     */
    typedef boost::shared_ptr<FileIO> Pointer;

    /**
     * A reasonable maximum read size. Attempting to read more than this size
     * could result in system instability! This size is guaranteed to be equal
     * to or larger than 512MB.
     */
    static const uint64_t skDefaultReadSize;

    /**
     * Opens a file, instatiating a new FileIO object.
     *
     * @param[in] path The path to the file to open.
     * @param[in] open_mode The mode string as accepted by fopen().
     * @param[out] error (Optional) Any that occurred during opening the file.
     * @return The Pointer object that points to the newly-opened FileIO object.
     */
    static Pointer Open(
            const std::string& path,
            const std::string& open_mode,
            std::error_code * error
        );

    /**
     * Opens a file, instatiating a new FileIO object.
     *
     * @param[in] path The path to the file to open.
     * @param[in] open_mode The mode string as accepted by fopen().
     * @param[out] error (Optional) Any that occurred during opening the file.
     * @return The Pointer object that points to the newly-opened FileIO object.
     */
    static Pointer Open(
            boost::filesystem::path,
            const std::string& open_mode,
            std::error_code * error
        );

    ~FileIO();

    /**
     * Reads a contiguous block from the file beginning at the current stream
     * position. This function will attempt to read buffer_size bytes, but may
     * read less if an error occurs or the end-of-file is reached.
     *
     * @param[in] buffer A pointer to the buffer where read file content will
     *                   be placed. Must be at least buffer_size bytes long.
     * @param[in] buffer_size The number of bytes to attempt to read.
     * @param[out] error (Optional) Any error that occured during the file read.
     * @return The number of bytes actually read. Note that if the bytes
     *         returned is not equal to buffer_size, then either an error
     *         occurred or the end-of-file was reached.
     */
    uint64_t Read(
            void* buffer,
            uint64_t buffer_size,
            std::error_code * error
        );

    /**
     * Reads a contiguous block from the file beginning at the current stream
     * position. The function will attempt to read either desired_bytes or until
     * the end of the stream.
     *
     * @param[in] desired_bytes The maximum number of bytes that will be read
     *                          from the stream. Set to skDefaultReadSize for
     *                          a reasonable limit.
     * @param[out] error (Optional) Any error that occured during the file read.
     * @return The data that was read from the file. The lenth of data will be
     *         different from the desired_bytes if an EOF or other error occurs.
     */
    std::string ReadString(
            uint64_t desired_bytes,
            std::error_code * error
        );

    /**
     * Reads from the file at the current stream position until hits a newline.
     * Will fail if it encounters an error or reads more than skDefaultReadSize
     * without encountering a newline. Upon error, it will still return the
     * bytes read before the error was encountered.
     *
     * @param[in] error (Optional) Any error encountered while trying to read.
     * @return The line read or empty if an error except EOF occurred.
     */
    std::string ReadLine(
            std::error_code * error
        );

    /**
     * Writes the content of a buffer to the file at the current stream
     * position. May write less than desired if an error occurs.
     *
     * @param[in] buffer The buffer holding the content to write to the file.
     *                   Must be a least buffer_size bytes long.
     * @param[in] buffer_size The number of bytes to write to the file.
     * @param[out] error (Optional) Any error that occurred when attempting to
     *                   write.
     * @return The number of bytes actually written. Note that if the number
     *         of bytes written is less than buffer_size, then an error
     *         occurred during the write.
     */
    uint64_t Write(
            const void* buffer,
            uint64_t buffer_size,
            std::error_code * error
        );

    /**
     * Writes the content of a buffer to the file at the current stream
     * position. May write less than desired if an error occurs.
     *
     * @param[in] buffer The buffer holding the content to write to the file.
     * @param[out] error (Optional) Any error that occurred when attempting to
     *                   write.
     * @return The number of bytes actually written. Note that if the number
     *         of bytes written is less than buffer.size(), then an error
     *         occurred during the write.
     */
    uint64_t WriteString(
            const std::string& buffer,
            std::error_code * error
        );

    /**
     * Sets the current stream position.
     *
     * @param[in] anchor The base point from which offset will be applied.
     * @param[in] offset The offset from the anchor at which to place set the
     *                   stream position.
     * @param[out] error (Optional) Any error that occurred while attempting to
     *                   seek.
     */
    void Seek(
            SeekAnchor anchor,
            uint64_t offset,
            std::error_code * error
        );

    /**
     * Returns the current stream position.
     */
    uint64_t Tell();

    /**
     * Flushes data to the disk.
     */
    void Flush();

private:
    FileIO(
            FILE* file_handle,
            uint64_t size_hint
        );

    FILE* file_handle_;
    uint64_t size_hint_;
};

}  // namespace utils
}  // namespace mf

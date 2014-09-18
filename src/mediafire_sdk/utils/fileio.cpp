/**
 * @file fileio.cpp
 * @author Zach Marquez
 * @copyright Copyright 2014 Mediafire
 */
#include "fileio.hpp"

#include <cstdlib>
#include <cstdio>
#include <limits>

#include "boost/filesystem.hpp"

#include "./error.hpp"
#include "./string.hpp"

namespace mf {
namespace utils {

namespace {
const uint64_t skDefaultBlockSize = 64 * uint64_t(1) << 20;  // 64 MB
}  // anonymous namespace

const uint64_t FileIO::skDefaultReadSize = uint64_t(1)<<30;  // 1 GB

FileIO::Pointer FileIO::Open(
        const std::string& path_str,
        const std::string& open_mode,
        std::error_code * error
    )
{
    if (error)
        error->clear();

#if _WIN32
    std::wstring wide_path = nsUtils::Utf8ToWideString(path_str.c_str());
    auto path = boost::filesystem::path(wide_path);
#else
    auto path = boost::filesystem::path(path_str);
#endif

    return Open(path, open_mode, error);
}

FileIO::Pointer FileIO::Open(
        boost::filesystem::path path,
        const std::string& open_mode,
        std::error_code * error
    )
{
    if (error)
        error->clear();

#if _WIN32
    std::wstring wide_open_mode = nsUtils::Utf8ToWideString(open_mode.c_str());
    FILE* file_handle = _wfopen(
            path.wstring().c_str(),
            wide_open_mode.c_str()
        );
#else
    FILE* file_handle = fopen(
        path.string().c_str(),
        open_mode.c_str());
#endif

    if (file_handle != nullptr)
    {
        boost::system::error_code bec;
        uint64_t size_hint = boost::filesystem::file_size(path, bec);
        if (bec || size_hint == 0)
            size_hint = skDefaultBlockSize;

        return FileIO::Pointer(new FileIO(
                file_handle,
                size_hint
        ));
    }
    else
    {
        if ( error != nullptr )
            *error = std::error_code(errno, std::system_category());
        return FileIO::Pointer();
    }
}

FileIO::FileIO(
        FILE* file_handle,
        uint64_t size_hint
    ) :
    file_handle_(file_handle),
    size_hint_(size_hint)
{
    assert( size_hint_ != std::numeric_limits<uint64_t>::max() );
}

FileIO::~FileIO()
{
    fclose(file_handle_);
}

uint64_t FileIO::Read(
        void* buffer,
        uint64_t buffer_size,
        std::error_code * error
    )
{
    if (error)
        error->clear();

    uint64_t bytes_read = fread(buffer, 1, buffer_size, file_handle_);

    // Detect errors if requested
    if ( error != nullptr && bytes_read != buffer_size )
    {
        if ( feof(file_handle_) )
        {
            *error = make_error_code(mf::utils::errc::EndOfFile);
        }
        else if ( ferror(file_handle_) )
        {
            *error = make_error_code(mf::utils::errc::StreamError);
        }
        else
        {
            assert( ! "Unknown error in FileIO::Read" );
            *error = make_error_code(mf::utils::errc::UnknownError);
        }
    }

    return bytes_read;
}

std::string FileIO::ReadString(
        uint64_t desired_bytes,
        std::error_code * error
    )
{
    if (error)
        error->clear();

    // Initialize
    std::string buffer;
    uint64_t total_bytes_read = 0;
    std::error_code read_error;

    try {
        // Calculate initial read size
        uint64_t read_size = std::min(desired_bytes, size_hint_ + 1);

        // Loop until we've read all desired or encounter an error
        while ( ! read_error && total_bytes_read < desired_bytes )
        {
            // Resize buffer
            buffer.resize(buffer.size() + read_size);

            // Calculate buffer position
            char* buffer_pos = &(&buffer.front())[total_bytes_read];

            // Read file
            uint64_t bytes_read = Read(buffer_pos, read_size, &read_error);
            total_bytes_read += bytes_read;

            // Ensure reads are complete or error
            assert( bytes_read == read_size || read_error );

            // Calculate next read size
            read_size = std::min(desired_bytes - total_bytes_read, skDefaultBlockSize);
        }
    }
    catch (const std::bad_alloc& a)
    {
        read_error = make_error_code(std::errc::not_enough_memory);
    }
    catch (const std::length_error&)
    {
        read_error = make_error_code(std::errc::result_out_of_range);
    }

    // Shrink buffer to actual bytes read
    buffer.resize(total_bytes_read);

    // Set return status
    if ( error != nullptr )
        *error = read_error;

    // Return read bytes
    return buffer;
}

std::string FileIO::ReadLine(
        std::error_code * error
    )
{
    if (error)
        error->clear();

    uint64_t bytes_consumed = 0;
    std::string lineData;

    // Read characters until we encounter an error, a newline, or we have read
    // too many bytes.
    int inputChar = fgetc(file_handle_);
    while ( inputChar != EOF && inputChar != '\n'
        && bytes_consumed < skDefaultReadSize )
    {
        lineData.push_back(static_cast<char>(inputChar));
        inputChar = fgetc(file_handle_);
        ++bytes_consumed;
    }

    // Set error condition
    if ( error != nullptr )
    {
        if ( inputChar == EOF )
        {   // Read error
            if ( feof(file_handle_) )
            {
                *error = make_error_code(mf::utils::errc::EndOfFile);
            }
            else if ( ferror(file_handle_) )
            {
                *error = make_error_code(mf::utils::errc::StreamError);
            }
            else
            {
                assert( ! "Unknown error in FileIO::Read" );
                *error = make_error_code(mf::utils::errc::UnknownError);
            }
        }
        else if ( bytes_consumed >= skDefaultReadSize )
        {   // Reached our limit
            *error = make_error_code(mf::utils::errc::LineTooLong);
        }
        else
        {   // No error
            assert( inputChar == '\n' );
        }
    }

    return lineData;
}

uint64_t FileIO::Write(
        const void* buffer,
        uint64_t buffer_size,
        std::error_code * error
    )
{
    if (error)
        error->clear();

    uint64_t bytes_written = fwrite(buffer, 1, buffer_size, file_handle_);

    // Detect errors if requested
    if ( error != nullptr && bytes_written != buffer_size )
    {
        if ( ferror(file_handle_) )
        {
            *error = make_error_code(mf::utils::errc::StreamError);
        }
        else
        {
            assert( ! "Unknown error in FileIO::Write" );
            *error = make_error_code(mf::utils::errc::UnknownError);
        }
    }

    return bytes_written;
}

uint64_t FileIO::WriteString(
        const std::string& buffer,
        std::error_code * error
    )
{
    return Write(buffer.data(), buffer.size(), error);
}

void FileIO::Seek(
        const SeekAnchor anchor,
        const uint64_t offset,
        std::error_code * error
    )
{
    if (error)
        error->clear();

    const int origin = [&anchor](){
        switch(anchor)
        {
            case SeekAnchor::Current:
                return SEEK_CUR;
                break;
            case SeekAnchor::Beginning:
                return SEEK_SET;
                break;
            case SeekAnchor::End:
                return SEEK_END;
                break;
            default:
                assert(!"Invalid anchor");
                return SEEK_CUR;
        }}();

#ifdef _WIN32
    const int res = fseeko64(file_handle_, offset, origin);
#else
    const int res = fseeko(file_handle_, offset, origin);
#endif

    if ( error != nullptr && res != 0 )
    {
        if ( ferror(file_handle_) )
        {
            *error = make_error_code(mf::utils::errc::StreamError);
        }
        else
        {
            assert( ! "Unknown error in FileIO::Seek" );
            *error = make_error_code(mf::utils::errc::UnknownError);
        }
    }
}

uint64_t FileIO::Tell()
{
#ifdef _WIN32
    uint64_t position = ftello64(file_handle_);
#else
    uint64_t position = ftello(file_handle_);
#endif

    return position;
}

void FileIO::Flush()
{
    fflush(file_handle_);
}

}  // namespace utils
}  // namespace mf

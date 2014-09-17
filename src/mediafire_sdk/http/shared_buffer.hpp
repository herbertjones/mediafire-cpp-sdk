/**
 * @file shared_buffer.hpp
 * @author Herbert Jones
 * @brief A buffer implementation to be used for efficient passing of data.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cassert>
#include <cstring>

#include <string>

#include "boost/smart_ptr/shared_array.hpp"

namespace mf {
namespace http {

/**
 * @class SharedBuffer
 * @brief Wrapper for efficiently pass raw bytes to and from HttpRequest.
 */
class SharedBuffer
{
public:
    /** Shared pointer acts as handle. */
    typedef std::shared_ptr<SharedBuffer> Pointer;

    /**
     * @brief Construct empty SharedBuffer
     *
     * Use this then set the buffer data via the Data() function.
     *
     * @param[in] size Size of buffer.
     *
     * @return SharedBuffer handle
     */
    static Pointer Create(uint64_t size)
    {
        return std::shared_ptr<SharedBuffer>(
            new SharedBuffer(size));
    }

    /**
     * @brief Construct SharedBuffer and fill with data from string.
     *
     * @param[in] str Source string.
     *
     * @return SharedBuffer handle
     */
    static Pointer Create(const std::string & str)
    {
        std::shared_ptr<SharedBuffer> shared_buffer(
            new SharedBuffer(str.size()));

        std::memcpy( shared_buffer->Data(), str.data(), str.size() );

        return shared_buffer;
    }

    virtual ~SharedBuffer() {}

    /**
     * @return The size of the buffer.
     */
    virtual uint64_t Size() const
    {
        return size_;
    }

    /**
     * @brief Get pointer to raw buffer for reading and writing.
     *
     * @return Pointer to buffer data.
     *
     * @warning Do not exceed Size().
     */
    virtual uint8_t * Data()
    {
        // Buffer has canary
        assert( buffer_[size_] == 0 );

        return buffer_.get();
    }

    /**
     * @brief Get pointer to raw buffer for reading.
     *
     * @return Pointer to buffer data.
     *
     * @warning Do not exceed Size().
     */
    virtual const uint8_t * Data() const
    {
        // Buffer has canary
        assert( buffer_[size_] == 0 );

        return buffer_.get();
    }

protected:
    /**
     * @brief Create shared buffer.
     *
     * @param[in] size The size of the buffer to be created.
     */
    SharedBuffer(uint64_t size) :
        size_(size),
        buffer_(new uint8_t[size_+1])
    {
        // Buffer has canary
        buffer_[size_] = 0;
    }

    /** Size of buffer_ */
    uint64_t size_;

    /** Buffer to hold data. */
    boost::shared_array<uint8_t> buffer_;
};

}  // namespace http
}  // namespace mf

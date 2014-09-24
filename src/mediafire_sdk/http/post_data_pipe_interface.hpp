/**
 * @file post_data_pipe_interface.hpp
 * @author Herbert Jones
 * @brief Interface for HttpRequest::SetPostDataPipe
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "shared_buffer.hpp"

namespace mf {
namespace http {

/**
 * @class PostDataPipeInterface
 * @brief Interface for sending POST data to an HttpRequest.
 */
class PostDataPipeInterface
{
public:
    PostDataPipeInterface() {}
    virtual ~PostDataPipeInterface() {}

    /**
     * @brief Size of the data to be read.
     *
     * This must return the entire size to be read.
     *
     * @return Byte size of POST data.
     */
    virtual uint64_t PostDataSize() const = 0;

    /**
     * @brief Returns a portion of the bytes to be read.
     *
     * When implementing this function in your class, you must always return
     * valid data until no more data exists.  This function will be called until
     * PostDataSize() bytes are read or an error occurs.
     *
     * If an error occurs while preparing the next chunk, return a bad Pointer
     * or empty response.  This will stop the request with an
     * http_error::PostInterfaceReadFailure(std::errc::broken_pipe) error.
     *
     * @return Handle to buffer of next POST data to be sent.
     */
    virtual SharedBuffer::Pointer RetreivePostDataChunk() = 0;

private:
};

}  // namespace http
}  // namespace mf

//
//  io_service_connection.hpp
//
//
//  Created by Zach Marquez on 2/17/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
/// @file
/// Contains the IoServiceConnection class.
#pragma once

#include "boost/shared_ptr.hpp"

// Forward declarations
namespace boost { namespace asio { class io_service; } }

namespace mf {
namespace utils {

// Forward declare a private helper class
namespace detail { class IoServiceConnectionHelper; }

/**
 * Represents a connection to an io_service queue of completion handlers. Allows
 * a user to post handlers without worrying about the handlers being called
 * after the user has been destroyed as long as the IoServiceConnection is
 * destroyed when the user is.
 */
class IoServiceConnection
{
public:
    /**
     * Constructor.
     *
     * @param[in] io_service The io_service on which to post function calls.
     */
    explicit IoServiceConnection(boost::asio::io_service* io_service);
    
    /**
     * Destructor.
     *
     * Ensures that any posted function calls will not be executed.
     */
    ~IoServiceConnection();
    
    /**
     * Posts the function object `function` to a delayed execution routine on
     * the io_service passed into the constructor, so that it will be called
     * once resolved only if this connection still exists.
     *
     * \param[in] function The function to be called when the io_service post
     *                     resolves.
     */
    template <class Function>
    void Post(Function function);
    
private:
    void Disconnect();
    
    boost::asio::io_service* io_service_;
    boost::shared_ptr<detail::IoServiceConnectionHelper> helper_;
};

}  // namespace utils
}  // namespace mf

// Inline function definitions
#include "io_service_connection-inl.hpp"

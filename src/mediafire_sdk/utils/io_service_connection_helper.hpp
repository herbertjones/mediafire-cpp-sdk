//
//  io_service_connection_helper.hpp
//
//
//  Created by Zach Marquez on 2/17/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
// Contains the IoServiceConnectionHelper class.
#pragma once

#include "boost/function.hpp"

namespace mf {
namespace utils {
namespace detail {

/**
 * Internal object that is the target of posts made by IoServiceConnection so
 * that they can resolve after the connection has been destroyed and be
 * silently ignored.
 */
class IoServiceConnectionHelper
{
public:
    typedef boost::function<void()> DeferredFunction;

    IoServiceConnectionHelper() : is_connected_(true) { }

    void Execute(const DeferredFunction& function) const
    {
        if ( is_connected_ )
            function();
    }
    
    void Disconnect() { is_connected_ = false; }
    
private:
    bool is_connected_;
};

}  // namespace detail
}  // namespace utils
}  // namespace mf

//
//  io_service_connection-inl.hpp
//
//
//  Created by Zach Marquez on 2/17/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
/// @file
/// Contains the IoServiceConnection inline functions. Not meant to be included
/// directly!
#pragma once

#include "boost/asio/io_service.hpp"
#include "boost/bind.hpp"

#include "io_service_connection_helper.hpp"

namespace mf {
namespace utils {

template <class Function>
void IoServiceConnection::Post(Function function)
{
    io_service_->post(
            boost::bind(
                &detail::IoServiceConnectionHelper::Execute,
                helper_,
                detail::IoServiceConnectionHelper::DeferredFunction(function)
            )
        );
}

}  // namespace utils
}  // namespace mf

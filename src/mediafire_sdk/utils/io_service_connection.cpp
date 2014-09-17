//
//  io_service_connection.cpp
//
//
//  Created by Zach Marquez on 2/17/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
/// @file
/// Contains the implementation of IoServiceConnection.
#include "io_service_connection.hpp"

#include "boost/make_shared.hpp"

namespace mf {
namespace utils {

IoServiceConnection::IoServiceConnection(boost::asio::io_service* io_service) :
    io_service_(io_service),
    helper_(boost::make_shared<detail::IoServiceConnectionHelper>())
{
    // Empty
}

IoServiceConnection::~IoServiceConnection()
{
    Disconnect();
}

void IoServiceConnection::Disconnect()
{
    helper_->Disconnect();
}

}  // namespace utils
}  // namespace mf

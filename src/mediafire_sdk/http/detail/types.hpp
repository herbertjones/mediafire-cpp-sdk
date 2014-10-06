/**
 * @file types.hpp
 * @author Herbert Jones
 * @brief Shared types
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <memory>

#include "boost/asio/streambuf.hpp"

namespace mf {
namespace http {
namespace detail {

using sclock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;
using TimePoint = std::chrono::steady_clock::time_point;

template<typename T>
Duration AsDuration(const T & t)
{
    return std::chrono::duration_cast<Duration>(t);
}

using SharedStreamBuf = std::shared_ptr<boost::asio::streambuf>;


}  // namespace detail
}  // namespace http
}  // namespace mf

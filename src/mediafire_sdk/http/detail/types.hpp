/**
 * @file types.hpp
 * @author Herbert Jones
 * @brief Shared types
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>

namespace mf {
namespace http {
namespace detail {

using sclock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template<typename T>
Duration AsDuration(const T & t)
{
    return std::chrono::duration_cast<Duration>(t);
}

}  // namespace detail
}  // namespace http
}  // namespace mf

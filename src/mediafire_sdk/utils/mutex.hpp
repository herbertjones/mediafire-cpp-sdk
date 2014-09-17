/**
 * @file mutex.hpp
 * @author Herbert Jones
 * @brief Multiplatform mutex namespace
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

// Mingw does not support c++11 mutex
#if defined(__MINGW32__)

#include "boost/thread.hpp"

namespace mf {
namespace utils {

using boost::mutex;
using boost::lock_guard;

}  // namespace utils
}  // namespace mf

#else

#include <mutex>

#include "boost/thread.hpp"

namespace mf {
namespace utils {

using std::mutex;
using std::lock_guard;

}  // namespace utils
}  // namespace mf
#endif


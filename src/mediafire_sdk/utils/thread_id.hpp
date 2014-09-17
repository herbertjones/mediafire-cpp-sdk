//
//  thread_id.hpp
//
//
//  Created by Zach Marquez on 2/12/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
/// @file
/// Contains types and functions related to native thread ids.
#pragma once

#if defined(__APPLE__) || defined(__linux__)
#  include <pthread.h>
#elif defined(_WIN32)
#  include <windows.h>
#endif

#include <string>

namespace mf {
namespace utils {

/**
 * A native thread identifier. This should be considered an opaque type and
 * should only be used for the following purposes:
 *   + Output via stream insertion operator
 *   + Comparison to other ThreadIds
 *     - Two StreamIds will compare equal if and only if they were generated
 *       by calls to GetCurrentThreadId() from the same thread.
 */
#if defined(__APPLE__)
typedef mach_port_t ThreadId;
#elif defined(_WIN32)
typedef DWORD ThreadId;
#elif defined(__linux__)
typedef pthread_t ThreadId;
#else
#  error "Unsupported platform"
#endif

/**
 * Returns the native thread identifier for the current thread of execution.
 * This identifier should only be used for debugging purposes and will be one of
 * the OSes native types. This value should NOT be used for generic programming,
 */
ThreadId GetCurrentThreadId();

/**
 * Sets the name of the current thread. On OSX, this will set the thread name
 * so that crash reports will show it. On Windows, this will set the thread name
 * in any attached debuggers.
 *
 * @warning This function cannot be called before main() has been entered or
 *          after main() has exited.
 *
 * @param[in] thread_name The name that we wish to set the current thread name
 *                        to. This name should be less than 16 characters if you
 *                        want GetCurrentThreadName() to return the exact name
 *                        assigned because some OS's truncate at 16 characters.
 */
void SetCurrentThreadName(const std::string& thread_name);

/**
 * Returns the name of the thread.
 *
 * @warning This function cannot be called before main() has been entered or
 *          after main() has exited.
 *
 * \return The name of the current thread.
 */
std::string GetCurrentThreadName();

}  // namespace utils
}  // namespace mf

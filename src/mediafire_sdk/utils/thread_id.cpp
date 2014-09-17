//
//  thread_id.cpp
//
//
//  Created by Zach Marquez on 2/12/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
/// @file
/// Contains the implementation of thread identification functions.
#include "thread_id.hpp"

#include <string>

#include "boost/thread/tss.hpp"

#include "thread_id_impl.hpp"

namespace mf {
namespace utils {

namespace detail {

#ifdef _WIN32
// Windows does not support thread names natively, so we store it in thread-
// local data.
struct ThreadIdData
{
    std::string name;
};
boost::thread_specific_ptr<ThreadIdData> g_thread_id_data;
#endif

}  // namespace detail

ThreadId GetCurrentThreadId() {
#if defined(__APPLE__)
    return pthread_mach_thread_np(pthread_self());
#elif defined(__linux__)
    return pthread_self();
#elif defined(_WIN32)
    return ::GetCurrentThreadId();
#endif
}

void SetCurrentThreadName(const std::string& thread_name)
{
    detail::SetNativeThreadName(thread_name.c_str());
#ifdef _WIN32
    // Windows needs to additionally store the thread name in thread-local
    // storage
    if ( !detail::g_thread_id_data.get() )
        detail::g_thread_id_data.reset(new detail::ThreadIdData);
    detail::g_thread_id_data->name = thread_name;
#endif
}

std::string GetCurrentThreadName()
{
    std::string thread_name;
#ifdef _WIN32
    if ( detail::g_thread_id_data.get() )
        thread_name = detail::g_thread_id_data->name;
#else
    thread_name = detail::GetNativeThreadName();
#endif
    return thread_name;
}

}  // namespace utils
}  // namespace mf

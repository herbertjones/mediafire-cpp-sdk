//
//  thread_id_impl.cpp
//
//
//  Created by Zach Marquez on 2/12/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
// Contains the unix implementation of thread_id_impl.hpp.
#include "../thread_id_impl.hpp"

#include <pthread.h>

#include <string>

#define MAX_THREAD_NAME 256

namespace mf {
namespace utils {
namespace detail {

void SetNativeThreadName(const std::string& threadName)
{
    // Note: Linux man pages specify max length of 16 characters here.
#if defined(__APPLE__)
    pthread_setname_np(threadName.c_str());
#elif defined(__linux__)
    pthread_setname_np(pthread_self(), threadName.c_str());
#endif
}

std::string GetNativeThreadName()
{
    static char thread_name_buf[MAX_THREAD_NAME];
    std::string thread_name;
    
    thread_name_buf[0] = '\0';
    // Retuns a null-terminated thread name
    int res = pthread_getname_np(
            pthread_self(),
            thread_name_buf,
            MAX_THREAD_NAME
        );
    
    if ( res == 0 )
        thread_name = thread_name_buf;
    // else We could not retrieve the thread name
    
    return thread_name;
}

}  // namespace detail
}  // namespace utils
}  // namespace mf

//
//  thread_id_impl.cpp
//
//
//  Created by Zach Marquez on 2/12/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
// Contains the Windows implementation of thread_id_impl.hpp.
#include "../thread_id_impl.hpp"

#include <windows.h>

#include <string>

namespace mf {
namespace utils {
namespace detail {
#if defined(_MSC_VER)
// You can only name a thread on windows if you are building with VS and capable
// of raising SEH exceptions. This code to name thread is based off of a
// Microsoft example: http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx.
/// @note You must not compile with /EHs on Windows or the exceptions thrown
///       by SetNativeThreadName() will NOT be caught.
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#  pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;      // Must be 0x1000.
    LPCSTR szName;      // Pointer to name (in user addr space).
    DWORD  dwThreadID;  // Thread ID (-1=caller thread).
    DWORD  dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#  pragma pack(pop)

void SetNativeThreadName(const std::string& threadName)
{
    // See notes on THREADNAME_INFO struct
    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = threadName.c_str();
    info.dwThreadID = -1;
    info.dwFlags    = 0;

    // Raise exception that will instruct debugger to name our thread
    __try
    {
        RaiseException(
                MS_VC_EXCEPTION,
                0,
                sizeof(info) / sizeof(ULONG_PTR),
                reinterpret_cast<ULONG_PTR*>(&info)
            );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // Empty
    }
}

#else
// Windows cannot name a thread if not compiled with SEH support.
void SetNativeThreadName(const std::string& /*threadName*/)
{
    // This is a no-op.
}

#endif

std::string GetNativeThreadName()
{
    // Windows does not have concept of native thread name.
    return "";
}

}  // namespace detail
}  // namespace utils
}  // namespace mf

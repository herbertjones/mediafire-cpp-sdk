
include_directories("..")

add_definitions(-DTEST_USER_1_USERNAME="${TEST_USER_1_USERNAME}")
add_definitions(-DTEST_USER_1_PASSWORD="${TEST_USER_1_PASSWORD}")
add_definitions(-DTEST_USER_2_USERNAME="${TEST_USER_2_USERNAME}")
add_definitions(-DTEST_USER_2_PASSWORD="${TEST_USER_2_PASSWORD}")

# Boost json parser relies on boost spirit, which needs a define to be thread
# safe.
add_definitions(-DBOOST_SPIRIT_THREADSAFE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
IF (APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ENDIF (APPLE)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    # Path to Windows Sockets
    find_library(WINSOCK_LIBRARY NAMES ws2_32 DOC "Path to WinSock library (WS2_32.lib)")
    if( NOT WINSOCK_LIBRARY STREQUAL "WINSOCK_LIBRARY-NOTFOUND" )
        mark_as_advanced(WINSOCK_LIBRARY)
    endif()

    # Path to mswsock
    find_library(MSWSOCK_LIBRARY NAMES mswsock DOC "Path to other winsock library? (mswsock.dll)")
        if( NOT MSWSOCK_LIBRARY STREQUAL "MSWSOCK_LIBRARY-NOTFOUND" )
        mark_as_advanced(MSWSOCK_LIBRARY)
    endif()

    find_library(LIPHLAPI_LIBRARY NAMES IPHLPAPI DOC "Path to IPHLPAPI library (liphlpapi)")
    if( NOT LIPHLAPI_LIBRARY STREQUAL "LIPHLAPI_LIBRARY-NOTFOUND" )
        mark_as_advanced(LIPHLAPI_LIBRARY)
    endif()

    # PTHREADS
    SET(PTHREADS_INCLUDE_DIR  "" CACHE PATH "Path to pthread includes (Optional).")
    SET(PTHREADS_LIBRARY_DIR  "" CACHE PATH "Path to pthread library (Optional).")
endif()

add_subdirectory(utils)
add_subdirectory(http)
add_subdirectory(api)
add_subdirectory(uploader)

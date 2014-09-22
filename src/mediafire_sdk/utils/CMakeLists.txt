add_definitions("-DDI_MODULE_NAME=\"utils\"")
add_subdirectory(unit_tests)

if(WIN32)
  set(OS_DIR "win")
elseif(UNIX)
  set(OS_DIR "unix")
else(WIN32)
  message(FATAL_ERROR "Not windows or unix")
endif(WIN32)

set(UTILS_SOURCES
    error/codes/fileio.cpp

    base64.cpp
    fileio.cpp
    io_service_connection.cpp
    md5_hasher.cpp
    thread_id.cpp
    sha1_hasher.cpp
    sha256_hasher.cpp
    string.cpp
    time.cpp
    url_encode.cpp
    ${OS_DIR}/thread_id_impl.cpp
)
set(UTILS_HEADERS
    error/codes/fileio.hpp

    base64.hpp
    fileio.hpp
    io_service_connection.hpp
    io_service_connection-inl.hpp
    io_service_connection_helper.hpp
    md5_hasher.hpp
    mutex.hpp
    thread_id.hpp
    thread_id_impl.hpp
    timed_actions.hpp
    singleton.hpp
    sha1_hasher.hpp
    sha256_hasher.hpp
    string.hpp
    time.hpp
    url_encode.hpp
)

add_library(mf_sdk_utils STATIC
    ${UTILS_HEADERS}
    ${UTILS_SOURCES}
)

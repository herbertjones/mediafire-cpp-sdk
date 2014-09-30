
if (WIN32 AND NOT DEFINED MINGW)
    # We are using the full path of the file to find the template directory.
    # This changes __FILE__ to have the full path on Windows.
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")
endif(WIN32 AND NOT DEFINED MINGW)

# Unit test usernames/passwords
add_definitions(-DTEST_USER_1_USERNAME="${TEST_USER_1_USERNAME}")
add_definitions(-DTEST_USER_1_PASSWORD="${TEST_USER_1_PASSWORD}")
add_definitions(-DTEST_USER_2_USERNAME="${TEST_USER_2_USERNAME}")
add_definitions(-DTEST_USER_2_PASSWORD="${TEST_USER_2_PASSWORD}")

# --- session token server ---------------------------------

set(ST_SERVER_SOURCES
    api_ut_helpers.cpp
    session_token_test_server.cpp
    session_token_test_connection.cpp
    session_token_test_request.cpp
)
set(ST_SERVER_HEADERS
    api_ut_helpers.hpp
    session_token_test_server.hpp
    session_token_test_connection.hpp
    session_token_test_request.hpp
)

add_library(session_token_server STATIC
    ${ST_SERVER_HEADERS}
    ${ST_SERVER_SOURCES}
)

TARGET_LINK_LIBRARIES(session_token_server
    ${MFD_API_APP_CONSTANTS_LIBRARY}
    api_error
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${PTHREAD_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
)

# --- ut_api -----------------------------------------------
add_executable(ut_api
    api_ut_helpers.cpp
    ut_api.cpp)

target_link_libraries(ut_api
    mf_api_sdk
    ut_expect_server
)

add_test(ut_api
    ut_api "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- ut_session_maintainer --------------------------
add_executable(ut_session_maintainer
    ut_session_maintainer.cpp)

target_link_libraries(ut_session_maintainer
    mf_api_sdk
    session_token_server
    ut_expect_server
)

add_test(ut_session_maintainer
    ut_session_maintainer "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- ut_multithread ---------------------------------------
add_executable(ut_multithread
    ut_multithread.cpp)

target_link_libraries(ut_multithread
    mf_api_sdk
)

add_test(ut_multithread
    ut_multithread "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- live -------------------------------------------------
add_executable(ut_live
    ut_live.cpp)

target_link_libraries(ut_live
    mf_api_sdk
    session_token_server
    ut_expect_server
)

# Don't run these automatically.
#add_test(ut_live
#    ut_live "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
#)

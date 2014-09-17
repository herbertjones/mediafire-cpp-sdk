
add_library(ut_expect_server
    expect_server_base.cpp
    expect_server.cpp
    expect_server_ssl.cpp
)

# --- ut_url -----------------------------------------------
add_executable(ut_url ut_url.cpp)

target_link_libraries(ut_url
    mf_http_sdk
    ut_expect_server
    ${Boost_LIBRARIES}
)

add_test(ut_url
    ut_url "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- ut_http_request -----------------------------------------------
add_executable(ut_http_request ut_http_request.cpp)

target_link_libraries(ut_http_request
    mf_http_sdk
    ut_expect_server
    ${Boost_LIBRARIES}
)

add_test(ut_http_request
    ut_http_request "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)


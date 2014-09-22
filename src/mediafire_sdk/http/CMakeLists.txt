cmake_minimum_required(VERSION 2.8)

project (mf_http_sdk CXX)

add_subdirectory(error)

set(HTTP_LIBRARY_SOURCES
    http_config.cpp
    http_request.cpp
    url.cpp
)
set(HTTP_LIBRARY_HEADERS
    detail/default_http_headers.hpp
    detail/default_pem.hpp
    detail/http_request_events.hpp
    detail/http_request_state_machine.hpp
    detail/race_preventer.hpp
    detail/socket_wrapper.hpp
    detail/transition_config.hpp
    detail/transition_initialize.hpp
    detail/transition_resolve_host.hpp

    bandwidth_analyser_interface.hpp
    buffer_interface.hpp
    error.hpp
    headers.hpp
    http_config.hpp
    http_request.hpp
    post_data_pipe_interface.hpp
    request_response_interface.hpp
    shared_buffer.hpp
    url.hpp
    )

add_definitions(
    -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
    -DBOOST_MPL_LIMIT_VECTOR_SIZE=40 # Set max MSM transitions
    -DBOOST_MPL_LIMIT_MAP_SIZE=40 # Set max MSM transitions
    -DFUSION_MAX_VECTOR_SIZE=20 # Set max MSM states
)

add_library(mf_http_sdk STATIC
    ${HTTP_LIBRARY_HEADERS}
    ${HTTP_LIBRARY_SOURCES}
)

TARGET_LINK_LIBRARIES(mf_http_sdk
    ${WINSOCK_LIBRARY}
    ${MSWSOCK_LIBRARY}
    http_error
    mf_sdk_utils
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PTHREAD_LIBRARY}
    ${ZLIB_LIBRARIES}
)

add_subdirectory(standalone)
add_subdirectory(unit_tests)


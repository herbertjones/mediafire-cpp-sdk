cmake_minimum_required(VERSION 2.8)

add_subdirectory(error)

# -- API by path ---------------
add_subdirectory(billing)
add_subdirectory(contact)
add_subdirectory(device)
add_subdirectory(file)
add_subdirectory(folder)
add_subdirectory(notifications)
add_subdirectory(system)
add_subdirectory(upload)
add_subdirectory(user)

project (mf_api_sdk CXX)

set(API_SOURCES
    ${API_SOURCES}
    app_constants.cpp
    connection_state.cpp
    credentials.cpp
    error.cpp
    ptree_helpers.cpp
    response_base.cpp
    session_maintainer.cpp
    session_state.cpp
    session_token_api_base.cpp
    tokenless_api_base.cpp
    type_helpers.cpp
)
set(API_HEADERS
    ${API_HEADERS}
    api_base.hpp
    app_constants.hpp
    config.hpp
    connection_state.hpp
    credentials.hpp
    error.hpp
    ptree_helpers.hpp
    request_interface.hpp
    requester.hpp
    requester_impl.hpp
    response_base.hpp
    session_maintainer.hpp
    session_maintainer_request.hpp
    session_state.hpp
    session_token_api_base.hpp
    tokenless_api_base.hpp
    type_helpers.hpp
)

add_library(mf_api_sdk STATIC
    ${API_HEADERS}
    ${API_SOURCES}
)

TARGET_LINK_LIBRARIES(mf_api_sdk
    ${MFD_API_APP_CONSTANTS_LIBRARY}
    api_error
    mf_sdk_utils
    mf_http_sdk
)

add_subdirectory(unit_tests)

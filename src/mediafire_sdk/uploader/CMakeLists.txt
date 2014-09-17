cmake_minimum_required(VERSION 2.8)

# TODO
#add_subdirectory(error)

project (mf_uploader_sdk CXX)

set(MF_UPLOADER_SOURCES
    detail/hasher_transitions.cpp
    detail/stepping.cpp
    detail/upload_manager_impl.cpp

    error/poll_file_error.cpp
    error/poll_result.cpp
    error/upload_response.cpp
    error/uploader_category.cpp

    hasher.cpp
    upload_manager.cpp
    upload_request.cpp
)
set(MF_UPLOADER_HEADERS
    detail/hasher_events.hpp
    detail/hasher_transitions.hpp
    detail/stepping.hpp
    detail/transition_check.hpp
    detail/transition_instant_upload.hpp
    detail/transition_upload.hpp
    detail/transitions.hpp
    detail/types.hpp
    detail/upload_events.hpp
    detail/upload_manager_impl.hpp
    detail/upload_state_machine.hpp
    detail/upload_target.hpp

    error/poll_file_error.cpp
    error/poll_result.hpp
    error/upload_response.hpp
    error/uploader_category.hpp

    error.hpp
    hasher.hpp
    upload_manager.hpp
    upload_modification.hpp
    upload_request.hpp
    upload_status.hpp
)

add_definitions(
    -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
    -DBOOST_MPL_LIMIT_VECTOR_SIZE=40 # Set max MSM transitions
    -DBOOST_MPL_LIMIT_MAP_SIZE=40 # Set max MSM transitions
    -DFUSION_MAX_VECTOR_SIZE=20 # Set max MSM states
)

add_library(mf_uploader_sdk STATIC
    ${MF_UPLOADER_HEADERS}
    ${MF_UPLOADER_SOURCES}
)

TARGET_LINK_LIBRARIES(mf_uploader_sdk
    ${MFD_API_APP_CONSTANTS_LIBRARY}
    api_error
    mf_sdk_utils
    mf_http_sdk
    mf_api_sdk
)

add_subdirectory(unit_tests)

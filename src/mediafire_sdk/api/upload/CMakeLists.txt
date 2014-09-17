
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/pre_upload.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/poll_upload.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/check.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/instant.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/pre_upload.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/poll_upload.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/check.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/instant.hpp
    PARENT_SCOPE
)


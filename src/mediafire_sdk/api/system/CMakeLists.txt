
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_limits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_status.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_limits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_status.hpp
    PARENT_SCOPE
)


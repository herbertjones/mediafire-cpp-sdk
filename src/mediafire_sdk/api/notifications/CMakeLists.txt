
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/peek_cache.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_cache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/peek_cache.hpp
    PARENT_SCOPE
)


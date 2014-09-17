
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/copy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/create.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_delete.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_links.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/move.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/update.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/copy.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/create.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_delete.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_links.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/move.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/update.hpp
    PARENT_SCOPE
)


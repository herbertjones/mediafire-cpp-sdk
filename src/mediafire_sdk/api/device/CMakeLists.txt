
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_changes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_foreign_resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_patch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_resource_shares.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_status.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_updates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_user_shares.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/share_resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unshare_resource.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/get_changes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_foreign_resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_patch.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_resource_shares.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_updates.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_user_shares.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/share_resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unshare_resource.hpp
    PARENT_SCOPE
)



set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/destroy_action_token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_action_token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_limits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_login_token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_session_token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/link_facebook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/link_twitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renew_session_token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unlink_facebook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unlink_twitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/user_register.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/destroy_action_token.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_action_token.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_limits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_login_token.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_session_token.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/link_facebook.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/link_twitter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renew_session_token.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unlink_facebook.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unlink_twitter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/user_register.hpp
    PARENT_SCOPE
)


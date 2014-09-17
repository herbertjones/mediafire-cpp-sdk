cmake_minimum_required(VERSION 2.8)

project (app_constants CXX)

set(APP_CONSTANTS_SOURCES
    application_key.cpp
)
set(APP_CONSTANTS_HEADERS
    application_key.cpp
)

add_library(app_constants STATIC
    ${APP_CONSTANTS_HEADERS}
    ${APP_CONSTANTS_SOURCES}
)

TARGET_LINK_LIBRARIES(app_constants
    utils
)


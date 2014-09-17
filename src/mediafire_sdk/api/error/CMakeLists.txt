
set(API_ERROR_HEADERS
    api_category.hpp
    api_condition.hpp
)
set(API_ERROR_SOURCES
    api_category.cpp
    api_condition.cpp
)

add_library(api_error STATIC
    ${API_ERROR_HEADERS}
    ${API_ERROR_SOURCES}
)

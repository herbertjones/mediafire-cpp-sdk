
set(HTTP_LIB_ERROR_HEADERS
    category_impl.hpp
    codes.hpp
    error_impl.hpp
)
set(HTTP_LIB_ERROR_SOURCES
    category_impl.cpp
    error_impl.cpp
)

add_library(http_error STATIC
    ${HTTP_LIB_ERROR_HEADERS}
    ${HTTP_LIB_ERROR_SOURCES}
)

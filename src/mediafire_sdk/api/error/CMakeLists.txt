
set(API_ERROR_HEADERS
    codes/api_code.hpp
    codes/http_status.hpp
    codes/result_code.hpp
    conditions/generic.hpp
)
set(API_ERROR_SOURCES
    codes/api_code.cpp
    codes/http_status.cpp
    codes/result_code.cpp
    conditions/generic.cpp
)

add_library(api_error STATIC
    ${API_ERROR_HEADERS}
    ${API_ERROR_SOURCES}
)

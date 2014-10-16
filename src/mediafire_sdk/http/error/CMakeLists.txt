
set(HTTP_LIB_ERROR_HEADERS
    error_impl.hpp
)
set(HTTP_LIB_ERROR_SOURCES
    error_impl.cpp
)

add_library(http_error OBJECT
    ${HTTP_LIB_ERROR_HEADERS}
    ${HTTP_LIB_ERROR_SOURCES}
)

foreach( file ${HTTP_LIB_ERROR_HEADERS} )
    get_filename_component( dir ${file} DIRECTORY )
    install(FILES ${file} DESTINATION include/mediafire_sdk/http/error/${dir} )
endforeach()

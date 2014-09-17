
add_executable(http_get
    http_get.cpp
)

target_link_libraries(http_get
    mf_http_sdk
    ${PTHREAD_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
)


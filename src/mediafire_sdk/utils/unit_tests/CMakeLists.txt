# --- ut_io_service_connection --------------------------
add_executable(ut_io_service_connection
    ut_io_service_connection.cpp
)
target_link_libraries(ut_io_service_connection
    ${WINSOCK_LIBRARY}
    ${MSWSOCK_LIBRARY}
    mf_sdk_utils
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_io_service_connection ut_io_service_connection)

# --- ut_md5_hasher -------------------------------------
add_executable(ut_md5_hasher
    ut_md5_hasher.cpp
)
target_link_libraries(ut_md5_hasher
    ${WINSOCK_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_md5_hasher ut_md5_hasher)

# --- ut_sha1_hasher ------------------------------------
add_executable(ut_sha1_hasher
    ut_sha1_hasher.cpp
)
target_link_libraries(ut_sha1_hasher
    ${WINSOCK_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_sha1_hasher ut_sha1_hasher)

# --- ut_sha256_hasher ----------------------------------
add_executable(ut_sha256_hasher
    ut_sha256_hasher.cpp
)
target_link_libraries(ut_sha256_hasher
    ${WINSOCK_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_sha256_hasher ut_sha256_hasher)

# --- ut_base64 -----------------------------------------
add_executable(ut_base64
    ut_base64.cpp
)
target_link_libraries(ut_base64
    ${WINSOCK_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_base64 ut_base64)

# --- ut_url_encode -------------------------------------
add_executable(ut_url_encode
    ut_url_encode.cpp
)
target_link_libraries(ut_url_encode
    ${WINSOCK_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PTHREAD_LIBRARY}
)
add_test(ut_url_encode ut_url_encode)


# Unit test usernames/passwords
add_definitions(-DTEST_USER_1_USERNAME="${TEST_USER_1_USERNAME}")
add_definitions(-DTEST_USER_1_PASSWORD="${TEST_USER_1_PASSWORD}")
add_definitions(-DTEST_USER_2_USERNAME="${TEST_USER_2_USERNAME}")
add_definitions(-DTEST_USER_2_PASSWORD="${TEST_USER_2_PASSWORD}")

# --- ut_hasher --------------------------------------------
add_executable(ut_hasher
    ut_hasher.cpp
)

target_link_libraries(ut_hasher
    mf_api_sdk
    mf_uploader_sdk
    ${Boost_LIBRARIES}
)

add_test(ut_hasher ut_hasher)

# --- ut_uploader_live -------------------------------------
add_executable(ut_uploader_live
    ut_uploader_live.cpp
)

target_link_libraries(ut_uploader_live
    mf_api_sdk
    mf_uploader_sdk
    ${Boost_LIBRARIES}
)

# --- hash_file --------------------------------------------
add_executable(hash_file
    hash_file.cpp
)

target_link_libraries(hash_file
    mf_api_sdk
    mf_uploader_sdk
    ${Boost_LIBRARIES}
)

# --- upload_file ------------------------------------------
add_executable(upload_file
    upload_file.cpp
)

target_link_libraries(upload_file
    mf_api_sdk
    mf_uploader_sdk
    ${Boost_LIBRARIES}
)


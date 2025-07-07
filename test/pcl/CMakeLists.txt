set(BASE_TEST_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/filter_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/knn_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/norm_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/features_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/descriptors_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/correspondence_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/correspondence_sorter_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/registration_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/prosac_registration_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fine_registration_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fine_registration_debug_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fine_registration_transform_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/icp_simple_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aa_icp_debug_test.cpp
)

list(APPEND TEST_FILES ${BASE_TEST_FILES})
set(TEST_FILES ${TEST_FILES} PARENT_SCOPE)

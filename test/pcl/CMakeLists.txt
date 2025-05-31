set(BASE_TEST_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/filter_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/knn_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/norm_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/features_test.cpp
)

list(APPEND TEST_FILES ${BASE_TEST_FILES})
set(TEST_FILES ${TEST_FILES} PARENT_SCOPE)

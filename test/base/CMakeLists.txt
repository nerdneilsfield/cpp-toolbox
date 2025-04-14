# add file to TEST_FILES

set(BASE_TEST_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/env_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool_singleton_test.cpp
)

list(APPEND TEST_FILES ${BASE_TEST_FILES})
set(TEST_FILES ${TEST_FILES} PARENT_SCOPE)
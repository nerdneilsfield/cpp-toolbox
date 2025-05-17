set(BASE_TEST_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/timer_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/click_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/random_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ini_config_test.cpp
)

list(APPEND TEST_FILES ${BASE_TEST_FILES})
set(TEST_FILES ${TEST_FILES} PARENT_SCOPE)

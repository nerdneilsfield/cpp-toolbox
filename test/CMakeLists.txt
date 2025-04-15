cmake_minimum_required(VERSION 3.14)

project(cpp-toolboxTests LANGUAGES CXX C)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# # ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(cpp-toolbox REQUIRED)
  enable_testing()
endif()

# # ---- Tests ----

# add_executable(cpp-toolbox_test source/cpp-toolbox_test.cpp)
# target_link_libraries(cpp-toolbox_test PRIVATE cpp-toolbox::cpp-toolbox)
# target_compile_features(cpp-toolbox_test PRIVATE cxx_std_17)

# add_test(NAME cpp-toolbox_test COMMAND cpp-toolbox_test)

add_library(cpp-toolbox_test_lib INTERFACE)
target_compile_features(cpp-toolbox_test_lib INTERFACE cxx_std_17)
if(WIN32)
  target_link_libraries(cpp-toolbox_test_lib INTERFACE cpp-toolbox::cpp-toolbox_static)
  target_compile_options(cpp-toolbox_test_lib INTERFACE /std:c++17 /Zc:__cplusplus)
else()
  target_link_libraries(cpp-toolbox_test_lib INTERFACE cpp-toolbox::cpp-toolbox)
endif()


# add a list to contain files
set(TEST_FILES)

# # ---- End-of-file commands ----

add_subdirectory(base)
add_subdirectory(file)
add_subdirectory(logger)
add_subdirectory(container)
add_subdirectory(concurrent)
add_subdirectory(utils)
add_subdirectory(functional)
add_subdirectory(types)

list(APPEND TEST_FILES my_catch2_main.cpp)

add_executable(cpp-toolbox_test ${TEST_FILES})
target_link_libraries(cpp-toolbox_test PRIVATE cpp-toolbox_test_lib Catch2WithMain)
target_compile_features(cpp-toolbox_test PRIVATE cxx_std_17)
target_precompile_headers(cpp-toolbox_test PRIVATE ${CPP_TOOLBOX_PCH_FILE})
# target_compile_definitions(cpp-toolbox_test PRIVATE CATCH_CONFIG_MAIN)
add_test(NAME cpp-toolbox_test COMMAND cpp-toolbox_test)

add_folders(Test)

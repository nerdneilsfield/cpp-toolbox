cmake_minimum_required(VERSION 3.14)

project(cpp-toolbox-newTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(cpp-toolbox-new REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(cpp-toolbox-new_test source/cpp-toolbox-new_test.cpp)
target_link_libraries(cpp-toolbox-new_test PRIVATE cpp-toolbox-new::cpp-toolbox-new)
target_compile_features(cpp-toolbox-new_test PRIVATE cxx_std_17)

add_test(NAME cpp-toolbox-new_test COMMAND cpp-toolbox-new_test)

# ---- End-of-file commands ----

add_folders(Test)

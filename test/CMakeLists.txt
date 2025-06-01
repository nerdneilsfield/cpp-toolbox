cmake_minimum_required(VERSION 3.14)

project(cpp-toolboxTests LANGUAGES CXX C)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# # ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(cpp-toolbox REQUIRED)
  enable_testing()
endif()

include(CTest)

set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
get_filename_component(TEST_DATA_DIR ${TEST_DATA_DIR} ABSOLUTE)

configure_file(test_data_dir.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test_data_dir.hpp)


# # ---- Tests ----

# add_executable(cpp-toolbox_test source/cpp-toolbox_test.cpp)
# target_link_libraries(cpp-toolbox_test PRIVATE cpp-toolbox::cpp-toolbox)
# target_compile_features(cpp-toolbox_test PRIVATE cxx_std_17)

# add_test(NAME cpp-toolbox_test COMMAND cpp-toolbox_test)

add_library(cpp-toolbox_test_lib INTERFACE)
target_compile_features(cpp-toolbox_test_lib INTERFACE cxx_std_17)
target_include_directories(cpp-toolbox_test_lib INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
if(WIN32 OR MSVC OR USE_ZIG)
  target_link_libraries(cpp-toolbox_test_lib INTERFACE cpp-toolbox::cpp-toolbox_static nanoflann eigen)
  # target_compile_options(cpp-toolbox_test_lib INTERFACE /std:c++17 /Zc:__cplusplus)
else()
  target_link_libraries(cpp-toolbox_test_lib INTERFACE cpp-toolbox::cpp-toolbox nanoflann eigen)
endif()

if(MSVC)
  target_compile_options(cpp-toolbox_test_lib INTERFACE "/std:c++17" "/sdl" "/guard:cf" "/utf-8" "/diagnostics:caret" "/w14165" "/w44242" "/w44254" "/w44263"
    "/w34265" "/w34287" "/w44296" "/w44365" "/w44388" "/w44464" "/w14545" "/w14546" "/w14547" "/w14549"
    "/w14555" "/w34619" "/w34640" "/w24826" "/w14905" "/w14906" "/w14928" "/w45038" "/W4" "/permissive-"
    "/volatile:iso" "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda" "/Zc:__cplusplus"
    "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()
if(USE_ZIG)
  target_compile_definitions(cpp-toolbox_test_lib INTERFACE CPP_TOOLBOX_COMPILER_ZIG)
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
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(pcl)
list(APPEND TEST_FILES my_catch2_main.cpp)

add_executable(cpp-toolbox_test ${TEST_FILES})
target_link_libraries(cpp-toolbox_test PRIVATE cpp-toolbox_test_lib Catch2::Catch2WithMain)

if(CPP_TOOLBOX_USE_TBB)
  find_package(TBB REQUIRED)
  target_link_libraries(cpp-toolbox_test PRIVATE TBB::tbb)
endif()

target_compile_features(cpp-toolbox_test PRIVATE cxx_std_17)
target_precompile_headers(cpp-toolbox_test PRIVATE ${CPP_TOOLBOX_PCH_FILE})
# target_compile_definitions(cpp-toolbox_test PRIVATE CATCH_CONFIG_MAIN)
add_test(NAME cpp-toolbox_test COMMAND cpp-toolbox_test)
if(MSVC)
  target_compile_options(cpp-toolbox_test PRIVATE "/std:c++17" "/sdl" "/guard:cf" "/utf-8" "/diagnostics:caret" "/w14165" "/w44242" "/w44254" "/w44263"
    "/w34265" "/w34287" "/w44296" "/w44365" "/w44388" "/w44464" "/w14545" "/w14546" "/w14547" "/w14549"
    "/w14555" "/w34619" "/w34640" "/w24826" "/w14905" "/w14906" "/w14928" "/w45038" "/W4" "/permissive-"
    "/volatile:iso" "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda" "/Zc:__cplusplus"
    "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()

add_folders(Test)

cmake_minimum_required(VERSION 3.14)


find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # 通常推荐关闭编译器特定扩展


set(cpp-toolbox_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/base/env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container/concurrent_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/base/thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/base/thread_pool_singleton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger/thread_logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file/memory_mapped_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/point.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/click.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/pcd.cpp
)

add_library(cpp-toolbox SHARED ${cpp-toolbox_srcs})
target_compile_features(cpp-toolbox PRIVATE cxx_std_17)
add_library(cpp-toolbox::cpp-toolbox ALIAS cpp-toolbox)
include(GenerateExportHeader)
generate_export_header(cpp-toolbox
    BASE_NAME CPP_TOOLBOX
    EXPORT_FILE_NAME cpp-toolbox/cpp-toolbox_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)
target_compile_definitions(cpp-toolbox PUBLIC
    NOMINMAX
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_include_directories(cpp-toolbox PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    ${CPP_TOOLBOX_INCLUDE_DIRS}
)
set_target_properties(cpp-toolbox PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME cpp-toolbox
    OUTPUT_NAME cpp-toolbox
)
target_link_libraries(cpp-toolbox
    PRIVATE
    concurrentqueue::concurrentqueue
)
target_link_libraries(cpp-toolbox PUBLIC Threads::Threads)
target_precompile_headers(cpp-toolbox PRIVATE ${CPP_TOOLBOX_PCH_FILE})
if(MSVC AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_options(cpp-toolbox PRIVATE "/sdl" "/guard:cf" "/utf-8" "/diagnostics:caret" "/w14165" "/w44242" "/w44254" "/w44263"
    "/w34265" "/w34287" "/w44296" "/w44365" "/w44388" "/w44464" "/w14545" "/w14546" "/w14547" "/w14549"
    "/w14555" "/w34619" "/w34640" "/w24826" "/w14905" "/w14906" "/w14928" "/w45038" "/W4" "/permissive-"
    "/volatile:iso" "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda" "/Zc:__cplusplus"
    "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()


add_library(cpp-toolbox_static STATIC ${cpp-toolbox_srcs})
target_include_directories(cpp-toolbox_static PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(cpp-toolbox_static PUBLIC
    NOMINMAX
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    CPP_TOOLBOX_STATIC_DEFINE
)
target_link_libraries(cpp-toolbox_static PRIVATE concurrentqueue::concurrentqueue)
target_link_libraries(cpp-toolbox_static PUBLIC Threads::Threads)
add_library(cpp-toolbox::cpp-toolbox_static ALIAS cpp-toolbox_static)
target_precompile_headers(cpp-toolbox_static PRIVATE ${CPP_TOOLBOX_PCH_FILE})

if(MSVC AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_options(cpp-toolbox_static PRIVATE "/sdl" "/guard:cf" "/utf-8" "/diagnostics:caret" "/w14165" "/w44242" "/w44254" "/w44263"
    "/w34265" "/w34287" "/w44296" "/w44365" "/w44388" "/w44464" "/w14545" "/w14546" "/w14547" "/w14549"
    "/w14555" "/w34619" "/w34640" "/w24826" "/w14905" "/w14906" "/w14928" "/w45038" "/W4" "/permissive-"
    "/volatile:iso" "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda" "/Zc:__cplusplus"
    "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()


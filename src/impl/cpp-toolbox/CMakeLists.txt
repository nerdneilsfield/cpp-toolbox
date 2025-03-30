cmake_minimum_required(VERSION 3.14)


set(cpp-toolbox_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/base/env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger/thread_logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/container/string.cpp
)

add_library(cpp-toolbox SHARED ${cpp-toolbox_srcs})
target_compile_features(cpp-toolbox PRIVATE cxx_std_17)
add_library(cpp-toolbox::cpp-toolbox ALIAS cpp-toolbox)
include(GenerateExportHeader)
generate_export_header(cpp-toolbox
    BASE_NAME          CPP_TOOLBOX
    EXPORT_FILE_NAME   cpp-toolbox/cpp-toolbox_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)
target_compile_definitions(cpp-toolbox PUBLIC 
  NOMINMAX 
  $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_include_directories(cpp-toolbox PUBLIC  ${CPP_TOOLBOX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(cpp-toolbox PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME cpp-toolbox
    OUTPUT_NAME cpp-toolbox
)


add_library(cpp-toolbox_static STATIC ${cpp-toolbox_srcs})
target_include_directories(cpp-toolbox_static PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(cpp-toolbox_static PUBLIC
   NOMINMAX 
   $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
   CPP_TOOLBOX_STATIC_DEFINE
)
add_library(cpp-toolbox::cpp-toolbox_static ALIAS cpp-toolbox_static)



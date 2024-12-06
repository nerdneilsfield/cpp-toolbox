cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    cpp-toolbox
    VERSION 0.1.0
    DESCRIPTION "A tools box library for accelerating cpp programe developing."
    HOMEPAGE_URL "https://github.com/nerdneilsfield/cpp-toolbox"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Dependencies ----

add_subdirectory(deps)

# ---- Declare library ----
set(CPP_TOOLBOX_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/include)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# add_library(
#     cpp-toolbox_cpp-toolbox
#     src/cpp-toolbox/cpp-toolbox.cpp
# )
# add_library(cpp-toolbox::cpp-toolbox ALIAS cpp-toolbox_cpp-toolbox)

# include(GenerateExportHeader)
# generate_export_header(
#     cpp-toolbox_cpp-toolbox
#     BASE_NAME cpp-toolbox
#     EXPORT_FILE_NAME export/cpp-toolbox/cpp-toolbox_export.hpp
#     CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
# )

# if(NOT BUILD_SHARED_LIBS)
#   target_compile_definitions(cpp-toolbox_cpp-toolbox PUBLIC CPP_TOOLBOX_STATIC_DEFINE)
# endif()

# set_target_properties(
#     cpp-toolbox_cpp-toolbox PROPERTIES
#     CXX_VISIBILITY_PRESET hidden
#     VISIBILITY_INLINES_HIDDEN YES
#     VERSION "${PROJECT_VERSION}"
#     SOVERSION "${PROJECT_VERSION_MAJOR}"
#     EXPORT_NAME cpp-toolbox
#     OUTPUT_NAME cpp-toolbox
# )

# target_include_directories(
#     cpp-toolbox_cpp-toolbox ${warning_guard}
#     PUBLIC
#     "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
# )

# target_include_directories(
#     cpp-toolbox_cpp-toolbox SYSTEM
#     PUBLIC
#     "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
# )

# target_compile_features(cpp-toolbox_cpp-toolbox PUBLIC cxx_std_17)

# ---- Install rules ----

# if(NOT CMAKE_SKIP_INSTALL_RULES)
#   include(cmake/install-rules.cmake)
# endif()

# ---- Developer mode ----

if(NOT cpp-toolbox_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of cpp-toolbox"
  )
endif()

include(cmake/dev-mode.cmake)

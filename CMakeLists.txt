cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
  cpp-toolbox
  VERSION 0.1.0
  DESCRIPTION "A tools box library for accelerating cpp programe developing."
  HOMEPAGE_URL "https://github.com/nerdneilsfield/cpp-toolbox"
  LANGUAGES CXX
)

# ---- Declare library ----
set(CPP_TOOLBOX_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/include)
set(CPP_TOOLBOX_PCH_FILE ${CMAKE_SOURCE_DIR}/src/impl/cpp-toolbox/pch.hpp)

include(cmake/project-is-top-level.cmake)
include(cmake/zig.cmake)
include(cmake/variables.cmake)
include(cmake/cache.cmake)
include(cmake/output.cmake)
include(cmake/example.cmake)
include(cmake/optimization.cmake)
# ---- Dependencies ----

# check if the system is linux or macos
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_subdirectory(tools)
endif()

add_subdirectory(deps)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# ---- Install rules ----

# if(NOT CMAKE_SKIP_INSTALL_RULES)
#   include(cmake/install-rules.cmake)
# endif()


# if(NOT cpp-toolbox_DEVELOPER_MODE)
#   if(BUILD_TESTING)
#     add_subdirectory(test)
#   endif()
#   if(BUILD_BENCHMARKS)
#     add_subdirectory(benchmark)
#   endif()
#   return()
# elseif(NOT PROJECT_IS_TOP_LEVEL)
#   message(
#     AUTHOR_WARNING
#     "Developer mode is intended for developers of cpp-toolbox"
#   )
# endif()

if(NOT cpp-toolbox_DEVELOPER_MODE)
  if(PROJECT_IS_TOP_LEVEL)
    if(BUILD_TESTING)
     message(STATUS "BUILDING TESTS")
     enable_testing()
     add_subdirectory(test)
    endif()
    if(BUILD_BENCHMARKS)
     message(STATUS "BUILDING BENCHMARKS")
     add_subdirectory(benchmark)
    endif()
    return()
  else()
    message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of cpp-toolbox"
    )
    return()
  endif()
endif()

include(cmake/dev-mode.cmake)

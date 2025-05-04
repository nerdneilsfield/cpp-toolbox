include(cmake/folders.cmake)

set(CMAKE_BUILD_TYPE Debug)

if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()

# option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
# if(BUILD_MCSS_DOCS)
#   include(cmake/docs.cmake)
# endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

# include(cmake/lint-targets.cmake)
# include(cmake/spell-targets.cmake)

add_folders(Project)

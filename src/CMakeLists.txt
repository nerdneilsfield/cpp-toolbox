option(CPP_TOOLBOX_USE_TBB "Use TBB for parallelism" OFF)

if(CPP_TOOLBOX_USE_TBB)
  find_package(TBB REQUIRED)
  if(NOT TBB_FOUND)
    message(FATAL_ERROR "TBB not found, please install TBB or disable TBB usage")
  endif()
  message(STATUS "Using TBB for parallelism")
endif()

add_subdirectory(impl/cpp-toolbox)
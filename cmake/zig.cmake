option(USE_ZIG "Use Zig as the C++ compiler" OFF)

if(USE_ZIG)
  message(STATUS "Using Zig as the C++ compiler for ${ZIG_TARGET}")
#   set(CMAKE_CXX_COMPILER_ARG1 "c++ -target ${ZIG_TARGET}")
#   set(CMAKE_C_COMPILER_ARG1 "cc -target ${ZIG_TARGET}")
endif()

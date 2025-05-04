cmake_minimum_required(VERSION 3.10)
project(check_libc)

add_executable(check_libc check_libc.cpp)

# Set the output directory for the executable
set_target_properties(check_libc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
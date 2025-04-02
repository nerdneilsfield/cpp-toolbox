cmake_minimum_required(VERSION 3.14)
project(concurrentqueue VERSION 1.0.0)

add_library(${PROJECT_NAME} INTERFACE)
add_library(concurrentqueue::concurrentqueue ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
        INTERFACE 
                ${CMAKE_CURRENT_SOURCE_DIR}
)
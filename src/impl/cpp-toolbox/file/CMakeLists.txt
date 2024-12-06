file(GLOB_RECURSE cpp-toolbox_file_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(cpp-toolbox_file SHARED ${cpp-toolbox_file_srcs})
target_include_directories(cpp-toolbox_file ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::file ALIAS cpp-toolbox_file)

add_library(cpp-toolbox_file_static STATIC ${cpp-toolbox_file_srcs})
target_include_directories(cpp-toolbox_file_static ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::file_static ALIAS cpp-toolbox_file_static)

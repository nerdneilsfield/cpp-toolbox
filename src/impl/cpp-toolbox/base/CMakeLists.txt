file(GLOB_RECURSE cpp-toolbox_base_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(cpp-toolbox_base SHARED ${cpp-toolbox_base_srcs})
target_include_directories(cpp-toolbox_base ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::base ALIAS cpp-toolbox_base)

add_library(cpp-toolbox_base_static STATIC ${cpp-toolbox_base_srcs})
target_include_directories(cpp-toolbox_base_static ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::base_static ALIAS cpp-toolbox_base_static)

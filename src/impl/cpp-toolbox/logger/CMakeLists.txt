file(GLOB_RECURSE cpp-toolbox_logger_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(cpp-toolbox_logger SHARED ${cpp-toolbox_logger_srcs})
target_include_directories(cpp-toolbox_logger ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::logger ALIAS cpp-toolbox_logger)
set_target_properties(cpp-toolbox_logger PROPERTIES OUTPUT_NAME "cpp-toolbox_logger")

add_library(cpp-toolbox_logger_static STATIC ${cpp-toolbox_logger_srcs})
target_include_directories(cpp-toolbox_logger_static ${warning_guard} PUBLIC ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::logger_static ALIAS cpp-toolbox_logger_static)
set_target_properties(cpp-toolbox_logger_static PROPERTIES OUTPUT_NAME "cpp-toolbox_logger")
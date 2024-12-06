add_subdirectory(base)
add_subdirectory(file)
add_subdirectory(logger)

add_library(cpp-toolbox INTERFACE)
target_link_libraries(cpp-toolbox INTERFACE cpp-toolbox::base cpp-toolbox::file)
target_include_directories(cpp-toolbox ${warning_guard} INTERFACE ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::cpp-toolbox ALIAS cpp-toolbox)

add_library(cpp-toolbox_static INTERFACE)
target_link_libraries(cpp-toolbox_static INTERFACE cpp-toolbox::base_static cpp-toolbox::file_static)
target_include_directories(cpp-toolbox_static ${warning_guard} INTERFACE ${CPP_TOOLBOX_INCLUDE_DIRS})
add_library(cpp-toolbox::cpp-toolbox_static ALIAS cpp-toolbox_static)

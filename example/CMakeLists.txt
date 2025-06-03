# Find all .cpp files in the example directory (including subdirectories)
# Store their paths (relative to current CMakeLists.txt) in EXAMPLE_SOURCES variable
file(GLOB_RECURSE EXAMPLE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Iterate through each source file found
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    # Get filename (without path and extension) as target name
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)

    string(REGEX REPLACE "_example$" "" TARGET_SUFFIX_REMOVED "${EXAMPLE_NAME}")

    set(FINAL_TARGET_NAME "example_${TARGET_SUFFIX_REMOVED}")

    # Add an executable target for each source file
    add_executable(${FINAL_TARGET_NAME} ${EXAMPLE_SOURCE})

    # Link against cpp-toolbox library
    if(WIN32 OR MSVC OR USE_ZIG)
        target_link_libraries(${FINAL_TARGET_NAME} PRIVATE cpp-toolbox::cpp-toolbox_static)
    else()
        target_link_libraries(${FINAL_TARGET_NAME} PRIVATE cpp-toolbox::cpp-toolbox)
    endif()
    target_precompile_headers(${FINAL_TARGET_NAME} PRIVATE ${CPP_TOOLBOX_PCH_FILE})
    # Add required include directories
    target_include_directories(${FINAL_TARGET_NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/deps/eigen
        ${CMAKE_SOURCE_DIR}/deps/nanoflann)

    # (Optional) Group all examples into a folder for organization
    # set_target_properties(${FINAL_TARGET_NAME} PROPERTIES FOLDER "Examples")
    if(MSVC)
        target_compile_options(${FINAL_TARGET_NAME} PRIVATE "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda"
            "/Zc:__cplusplus" "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
    endif()

    # Print info for debugging
    message(STATUS "Added example target: ${FINAL_TARGET_NAME} from ${EXAMPLE_SOURCE}")
endforeach()
set(BENCHMARK_FILES)

list(APPEND BENCHMARK_FILES
#     ${CMAKE_SOURCE_DIR}/test/my_catch2_main.cpp
    concurrent/parallel_benc.cpp
    types/types_benc.cpp
    file/memory_mapped_file_benc.cpp
)


add_executable(cpp-toolbox_benchmark ${BENCHMARK_FILES})
target_link_libraries(cpp-toolbox_benchmark PRIVATE cpp-toolbox_test_lib Catch2WithMain)
target_include_directories(cpp-toolbox_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
target_compile_features(cpp-toolbox_benchmark PRIVATE cxx_std_17)
target_compile_definitions(cpp-toolbox_benchmark PRIVATE CATCH_CONFIG_MAIN)
target_precompile_headers(cpp-toolbox_benchmark PRIVATE ${CPP_TOOLBOX_PCH_FILE})

if(MSVC)
    target_compile_options(cpp-toolbox_benchmark PRIVATE "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda"
        "/Zc:__cplusplus" "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()
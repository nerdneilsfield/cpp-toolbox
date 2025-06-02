set(BENCHMARK_FILES)

list(APPEND BENCHMARK_FILES
#     ${CMAKE_SOURCE_DIR}/test/my_catch2_main.cpp
    concurrent/parallel_benc.cpp
    types/types_benc.cpp
    file/memory_mapped_file_benc.cpp
    pcl/downsampling_benc.cpp
    pcl/knn_benc.cpp
    pcl/norm_benc.cpp
    pcl/features_benc.cpp
    pcl/descriptors_benc.cpp
    pcl/correspondence_benc.cpp
    ${CMAKE_SOURCE_DIR}/test/my_catch2_main.cpp
)


add_executable(cpp-toolbox_benchmark ${BENCHMARK_FILES})
target_link_libraries(cpp-toolbox_benchmark PRIVATE cpp-toolbox_test_lib Catch2WithMain)

if(CPP_TOOLBOX_USE_TBB)
  find_package(TBB REQUIRED)
  target_link_libraries(cpp-toolbox_benchmark PRIVATE TBB::tbb)
endif()
target_include_directories(cpp-toolbox_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
target_compile_features(cpp-toolbox_benchmark PRIVATE cxx_std_17)
target_compile_definitions(cpp-toolbox_benchmark PRIVATE CATCH_CONFIG_MAIN)
target_precompile_headers(cpp-toolbox_benchmark PRIVATE ${CPP_TOOLBOX_PCH_FILE})

if(MSVC)
    target_compile_options(cpp-toolbox_benchmark PRIVATE "/Zc:inline" "/Zc:preprocessor" "/Zc:enumTypes" "/Zc:lambda"
        "/Zc:__cplusplus" "/Zc:externConstexpr" "/Zc:throwingNew" "/EHsc")
endif()

# Debug test executable
add_executable(descriptors_debug pcl/descriptors_debug.cpp)
target_link_libraries(descriptors_debug PRIVATE cpp-toolbox)
if(CPP_TOOLBOX_USE_TBB)
  target_link_libraries(descriptors_debug PRIVATE TBB::tbb)
endif()
target_include_directories(descriptors_debug PRIVATE 
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/deps/nanoflann
  ${CMAKE_SOURCE_DIR}/deps/eigen)
target_compile_features(descriptors_debug PRIVATE cxx_std_17)

# FPFH vs PFH comparison
add_executable(fpfh_vs_pfh_test pcl/fpfh_vs_pfh_test.cpp)
target_link_libraries(fpfh_vs_pfh_test PRIVATE cpp-toolbox)
if(CPP_TOOLBOX_USE_TBB)
  target_link_libraries(fpfh_vs_pfh_test PRIVATE TBB::tbb)
endif()
target_include_directories(fpfh_vs_pfh_test PRIVATE 
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/deps/nanoflann
  ${CMAKE_SOURCE_DIR}/deps/eigen)
target_compile_features(fpfh_vs_pfh_test PRIVATE cxx_std_17)
add_executable(file_test file_test.cpp)
target_link_libraries(file_test PRIVATE cpp-toolbox::file Catch2WithMain)
target_compile_features(file_test PRIVATE cxx_std_17)
add_test(NAME file_test COMMAND file_test)

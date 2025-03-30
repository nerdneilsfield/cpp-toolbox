add_executable(string_test string_test.cpp)
target_link_libraries(string_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
target_compile_features(string_test PRIVATE cxx_std_17)
add_test(NAME string_test COMMAND string_test)
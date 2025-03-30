add_executable(env_test env_test.cpp)
target_link_libraries(env_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
target_compile_features(env_test PRIVATE cxx_std_17)
add_test(NAME env_test COMMAND env_test)
add_executable(env_test env_test.cpp)
target_link_libraries(env_test PRIVATE cpp-toolbox_test_lib Catch2WithMain)
target_compile_features(env_test PRIVATE cxx_std_17)
add_test(NAME env_test COMMAND env_test)

add_executable(thread_pool_test thread_pool_test.cpp)
target_link_libraries(thread_pool_test PRIVATE cpp-toolbox_test_lib Catch2WithMain)
target_compile_features(thread_pool_test PRIVATE cxx_std_17)
add_test(NAME thread_pool_test COMMAND thread_pool_test)

add_executable(thread_pool_singleton_test thread_pool_singleton_test.cpp)
target_link_libraries(thread_pool_singleton_test PRIVATE cpp-toolbox_test_lib Catch2WithMain)
target_compile_features(thread_pool_singleton_test PRIVATE cxx_std_17)
add_test(NAME thread_pool_singleton_test COMMAND thread_pool_singleton_test)

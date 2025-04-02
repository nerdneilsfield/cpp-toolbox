add_executable(string_test string_test.cpp)
target_link_libraries(string_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
target_compile_features(string_test PRIVATE cxx_std_17)
add_test(NAME string_test COMMAND string_test)

# add_executable(lock_free_queue_test lock_free_queue_test.cpp)
# target_link_libraries(lock_free_queue_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
# target_compile_features(lock_free_queue_test PRIVATE cxx_std_17)
# add_test(NAME lock_free_queue_test COMMAND lock_free_queue_test)

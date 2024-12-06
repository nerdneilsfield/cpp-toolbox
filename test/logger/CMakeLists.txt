add_executable(thread_logger_test thread_logger_test.cpp)
target_link_libraries(thread_logger_test PRIVATE cpp-toolbox::logger Catch2WithMain)
target_compile_features(thread_logger_test PRIVATE cxx_std_17)
# NOTICE: this is a workaround for the macro PROJECT_SOURCE_DIR not being defined
# in the test environment.
target_compile_definitions(thread_logger_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME thread_logger_test COMMAND thread_logger_test)

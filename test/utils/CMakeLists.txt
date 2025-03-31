add_executable(timer_test timer_test.cpp)
target_link_libraries(timer_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
target_compile_features(timer_test PRIVATE cxx_std_17)
# NOTICE: this is a workaround for the macro PROJECT_SOURCE_DIR not being defined
# in the test environment.
target_compile_definitions(timer_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME timer_test COMMAND timer_test)

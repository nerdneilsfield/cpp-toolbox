add_executable(functional_test functional_test.cpp)
target_link_libraries(functional_test PRIVATE cpp-toolbox::cpp-toolbox Catch2WithMain)
target_compile_features(functional_test PRIVATE cxx_std_17)
# NOTICE: this is a workaround for the macro PROJECT_SOURCE_DIR not being defined
# in the test environment.
target_compile_definitions(functional_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME functional_test COMMAND functional_test)

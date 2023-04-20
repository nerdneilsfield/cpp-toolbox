# define a macro add exeutable and add tests
macro(add_exe_and_test exe_name)
    add_executable(${exe_name} ${exe_name}.cpp)
    target_link_libraries(${exe_name} ${GTEST_BOTH_LIBRARIES} pthread)
    add_test(test_${exe_name} ${exe_name})
endmacro()

# add tests
add_exe_and_test(logger_test)
add_exe_and_test(str_test)
add_exe_and_test(env_test)
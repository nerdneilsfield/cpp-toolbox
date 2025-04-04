if(BUILD_TESTING)
  message(STATUS "BUILDING TESTS")
  add_subdirectory(Catch2)
  if(TARGET Catch2)
    message(STATUS "Disabling CMAKE_CXX_COMPILER_LAUNCHER for target: Catch2")
    # 清除编译和链接命令的 Launcher 属性
    set_target_properties(Catch2 PROPERTIES
      RULE_LAUNCH_COMPILE ""
      RULE_LAUNCH_LINK ""
    )
    # 以防万一，再次尝试清除 clang-tidy 属性
    set_target_properties(Catch2 PROPERTIES CMAKE_CXX_CLANG_TIDY "")
  else()
    message(WARNING "Target 'Catch2' not found. Cannot disable launcher/tidy.")
    # 你需要根据实际情况找到 Catch2 库目标的确切名字
  endif()
endif()

add_subdirectory(concurrentqueue)

# set(FMT_HEADERS "")
# set(FMT_MODULE OFF)
# set(FMT_INSTALL OFF)
# set(FMT_TEST OFF)
# set(FMT_FUZZ OFF)
# set(FMT_CUDA_TEST OFF)
# set(FMT_OS ON)
# add_subdirectory(fmt)
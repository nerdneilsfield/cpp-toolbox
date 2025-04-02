option(ENABLE_CCACHE "Enable compiler cache (ccache) if found" ON) # 添加一个选项，允许用户开关
if(ENABLE_CCACHE)
    find_program(CCACHE_EXECUTABLE scache HINTS /usr/lib/scache /usr/local/lib/scache $ENV{HOME}/.local/bin $ENV{HOME}/.cargo/bin) # 查找 ccache 可执行文件, HINTS 可选
    if(NOT CCACHE_EXECUTABLE)
        find_program(CCACHE_EXECUTABLE ccache HINTS /usr/lib/ccache /usr/local/lib/ccache $ENV{HOME}/.local/bin)
    endif()
    if(CCACHE_EXECUTABLE)
        message(STATUS "Compiler cache enabled: Using ccache at ${CCACHE_EXECUTABLE}")
        # 设置 CMake 使用 ccache 来启动 C 和 C++ 编译器
        # 使用 CACHE STRING 和 FORCE 确保设置写入缓存并覆盖旧值
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_EXECUTABLE}" CACHE STRING "C compiler launcher" FORCE)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE STRING "C++ compiler launcher" FORCE)
    else()
        message(WARNING "ENABLE_CCACHE is ON but ccache executable was not found. Compiler cache disabled.")
        # 如果找不到 ccache，确保启动器变量为空
        unset(CMAKE_C_COMPILER_LAUNCHER CACHE)
        unset(CMAKE_CXX_COMPILER_LAUNCHER CACHE)
    endif()
else()
    message(STATUS "Compiler cache disabled (ENABLE_CCACHE is OFF).")
    # 如果 ccache 被禁用，确保启动器变量为空
    unset(CMAKE_C_COMPILER_LAUNCHER CACHE)
    unset(CMAKE_CXX_COMPILER_LAUNCHER CACHE)
endif()
#pragma once

/**
 * @brief 编译器检测宏 / Compiler detection macros
 *
 * @details 用于检测当前使用的编译器类型 / Used to detect the current compiler
 * type:
 * - MSVC: Microsoft Visual C++ 编译器 / Microsoft Visual C++ Compiler
 * - Clang: LLVM Clang 编译器 / LLVM Clang Compiler
 * - GCC: GNU C++ 编译器 / GNU C++ Compiler
 */
#if defined(_MSC_VER)
#  define CPP_TOOLBOX_COMPILER_MSVC
#elif defined(__clang__)
#  define CPP_TOOLBOX_COMPILER_CLANG
#elif defined(__GNUC__)
#  define CPP_TOOLBOX_COMPILER_GCC
#endif

/**
 * @brief 平台检测宏 / Platform detection macros
 *
 * @details 用于检测当前运行平台 / Used to detect the current platform:
 * - Windows: Windows 操作系统 / Windows Operating System
 * - Linux: Linux 操作系统 / Linux Operating System
 * - macOS: Apple macOS 操作系统 / Apple macOS Operating System
 * - FreeBSD: FreeBSD 操作系统 / FreeBSD Operating System
 * - NetBSD: NetBSD 操作系统 / NetBSD Operating System
 * - Android: Android 操作系统 / Android Operating System
 * - QNX: QNX 实时操作系统 / QNX Real-time Operating System
 */
#if defined(_WIN32) || defined(_WIN64)
#  define CPP_TOOLBOX_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__linux)
#  define CPP_TOOLBOX_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#  define CPP_TOOLBOX_PLATFORM_MACOS
#elif defined(__FreeBSD__) || defined(__FreeBSD)
#  define CPP_TOOLBOX_PLATFORM_FREEBSD
#elif defined(__NetBSD__) || defined(__NetBSD)
#  define CPP_TOOLBOX_PLATFORM_NETBSD
// Android
#elif defined(__ANDROID__) || defined(_ANDROID)
#  define CPP_TOOLBOX_PLATFORM_ANDROID
#elif defined(__QNX__) || defined(_QNX)
#  define CPP_TOOLBOX_PLATFORM_QNX
#endif

/**
 * @brief 架构检测宏 / Architecture detection macros
 *
 * @details 用于检测当前CPU架构 / Used to detect the current CPU architecture:
 * - x86_64: 64位x86架构 / 64-bit x86 architecture
 * - x86: 32位x86架构 / 32-bit x86 architecture
 * - ARM64: 64位ARM架构 / 64-bit ARM architecture
 * - ARM: 32位ARM架构 / 32-bit ARM architecture
 * - RISC-V: RISC-V架构 / RISC-V architecture
 * - LoongArch: 龙芯架构 / LoongArch architecture
 */
#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) \
    || defined(_M_X64)
#  define CPP_TOOLBOX_ARCH_X86_64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#  define CPP_TOOLBOX_ARCH_X86
#elif defined(__aarch64__) || defined(__arm64__) || defined(__arm64) \
    || defined(_M_ARM64)
#  define CPP_TOOLBOX_ARCH_ARM64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM)
#  define CPP_TOOLBOX_ARCH_ARM
#elif defined(__riscv__) || defined(__riscv) || defined(_M_RISCV)
#  define CPP_TOOLBOX_ARCH_RISCV
#elif defined(__loongarch__) || defined(__loongarch) || defined(_M_LOONGARCH)
#  define CPP_TOOLBOX_ARCH_LOONGARCH
#endif

/**
 * @brief 将C++版本转换为字符串 / Convert C++ version to string
 * @param year 版本号中的年份部分 / Year part of version number
 * @param month 版本号中的月份部分 / Month part of version number
 * @return 版本的字符串表示 / String representation of version
 *
 * @code{.cpp}
 * // 将C++17版本号转换为字符串
 * // Convert C++17 version to string
 * const char* version = CPP_VERSION_STR(201, 703);
 * std::cout << version << std::endl; // 输出: "201703L" / Outputs: "201703L"
 * @endcode
 */
#define CPP_VERSION_STR(year, month) #year #month "L"

/**
 * @brief 将C++版本转换为整数 / Convert C++ version to integer
 * @param year 版本号中的年份部分 / Year part of version number
 * @param month 版本号中的月份部分 / Month part of version number
 * @return 版本的整数表示 / Integer representation of version
 *
 * @code{.cpp}
 * // 将C++17版本号转换为整数
 * // Convert C++17 version to integer
 * constexpr auto version = CPP_VERSION_TO_INT(201, 703);
 * static_assert(version == 201703L);
 * @endcode
 */
#define CPP_VERSION_TO_INT(year, month) year##month##L

/**
 * @brief C++标准检测 / C++ standard detection
 */
#if defined(__cplusplus)
#  define CPP_TOOLBOX_CXX_STANDARD __cplusplus
#endif

/**
 * @brief C++版本宏定义 / C++ version macros
 */
#define CPP_11 CPP_VERSION_TO_INT(201, 103)
#define CPP_14 CPP_VERSION_TO_INT(201, 402)
#define CPP_17 CPP_VERSION_TO_INT(201, 703)
#define CPP_20 CPP_VERSION_TO_INT(202, 002)

/**
 * @brief 获取C++版本字符串 / Get C++ version as string
 * @return C++版本的字符串表示 / String representation of C++ version
 *
 * @code{.cpp}
 * const char* version = get_cpp_version();
 * std::cout << "Using " << version << std::endl; // 输出当前C++版本 / Outputs
 * current C++ version
 * @endcode
 */
constexpr const char* get_cpp_version()
{
#if __cplusplus >= CPP_20
  return "C++20";
#elif __cplusplus >= CPP_17
  return "C++17";
#elif __cplusplus >= CPP_14
  return "C++14";
#elif __cplusplus >= CPP_11
  return "C++11";
#else
  return "Pre-C++11";
#endif
}

/**
 * @brief C++17要求的静态断言 / Static assert for C++17 requirement
 * @param Content 断言消息 / Assert message
 *
 * @code{.cpp}
 * CPP_TOOLBOX_CXX_LARGE_THAN_17("This feature requires C++17 or later");
 * @endcode
 */
#define CPP_TOOLBOX_CXX_LARGE_THAN_17(Content) \
  static_assert(__cplusplus >= CPP_17, Content)

/**
 * @brief 获取C++版本字符串 / Get C++ version string
 */
#define CPP_TOOLBOX_CXX_VERSION get_cpp_version()
#pragma once

#include <cstring>  // For memcpy, memset
#include <filesystem>  // For file size

#include <cpp-toolbox/macro.hpp>  // For platform detection CPP_TOOLBOX_PLATFORM_*

// Platform-specific includes for memory mapping / 内存映射的平台特定包含文件
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else  // Assuming POSIX-like systems (Linux, macOS) / 假设为类 POSIX
       // 系统（Linux, macOS）
#  include <fcntl.h>  // For open() / 用于 open() 函数
#  include <sys/mman.h>  // For mmap(), munmap() / 用于 mmap(), munmap() 函数
#  include <sys/stat.h>  // For fstat() / 用于 fstat() 函数
#  include <unistd.h>  // For close() / 用于 close() 函数
#endif

namespace toolbox::file
{

/**
 * @brief RAII wrapper for memory-mapped files. / 内存映射文件的 RAII 封装。
 *
 * This class provides a cross-platform way to map a file into memory for
 * read-only access. It handles the opening, mapping, and closing of the file
 * automatically using RAII.
 *
 * 这个类提供了一种跨平台的方式将文件映射到内存中进行只读访问。
 * 它使用 RAII 自动处理文件的打开、映射和关闭。
 *
 * @code{.cpp}
 * #include <cpp-toolbox/file/memory_mapped_file.hpp>
 * #include <iostream>
 * #include <fstream>
 *
 * int main() {
 *     const std::filesystem::path test_file = "test_mmap.txt";
 *     // Create a dummy file
 *     {
 *         std::ofstream ofs(test_file);
 *         ofs << "Hello, Memory Map!";
 *     }
 *
 *     toolbox::file::memory_mapped_file_t mapped_file;
 *     if (mapped_file.open(test_file)) {
 *         std::cout << "File mapped successfully. Size: " << mapped_file.size()
 * << std::endl;
 *         // Access data (example: print first 5 chars)
 *         const unsigned char* data = mapped_file.data();
 *         if (data && mapped_file.size() >= 5) {
 *              std::cout << "First 5 chars: ";
 *              for(size_t i = 0; i < 5; ++i) {
 *                  std::cout << data[i];
 *              }
 *              std::cout << std::endl;
 *         }
 *         // File is automatically unmapped and closed when mapped_file goes
 * out of scope } else { std::cerr << "Failed to map file: " << test_file <<
 * std::endl;
 *     }
 *
 *     std::filesystem::remove(test_file); // Clean up
 *     return 0;
 * }
 * @endcode
 */
class CPP_TOOLBOX_EXPORT memory_mapped_file_t
{
  /**
   * @brief Pointer to the mapped memory region. / 指向映射内存区域的指针。
   */
  void* m_mapped_ptr = nullptr;

  /**
   * @brief Size of the mapped file in bytes. / 映射文件的字节大小。
   */
  size_t m_mapped_size = 0;

#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
  /**
   * @brief Windows file handle. / Windows 文件句柄。
   */
  HANDLE m_file_handle = INVALID_HANDLE_VALUE;

  /**
   * @brief Windows file mapping handle. / Windows 文件映射句柄。
   */
  HANDLE m_mapping_handle = NULL;
#else
  /**
   * @brief POSIX file descriptor. / POSIX 文件描述符。
   */
  int m_fd = -1;
#endif

public:
  /**
   * @brief Default constructor. / 默认构造函数。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mmap_file;
   * // Now you can call open() to map a file
   * @endcode
   */
  memory_mapped_file_t() = default;

  /**
   * @brief Destructor. Automatically closes the mapped file. /
   * 析构函数。自动关闭映射的文件。
   *
   * Calls close() to ensure all resources are properly released. / 调用 close()
   * 确保所有资源被正确释放。
   */
  ~memory_mapped_file_t();

  // Disable copy / 禁用拷贝
  /**
   * @brief Copy constructor (deleted). / 拷贝构造函数（已删除）。
   */
  memory_mapped_file_t(const memory_mapped_file_t&) = delete;

  /**
   * @brief Copy assignment operator (deleted). / 拷贝赋值运算符（已删除）。
   */
  memory_mapped_file_t& operator=(const memory_mapped_file_t&) = delete;

  // Disable move constructor and assignment / 禁用移动构造和赋值
  /**
   * @brief Move constructor (deleted). / 移动构造函数（已删除）。
   */
  memory_mapped_file_t(memory_mapped_file_t&&) = delete;

  /**
   * @brief Move assignment operator (deleted). / 移动赋值运算符（已删除）。
   */
  memory_mapped_file_t& operator=(memory_mapped_file_t&&) = delete;

  /**
   * @brief Opens and memory-maps the specified file. /
   * 打开并内存映射指定的文件。
   *
   * If a file is already open, it will be closed first. /
   * 如果已有文件打开，会先关闭。
   *
   * @param path The path to the file to map. / 要映射的文件的路径。
   * @return True if the file was successfully opened and mapped, false
   * otherwise. / 如果文件成功打开并映射，返回 true，否则返回 false。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mf;
   * if (mf.open("my_data.bin")) {
   *     // Use mf.data() and mf.size()
   *     const unsigned char* buffer = mf.data();
   *     size_t file_size = mf.size();
   *
   *     // Process the data
   *     for (size_t i = 0; i < file_size; ++i) {
   *         // Do something with buffer[i]
   *     }
   * } else {
   *     // Handle error
   *     std::cerr << "Failed to open file" << std::endl;
   * }
   * @endcode
   */
  bool open(const std::filesystem::path& path);

  /**
   * @brief Closes the memory-mapped file and releases resources. /
   * 关闭内存映射文件并释放资源。
   *
   * This is called automatically by the destructor, but can be called manually.
   * / 这个函数由析构函数自动调用，但也可以手动调用。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mf;
   * if (mf.open("data.bin")) {
   *     // Process the file
   *     // ...
   *
   *     // Explicitly close when done
   *     mf.close();
   *
   *     // Can now open another file
   *     mf.open("another_file.bin");
   * }
   * @endcode
   */
  void close();

  /**
   * @brief Gets a pointer to the mapped memory region. /
   * 获取指向映射内存区域的指针。
   *
   * @return A const pointer to the start of the mapped data, or nullptr if not
   * open. / 指向映射数据起始位置的 const 指针，如果未打开则为 nullptr。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mf;
   * if (mf.open("image.dat") && mf.size() >= 1024) {
   *     const unsigned char* img_data = mf.data();
   *     // Now you can read the image data without loading it all into memory
   *     unsigned int header_value = img_data[0] | (img_data[1] << 8) |
   * (img_data[2] << 16) | (img_data[3] << 24);
   * }
   * @endcode
   */
  [[nodiscard]] const unsigned char* data() const
  {
    return static_cast<const unsigned char*>(m_mapped_ptr);
  }

  /**
   * @brief Gets the size of the mapped file. / 获取映射文件的大小。
   *
   * @return The size of the mapped region in bytes, or 0 if not open. /
   * 映射区域的大小（字节），如果未打开则为 0。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mf;
   * if (mf.open("data.bin")) {
   *     std::cout << "File size: " << mf.size() << " bytes" << std::endl;
   *
   *     // Check if file is large enough for our needs
   *     if (mf.size() < required_size) {
   *         std::cerr << "File is too small" << std::endl;
   *     }
   * }
   * @endcode
   */
  [[nodiscard]] size_t size() const { return m_mapped_size; }

  /**
   * @brief Checks if a file is currently mapped. / 检查文件当前是否已映射。
   *
   * @return True if a file is mapped, false otherwise. / 如果文件已映射，返回
   * true，否则返回 false。
   *
   * @code{.cpp}
   * toolbox::file::memory_mapped_file_t mf;
   * // Check before using
   * if (!mf.is_open()) {
   *     if (!mf.open("config.dat")) {
   *         std::cerr << "Failed to open config file" << std::endl;
   *         return -1;
   *     }
   * }
   *
   * // Now safe to use mf.data() and mf.size()
   * @endcode
   */
  [[nodiscard]] bool is_open() const { return m_mapped_ptr != nullptr; }
};

}  // namespace toolbox::file

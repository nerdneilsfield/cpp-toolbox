#pragma once
#include <filesystem>
#include <functional>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/macro.hpp"

namespace toolbox::file
{

using file_time_type = std::filesystem::file_time_type;
using file_type = std::filesystem::file_type;

/**
 * @brief 将字符串转换为文件系统路径 / Convert a string to a filesystem path
 * @param str 要转换为路径的字符串 / The string to convert to a path
 * @return std::filesystem::path 转换后的路径对象 / The converted path object
 *
 * @code{.cpp}
 * // 基本用法 / Basic usage
 * auto path = string_to_path("/usr/local/bin");
 *
 * // Windows路径转换 / Windows path conversion
 * auto win_path = string_to_path("C:\\Program Files\\MyApp");
 *
 * // 相对路径转换 / Relative path conversion
 * auto rel_path = string_to_path("../data/config.json");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto string_to_path(const std::string& str)
    -> std::filesystem::path;

/**
 * @brief 将文件系统路径转换为字符串 / Convert a filesystem path to a string
 * @param path 要转换为字符串的路径 / The path to convert to a string
 * @return std::string 路径的字符串表示 / The string representation of the path
 *
 * @code{.cpp}
 * // 基本用法 / Basic usage
 * std::filesystem::path p = "/usr/local/bin";
 * auto str = path_to_string(p);  // 返回 "/usr/local/bin" / Returns
 * "/usr/local/bin"
 *
 * // Windows路径转换 / Windows path conversion
 * std::filesystem::path win_p = "C:\\Program Files\\MyApp";
 * auto win_str = path_to_string(win_p);  // 返回 "C:\\Program Files\\MyApp" /
 * Returns "C:\\Program Files\\MyApp"
 *
 * // 相对路径转换 / Relative path conversion
 * std::filesystem::path rel_p = "../data/config.json";
 * auto rel_str = path_to_string(rel_p);  // 返回 "../data/config.json" /
 * Returns "../data/config.json"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto path_to_string(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief 获取文件的字节大小 / Get the size of a file in bytes
 * @param path 文件的路径 / The path to the file
 * @return std::size_t 文件的字节大小 / The size of the file in bytes
 * @throws std::filesystem::filesystem_error 如果文件不存在或无法访问 / if the
 * file doesn't exist or can't be accessed
 *
 * @code{.cpp}
 * // 获取现有文件的大小 / Get size of existing file
 * try {
 *     auto size = get_file_size("/path/to/file.txt");
 *     std::cout << "文件大小 / File size: " << size << " bytes\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误 / Error: " << e.what() << '\n';
 * }
 *
 * // 获取大文件的大小 / Get size of a large file
 * try {
 *     auto large_size = get_file_size("/path/to/large.iso");
 *     std::cout << "大文件大小 / Large file size: " << large_size << "
 * bytes\n"; } catch (const std::filesystem::filesystem_error& e) { std::cerr <<
 * "错误 / Error: " << e.what() << '\n';
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_file_size(const std::filesystem::path& path)
    -> std::size_t;

/**
 * @brief 获取文件或目录的绝对路径 / Get the absolute path of a file or
 * directory
 * @param path 要转换为绝对路径的路径 / The path to convert to absolute
 * @return std::filesystem::path 绝对路径 / The absolute path
 *
 * @code{.cpp}
 * // 获取相对路径的绝对路径 / Get absolute path of relative path
 * auto abs_path = get_absolute_path("relative/path");
 *
 * // 获取当前目录的绝对路径 / Get absolute path of current directory
 * auto current_dir = get_absolute_path(".");
 *
 * // 获取上级目录文件的绝对路径 / Get absolute path of file in parent directory
 * auto parent_file = get_absolute_path("../config.json");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_absolute_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief 获取两个路径之间的相对路径 / Get the relative path between two paths
 * @param path 目标路径 / The target path
 * @param base 计算相对路径的基准路径 / The base path to compute relative path
 * from
 * @return std::filesystem::path 从基准路径到目标路径的相对路径 / The relative
 * path from base to target
 *
 * @code{.cpp}
 * // 获取两个目录之间的相对路径 / Get relative path between two directories
 * auto rel_path = get_relative_path("/usr/local/bin", "/usr");
 * // rel_path 将是 "local/bin" / rel_path will be "local/bin"
 *
 * // 获取文件之间的相对路径 / Get relative path between files
 * auto file_rel = get_relative_path("/a/b/c.txt", "/a/d/e.txt");
 * // file_rel 将是 "../b/c.txt" / file_rel will be "../b/c.txt"
 *
 * // 获取跨驱动器的相对路径 / Get relative path across drives
 * auto cross_drive = get_relative_path("D:/data", "C:/Program Files");
 * // 返回完整路径 / Returns full path
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_relative_path(const std::filesystem::path& path,
                                          const std::filesystem::path& base)
    -> std::filesystem::path;

/**
 * @brief 获取路径的父目录 / Get the parent directory of a path
 * @param path 要获取父目录的路径 / The path to get the parent of
 * @return std::filesystem::path 父目录路径 / The parent directory path
 *
 * @code{.cpp}
 * // 获取文件的父目录 / Get parent directory of file
 * auto parent = get_parent_path("/usr/local/bin/gcc");
 * // parent 将是 "/usr/local/bin" / parent will be "/usr/local/bin"
 *
 * // 获取目录的父目录 / Get parent directory of directory
 * auto parent_dir = get_parent_path("/usr/local/bin/");
 * // parent_dir 将是 "/usr/local" / parent_dir will be "/usr/local"
 *
 * // 获取根目录的父目录 / Get parent of root directory
 * auto root_parent = get_parent_path("/");
 * // root_parent 将是 "/" / root_parent will be "/"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_parent_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief 获取路径的文件名部分 / Get the file name component of a path
 * @param path 要提取文件名的路径 / The path to extract the file name from
 * @return std::string 文件名部分 / The file name component
 *
 * @code{.cpp}
 * // 从完整路径获取文件名 / Get file name from full path
 * auto name = get_file_name("/usr/local/bin/gcc");
 * // name 将是 "gcc" / name will be "gcc"
 *
 * // 从相对路径获取文件名 / Get file name from relative path
 * auto rel_name = get_file_name("docs/readme.txt");
 * // rel_name 将是 "readme.txt" / rel_name will be "readme.txt"
 *
 * // 获取带多个扩展名的文件名 / Get file name with multiple extensions
 * auto multi_ext = get_file_name("archive.tar.gz");
 * // multi_ext 将是 "archive.tar.gz" / multi_ext will be "archive.tar.gz"
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_file_name(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief 获取路径的文件扩展名 / Get the file extension of a path
 * @param path 要提取扩展名的路径 / The path to extract the extension from
 * @return std::string 包含点号的文件扩展名 / The file extension including the
 * dot
 *
 * @code{.cpp}
 * // 从文件路径获取扩展名 / Get extension from file path
 * auto ext = get_file_extension("/path/to/document.pdf");
 * // ext 将是 ".pdf" / ext will be ".pdf"
 *
 * // 从文件名获取扩展名 / Get extension from file name
 * auto ext2 = get_file_extension("archive.tar.gz");
 * // ext2 将是 ".gz" / ext2 will be ".gz"
 *
 * // 获取无扩展名文件的扩展名 / Get extension of file without extension
 * auto no_ext = get_file_extension("README");
 * // no_ext 将是空字符串 / no_ext will be empty string
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_file_extension(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief 复制文件 / Copy a file
 * @param src 源文件 / The source file
 * @param dst 目标文件 / The destination file
 * @return 如果文件复制成功则返回true / True if the file was copied successfully
 *
 * @code{.cpp}
 * // 复制单个文件 / Copy a single file
 * if (copy_file("/path/to/source.txt", "/path/to/dest.txt")) {
 *     std::cout << "文件复制成功 / File copied successfully\n";
 * }
 *
 * // 复制并覆盖现有文件 / Copy and overwrite existing file
 * bool success = copy_file("data.bak", "data.txt");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto copy_file(const std::filesystem::path& src,
                                  const std::filesystem::path& dst) -> bool;

/**
 * @brief 移动文件 / Move a file
 * @param src 源文件 / The source file
 * @param dst 目标文件 / The destination file
 * @return 如果文件移动成功则返回true / True if the file was moved successfully
 *
 * @code{.cpp}
 * // 移动文件到新位置 / Move file to new location
 * if (move_file("/tmp/temp.txt", "/home/user/final.txt")) {
 *     std::cout << "文件移动成功 / File moved successfully\n";
 * }
 *
 * // 重命名文件 / Rename file
 * bool renamed = move_file("old_name.txt", "new_name.txt");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto move_file(const std::filesystem::path& src,
                                  const std::filesystem::path& dst) -> bool;

/**
 * @brief 删除指定路径的文件 / Delete a file at the specified path
 * @param path 要删除的文件的文件系统路径 / The filesystem path to the file to
 * delete
 * @return 如果文件删除成功返回true，否则返回false / True if the file was
 * successfully deleted, false otherwise
 *
 * @code{.cpp}
 * // 删除现有文件 / Delete an existing file
 * if (delete_file("/tmp/test.txt")) {
 *     std::cout << "文件删除成功 / File deleted successfully\n";
 * } else {
 *     std::cerr << "删除文件失败 / Failed to delete file\n";
 * }
 *
 * // 尝试删除不存在的文件 / Attempt to delete non-existent file
 * bool result = delete_file("/path/to/nonexistent.file");
 * // result 将是 false / result will be false
 *
 * // 删除临时文件 / Delete temporary file
 * auto temp_file = "/tmp/temp_" + std::to_string(time(nullptr)) + ".tmp";
 * bool deleted = delete_file(temp_file);
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto delete_file(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查指定路径是否存在文件 / Check if a file exists at the specified
 * path
 * @param path 要检查的文件系统路径 / The filesystem path to check
 * @return 如果文件存在返回true，否则返回false / True if the file exists, false
 * otherwise
 *
 * @code{.cpp}
 * // 检查文件是否存在 / Check if file exists
 * if (file_exists("/etc/passwd")) {
 *     std::cout << "文件存在 / File exists\n";
 * } else {
 *     std::cout << "文件不存在 / File does not exist\n";
 * }
 *
 * // 检查不存在的文件 / Check non-existent file
 * bool exists = file_exists("/tmp/nonexistent.file");
 * // exists 将是 false / exists will be false
 *
 * // 在处理文件前检查其存在性 / Check existence before processing
 * if (file_exists("data.txt")) {
 *     // 处理文件 / Process file
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto file_exists(const std::filesystem::path& path) -> bool;

/**
 * @brief 在指定路径创建目录 / Create a directory at the specified path
 * @param path 应创建目录的文件系统路径 / The filesystem path where the
 * directory should be created
 * @return 如果目录创建成功返回true，否则返回false / True if the directory was
 * created successfully, false otherwise
 *
 * @code{.cpp}
 * // 创建新目录 / Create a new directory
 * if (create_directory("/tmp/new_dir")) {
 *     std::cout << "目录创建成功 / Directory created\n";
 * } else {
 *     std::cerr << "创建目录失败 / Failed to create directory\n";
 * }
 *
 * // 创建嵌套目录 / Create nested directories
 * create_directory("/tmp/a/b/c");  // 创建路径中的所有目录 / Creates all
 * directories in path
 *
 * // 创建带时间戳的目录 / Create directory with timestamp
 * auto timestamp_dir = "/tmp/backup_" + std::to_string(time(nullptr));
 * bool created = create_directory(timestamp_dir);
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto create_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 检查指定路径是否存在目录 / Check if a directory exists at the
 * specified path
 * @param path 要检查的文件系统路径 / The filesystem path to check
 * @return 如果目录存在返回true，否则返回false / True if the directory exists,
 * false otherwise
 *
 * @code{.cpp}
 * // 检查目录是否存在 / Check if directory exists
 * if (directory_exists("/usr/local/bin")) {
 *     std::cout << "目录存在 / Directory exists\n";
 * } else {
 *     std::cout << "目录不存在 / Directory does not exist\n";
 * }
 *
 * // 检查不存在的目录 / Check non-existent directory
 * bool exists = directory_exists("/tmp/nonexistent_dir");
 * // exists 将是 false / exists will be false
 *
 * // 在创建目录前检查 / Check before creating directory
 * if (!directory_exists("/tmp/data")) {
 *     create_directory("/tmp/data");
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto directory_exists(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 删除指定路径的目录 / Delete a directory at the specified path
 * @param path 要删除的目录的文件系统路径 / The filesystem path of the directory
 * to delete
 * @return 如果目录删除成功返回true，否则返回false / True if the directory was
 * deleted successfully, false otherwise
 *
 * @code{.cpp}
 * // 删除空目录 / Delete an empty directory
 * if (delete_directory("/tmp/empty_dir")) {
 *     std::cout << "目录删除成功 / Directory deleted\n";
 * } else {
 *     std::cerr << "删除目录失败 / Failed to delete directory\n";
 * }
 *
 * // 删除非空目录 / Delete non-empty directory
 * delete_directory("/tmp/non_empty_dir");  // 除非递归否则将失败 / Will fail
 * unless recursive
 *
 * // 删除临时目录 / Delete temporary directory
 * auto temp_dir = "/tmp/temp_" + std::to_string(time(nullptr));
 * bool deleted = delete_directory(temp_dir);
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto delete_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 获取当前工作目录 / Get the current working directory
 * @return 当前工作目录的文件系统路径 / The current working directory as a
 * filesystem path
 *
 * @code{.cpp}
 * // 打印当前目录 / Print current directory
 * auto current_dir = get_current_directory();
 * std::cout << "当前目录 / Current directory: " << current_dir << "\n";
 *
 * // 更改并获取当前目录 / Change and get current directory
 * std::filesystem::current_path("/tmp");
 * auto dir = get_current_directory();  // 返回 "/tmp" / Returns "/tmp"
 *
 * // 保存并恢复工作目录 / Save and restore working directory
 * auto original_dir = get_current_directory();
 * // 执行一些操作 / Do some operations
 * std::filesystem::current_path(original_dir);
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_current_directory() -> std::filesystem::path;

/**
 * @brief 获取指定路径的文件系统对象类型 / Get the type of the filesystem object
 * at the specified path
 * @param path 要检查的文件系统路径 / The filesystem path to check
 * @return 文件系统对象的类型（文件、目录等） / The type of the filesystem
 * object (file, directory, etc.)
 *
 * @code{.cpp}
 * // 检查文件类型 / Check type of a file
 * auto type = path_type("/etc/passwd");
 * if (type == std::filesystem::file_type::regular) {
 *     std::cout << "常规文件 / Regular file\n";
 * }
 *
 * // 检查目录类型 / Check type of a directory
 * auto dir_type = path_type("/usr/local/bin");
 * if (dir_type == std::filesystem::file_type::directory) {
 *     std::cout << "目录 / Directory\n";
 * }
 *
 * // 检查符号链接 / Check symbolic link
 * auto link_type = path_type("/usr/bin/python");
 * if (link_type == std::filesystem::file_type::symlink) {
 *     std::cout << "符号链接 / Symbolic link\n";
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto path_type(const std::filesystem::path& path)
    -> std::filesystem::file_type;

/**
 * @brief 检查路径是否为常规文件 / Check if a path is a regular file
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是常规文件返回true，否则返回false / True if the path is a
 * regular file, false otherwise
 *
 * @code{.cpp}
 * // 检查文件是否存在且为常规文件 / Check if a file exists and is regular
 * if (is_regular_file("/path/to/file.txt")) {
 *     std::cout << "是常规文件 / Regular file exists\n";
 * } else {
 *     std::cerr << "不是常规文件 / Not a regular file\n";
 * }
 *
 * // 在读取文件前检查 / Check before reading file
 * if (is_regular_file("config.json")) {
 *     // 读取文件 / Read file
 * }
 *
 * // 区分文件和符号链接 / Distinguish between file and symlink
 * bool is_file = is_regular_file("/usr/bin/python");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_regular_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 检查路径是否为目录 / Check if a path is a directory
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是目录返回true，否则返回false / True if the path is a
 * directory, false otherwise
 *
 * @code{.cpp}
 * // 检查目录是否存在 / Check if a directory exists
 * if (is_directory("/path/to/dir")) {
 *     std::cout << "是目录 / Directory exists\n";
 * } else {
 *     std::cerr << "不是目录 / Not a directory\n";
 * }
 *
 * // 在遍历目录前检查 / Check before traversing directory
 * if (is_directory("/usr/local")) {
 *     // 遍历目录 / Traverse directory
 * }
 *
 * // 检查临时目录 / Check temporary directory
 * bool is_dir = is_directory("/tmp");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_directory(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查路径是否为符号链接 / Check if a path is a symbolic link
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是符号链接返回true，否则返回false / True if the path is a
 * symbolic link, false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为符号链接 / Check if a path is a symlink
 * if (is_symlink("/path/to/symlink")) {
 *     std::cout << "是符号链接 / Symbolic link detected\n";
 * } else {
 *     std::cerr << "不是符号链接 / Not a symbolic link\n";
 * }
 *
 * // 检查Python可执行文件 / Check Python executable
 * bool is_link = is_symlink("/usr/bin/python");
 *
 * // 在跟随链接前检查 / Check before following link
 * if (is_symlink("link.txt")) {
 *     // 处理符号链接 / Handle symbolic link
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_symlink(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查路径是否为FIFO(命名管道) / Check if a path is a FIFO (named pipe)
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是FIFO返回true，否则返回false / True if the path is a FIFO,
 * false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为FIFO / Check if a path is a FIFO
 * if (is_fifo("/path/to/fifo")) {
 *     std::cout << "检测到FIFO / FIFO detected\n";
 * } else {
 *     std::cerr << "不是FIFO / Not a FIFO\n";
 * }
 *
 * // 在打开FIFO前检查 / Check before opening FIFO
 * if (is_fifo("/tmp/my_pipe")) {
 *     // 处理FIFO / Handle FIFO
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_fifo(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查路径是否为套接字 / Check if a path is a socket
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是套接字返回true，否则返回false / True if the path is a
 * socket, false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为套接字 / Check if a path is a socket
 * if (is_socket("/path/to/socket")) {
 *     std::cout << "检测到套接字 / Socket detected\n";
 * } else {
 *     std::cerr << "不是套接字 / Not a socket\n";
 * }
 *
 * // 检查Unix域套接字 / Check Unix domain socket
 * bool is_sock = is_socket("/tmp/my.sock");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_socket(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查路径是否为块设备 / Check if a path is a block device
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是块设备返回true，否则返回false / True if the path is a block
 * device, false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为块设备 / Check if a path is a block device
 * if (is_block_file("/dev/sda1")) {
 *     std::cout << "检测到块设备 / Block device detected\n";
 * } else {
 *     std::cerr << "不是块设备 / Not a block device\n";
 * }
 *
 * // 检查磁盘分区 / Check disk partition
 * bool is_disk = is_block_file("/dev/nvme0n1p1");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_block_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 检查路径是否为字符设备 / Check if a path is a character device
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是字符设备返回true，否则返回false / True if the path is a
 * character device, false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为字符设备 / Check if a path is a character device
 * if (is_character_file("/dev/tty")) {
 *     std::cout << "检测到字符设备 / Character device detected\n";
 * } else {
 *     std::cerr << "不是字符设备 / Not a character device\n";
 * }
 *
 * // 检查串口设备 / Check serial port device
 * bool is_serial = is_character_file("/dev/ttyUSB0");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_character_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief 检查路径是否为未知类型 / Check if a path is of an unknown type
 * @param path 要检查的路径 / The path to check
 * @return 如果路径是未知类型返回true，否则返回false / True if the path is of an
 * unknown type, false otherwise
 *
 * @code{.cpp}
 * // 检查路径是否为未知类型 / Check if a path is of unknown type
 * if (is_other("/path/to/unknown")) {
 *     std::cout << "检测到未知文件类型 / Unknown file type detected\n";
 * } else {
 *     std::cerr << "文件类型已知 / File type is known\n";
 * }
 *
 * // 在处理特殊文件前检查 / Check before handling special files
 * if (!is_other("/dev/custom_device")) {
 *     // 处理已知类型文件 / Handle known file type
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_other(const std::filesystem::path& path) -> bool;

/**
 * @brief 检查路径是否为空 / Check if a path is empty
 * @param path 要检查的路径 / The path to check
 * @return 如果路径为空返回true，否则返回false / True if the path is empty,
 * false otherwise
 *
 * @code{.cpp}
 * // 检查目录是否为空 / Check if a directory is empty
 * if (is_empty("/path/to/dir")) {
 *     std::cout << "目录为空 / Directory is empty\n";
 * } else {
 *     std::cerr << "目录不为空 / Directory is not empty\n";
 * }
 *
 * // 检查文件是否为空 / Check if a file is empty
 * bool is_empty_file = is_empty("data.txt");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto is_empty(const std::filesystem::path& path) -> bool;

/**
 * @brief 获取文件的创建时间 / Get the creation time of a file
 * @param path 要检查的路径 / The path to check
 * @return 文件的创建时间 / The creation time of the file
 * @throws std::filesystem::filesystem_error 如果文件不存在或无法访问 / if the
 * file doesn't exist or can't be accessed
 *
 * @code{.cpp}
 * // 获取文件的创建时间 / Get creation time of a file
 * try {
 *     auto creation_time = get_creation_time("/path/to/file.txt");
 *     std::cout << "文件创建于: / File created at: " << creation_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 比较两个文件的创建时间 / Compare creation times of two files
 * auto time1 = get_creation_time("file1.txt");
 * auto time2 = get_creation_time("file2.txt");
 * if (time1 < time2) {
 *     std::cout << "file1.txt创建较早 / file1.txt is older\n";
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_creation_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief 获取文件的最后访问时间 / Get the last access time of a file
 * @param path 要检查的路径 / The path to check
 * @return 文件的最后访问时间 / The last access time as file_time_type
 * @throws std::filesystem::filesystem_error 如果文件不存在或无法访问 / if the
 * file doesn't exist or can't be accessed
 *
 * @code{.cpp}
 * // 获取文件的最后访问时间 / Get last access time of a file
 * try {
 *     auto access_time = get_last_access_time("/path/to/file.txt");
 *     std::cout << "最后访问时间: / Last accessed: " << access_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 检查文件是否最近被访问 / Check if file was recently accessed
 * auto now = std::filesystem::file_time_type::clock::now();
 * auto last_access = get_last_access_time("data.log");
 * if ((now - last_access).count() > 86400) {
 *     std::cout << "文件超过24小时未访问 / File not accessed in 24 hours\n";
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_last_access_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief 获取文件的最后修改时间 / Get the last write time of a file
 * @param path 要检查的路径 / The path to check
 * @return 文件的最后修改时间 / The last write time as file_time_type
 * @throws std::filesystem::filesystem_error 如果文件不存在或无法访问 / if the
 * file doesn't exist or can't be accessed
 *
 * @code{.cpp}
 * // 获取文件的最后修改时间 / Get last modification time of a file
 * try {
 *     auto write_time = get_last_write_time("/path/to/file.txt");
 *     std::cout << "最后修改时间: / Last modified: " << write_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 检查文件是否需要更新 / Check if file needs update
 * auto cache_time = get_last_write_time("cache.dat");
 * auto source_time = get_last_write_time("source.dat");
 * if (cache_time < source_time) {
 *     std::cout << "缓存需要更新 / Cache needs update\n";
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_last_write_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief 获取符号链接的目标路径 / Get the target of a symbolic link
 * @param path 符号链接的路径 / The path to the symbolic link
 * @return 目标路径 / The target path as std::filesystem::path
 * @throws std::filesystem::filesystem_error 如果路径不是符号链接 / if the path
 * is not a symbolic link
 *
 * @code{.cpp}
 * // 获取符号链接的目标 / Get symlink target
 * try {
 *     auto target = get_symlink_target("/path/to/symlink");
 *     std::cout << "符号链接指向: / Symlink points to: " << target << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 检查符号链接是否指向有效文件 / Check if symlink points to valid file
 * auto target = get_symlink_target("link.txt");
 * if (file_exists(target)) {
 *     std::cout << "链接目标存在 / Link target exists\n";
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_symlink_target(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief 遍历目录 / Traverse a directory
 * @param path 要遍历的路径 / The path to traverse
 * @return 目录中的路径向量 / A vector of paths in the directory
 * @throws std::filesystem::filesystem_error 如果路径不是目录 / if the path is
 * not a directory
 *
 * @code{.cpp}
 * // 列出目录中的文件 / List files in a directory
 * try {
 *     auto files = traverse_directory("/path/to/dir");
 *     for (const auto& file : files) {
 *         std::cout << "找到文件: / Found file: " << file << "\n";
 *     }
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 统计目录中的文件数量 / Count files in directory
 * auto files = traverse_directory("/usr/local/bin");
 * std::cout << "文件总数: / Total files: " << files.size() << "\n";
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto traverse_directory(const std::filesystem::path& path)
    -> std::vector<std::filesystem::path>;

/**
 * @brief 递归遍历目录 / Recursively traverse a directory
 * @param path 要遍历的路径 / The path to traverse
 * @return 目录及其子目录中的路径向量 / A vector of paths in the directory and
 * its subdirectories
 * @throws std::filesystem::filesystem_error 如果路径不是目录 / if the path is
 * not a directory
 *
 * @code{.cpp}
 * // 递归列出目录中的所有文件 / Recursively list files in a directory
 * try {
 *     auto all_files = recursive_traverse_directory("/path/to/dir");
 *     for (const auto& file : all_files) {
 *         std::cout << "找到文件: / Found file: " << file << "\n";
 *     }
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 查找特定扩展名的文件 / Find files with specific extension
 * auto files = recursive_traverse_directory("/home/user");
 * for (const auto& file : files) {
 *     if (file.extension() == ".txt") {
 *         std::cout << "找到文本文件: / Found text file: " << file << "\n";
 *     }
 * }
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory(
    const std::filesystem::path& path) -> std::vector<std::filesystem::path>;

/**
 * @brief 遍历目录并为每个文件调用回调函数 / Traverse a directory and call a
 * callback for each file
 * @param path 要遍历的路径 / The path to traverse
 * @param callback 为每个文件调用的回调函数 / The callback to call for each file
 * @throws std::filesystem::filesystem_error 如果路径不是目录 / if the path is
 * not a directory
 *
 * @code{.cpp}
 * // 处理目录中的每个文件 / Process each file in a directory
 * try {
 *     traverse_directory_files("/path/to/dir", [](const auto& file) {
 *         std::cout << "处理文件: / Processing: " << file << "\n";
 *     });
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 统计文件大小 / Count file sizes
 * std::size_t total_size = 0;
 * traverse_directory_files("/data", [&total_size](const auto& file) {
 *     total_size += std::filesystem::file_size(file);
 * });
 * std::cout << "总大小: / Total size: " << total_size << " bytes\n";
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief 递归遍历目录并为每个文件调用回调函数 / Recursively traverse a
 * directory and call a callback for each file
 * @param path 要遍历的路径 / The path to traverse
 * @param callback 为每个文件调用的回调函数 / The callback to call for each file
 * @throws std::filesystem::filesystem_error 如果路径不是目录 / if the path is
 * not a directory
 *
 * @code{.cpp}
 * // 递归处理每个文件 / Process each file recursively
 * try {
 *     recursive_traverse_directory_files("/path/to/dir", [](const auto& file) {
 *         std::cout << "处理文件: / Processing: " << file << "\n";
 *     });
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 查找大文件 / Find large files
 * recursive_traverse_directory_files("/home", [](const auto& file) {
 *     if (std::filesystem::file_size(file) > 1024*1024) {
 *         std::cout << "大文件: / Large file: " << file << "\n";
 *     }
 * });
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief 获取当前工作目录 / Get the current working directory
 * @return 当前工作目录路径 / The current working directory as
 * std::filesystem::path
 * @throws std::filesystem::filesystem_error 如果无法确定当前目录 / if the
 * current directory cannot be determined
 *
 * @code{.cpp}
 * // 打印当前工作目录 / Print current working directory
 * try {
 *     auto cwd = get_current_working_directory();
 *     std::cout << "当前目录: / Current directory: " << cwd << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "错误: / Error: " << e.what() << '\n';
 * }
 *
 * // 保存并恢复工作目录 / Save and restore working directory
 * auto original_path = get_current_working_directory();
 * // 更改目录 / Change directory
 * std::filesystem::current_path("/tmp");
 * // 恢复原始目录 / Restore original directory
 * std::filesystem::current_path(original_path);
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_current_working_directory()
    -> std::filesystem::path;

}  // namespace toolbox::file

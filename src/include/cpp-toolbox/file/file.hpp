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
 * @brief Convert a string to a filesystem path
 * @param str The string to convert to a path
 * @return std::filesystem::path The converted path object
 *
 * @example
 * // Basic usage
 * auto path = string_to_path("/usr/local/bin");
 *
 * @example
 * // Windows path conversion
 * auto win_path = string_to_path("C:\\Program Files\\MyApp");
 */
CPP_TOOLBOX_EXPORT auto string_to_path(const std::string& str)
    -> std::filesystem::path;

/**
 * @brief Convert a filesystem path to a string
 * @param path The path to convert to a string
 * @return std::string The string representation of the path
 *
 * @example
 * // Basic usage
 * std::filesystem::path p = "/usr/local/bin";
 * auto str = path_to_string(p);  // Returns "/usr/local/bin"
 *
 * @example
 * // Windows path conversion
 * std::filesystem::path win_p = "C:\\Program Files\\MyApp";
 * auto win_str = path_to_string(win_p);  // Returns "C:\\Program Files\\MyApp"
 */
CPP_TOOLBOX_EXPORT auto path_to_string(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief Get the size of a file in bytes
 * @param path The path to the file
 * @return std::size_t The size of the file in bytes
 * @throws std::filesystem::filesystem_error if the file doesn't exist or can't
 * be accessed
 *
 * @example
 * // Get size of existing file
 * try {
 *     auto size = get_file_size("/path/to/file.txt");
 *     std::cout << "File size: " << size << " bytes\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_file_size(const std::filesystem::path& path)
    -> std::size_t;

/**
 * @brief Get the absolute path of a file or directory
 * @param path The path to convert to absolute
 * @return std::filesystem::path The absolute path
 *
 * @example
 * // Get absolute path of relative path
 * auto abs_path = get_absolute_path("relative/path");
 *
 * @example
 * // Get absolute path of current directory
 * auto current_dir = get_absolute_path(".");
 */
CPP_TOOLBOX_EXPORT auto get_absolute_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Get the relative path between two paths
 * @param path The target path
 * @param base The base path to compute relative path from
 * @return std::filesystem::path The relative path from base to target
 *
 * @example
 * // Get relative path between two directories
 * auto rel_path = get_relative_path("/usr/local/bin", "/usr");
 * // rel_path will be "local/bin"
 *
 * @example
 * // Get relative path between files
 * auto file_rel = get_relative_path("/a/b/c.txt", "/a/d/e.txt");
 * // file_rel will be "../b/c.txt"
 */
CPP_TOOLBOX_EXPORT auto get_relative_path(const std::filesystem::path& path,
                                          const std::filesystem::path& base)
    -> std::filesystem::path;

/**
 * @brief Get the parent directory of a path
 * @param path The path to get the parent of
 * @return std::filesystem::path The parent directory path
 *
 * @example
 * // Get parent directory of file
 * auto parent = get_parent_path("/usr/local/bin/gcc");
 * // parent will be "/usr/local/bin"
 *
 * @example
 * // Get parent directory of directory
 * auto parent_dir = get_parent_path("/usr/local/bin/");
 * // parent_dir will be "/usr/local"
 */
CPP_TOOLBOX_EXPORT auto get_parent_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Get the file name component of a path
 * @param path The path to extract the file name from
 * @return std::string The file name component
 *
 * @example
 * // Get file name from full path
 * auto name = get_file_name("/usr/local/bin/gcc");
 * // name will be "gcc"
 *
 * @example
 * // Get file name from relative path
 * auto rel_name = get_file_name("docs/readme.txt");
 * // rel_name will be "readme.txt"
 */
CPP_TOOLBOX_EXPORT auto get_file_name(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief Get the file extension of a path
 * @param path The path to extract the extension from
 * @return std::string The file extension including the dot
 *
 * @example
 * // Get extension from file path
 * auto ext = get_file_extension("/path/to/document.pdf");
 * // ext will be ".pdf"
 *
 * @example
 * // Get extension from file name
 * auto ext2 = get_file_extension("archive.tar.gz");
 * // ext2 will be ".gz"
 */
CPP_TOOLBOX_EXPORT auto get_file_extension(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief Copy a file
 * @param src The source file
 * @param dst The destination file
 * @return True if the file was copied successfully
 */
CPP_TOOLBOX_EXPORT auto copy_file(const std::filesystem::path& src,
                                  const std::filesystem::path& dst) -> bool;

/**
 * @brief Move a file
 * @param src The source file
 * @param dst The destination file
 * @return True if the file was moved successfully
 */
CPP_TOOLBOX_EXPORT auto move_file(const std::filesystem::path& src,
                                  const std::filesystem::path& dst) -> bool;

/**
 * @brief Delete a file at the specified path
 * @param path The filesystem path to the file to delete
 * @return True if the file was successfully deleted, false otherwise
 *
 * @example
 * // Delete an existing file
 * if (delete_file("/tmp/test.txt")) {
 *     std::cout << "File deleted successfully\n";
 * } else {
 *     std::cerr << "Failed to delete file\n";
 * }
 *
 * @example
 * // Attempt to delete non-existent file
 * bool result = delete_file("/path/to/nonexistent.file");
 * // result will be false
 */
CPP_TOOLBOX_EXPORT auto delete_file(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a file exists at the specified path
 * @param path The filesystem path to check
 * @return True if the file exists, false otherwise
 *
 * @example
 * // Check if file exists
 * if (file_exists("/etc/passwd")) {
 *     std::cout << "File exists\n";
 * } else {
 *     std::cout << "File does not exist\n";
 * }
 *
 * @example
 * // Check non-existent file
 * bool exists = file_exists("/tmp/nonexistent.file");
 * // exists will be false
 */
CPP_TOOLBOX_EXPORT auto file_exists(const std::filesystem::path& path) -> bool;

/**
 * @brief Create a directory at the specified path
 * @param path The filesystem path where the directory should be created
 * @return True if the directory was created successfully, false otherwise
 *
 * @example
 * // Create a new directory
 * if (create_directory("/tmp/new_dir")) {
 *     std::cout << "Directory created\n";
 * } else {
 *     std::cerr << "Failed to create directory\n";
 * }
 *
 * @example
 * // Create nested directories
 * create_directory("/tmp/a/b/c");  // Creates all directories in path
 */
CPP_TOOLBOX_EXPORT auto create_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a directory exists at the specified path
 * @param path The filesystem path to check
 * @return True if the directory exists, false otherwise
 *
 * @example
 * // Check if directory exists
 * if (directory_exists("/usr/local/bin")) {
 *     std::cout << "Directory exists\n";
 * } else {
 *     std::cout << "Directory does not exist\n";
 * }
 *
 * @example
 * // Check non-existent directory
 * bool exists = directory_exists("/tmp/nonexistent_dir");
 * // exists will be false
 */
CPP_TOOLBOX_EXPORT auto directory_exists(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Delete a directory at the specified path
 * @param path The filesystem path of the directory to delete
 * @return True if the directory was deleted successfully, false otherwise
 *
 * @example
 * // Delete an empty directory
 * if (delete_directory("/tmp/empty_dir")) {
 *     std::cout << "Directory deleted\n";
 * } else {
 *     std::cerr << "Failed to delete directory\n";
 * }
 *
 * @example
 * // Delete non-empty directory
 * delete_directory("/tmp/non_empty_dir");  // Will fail unless recursive
 */
CPP_TOOLBOX_EXPORT auto delete_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Get the current working directory
 * @return The current working directory as a filesystem path
 *
 * @example
 * // Print current directory
 * auto current_dir = get_current_directory();
 * std::cout << "Current directory: " << current_dir << "\n";
 *
 * @example
 * // Change and get current directory
 * std::filesystem::current_path("/tmp");
 * auto dir = get_current_directory();  // Returns "/tmp"
 */
CPP_TOOLBOX_EXPORT auto get_current_directory() -> std::filesystem::path;

/**
 * @brief Get the type of the filesystem object at the specified path
 * @param path The filesystem path to check
 * @return The type of the filesystem object (file, directory, etc.)
 *
 * @example
 * // Check type of a file
 * auto type = path_type("/etc/passwd");
 * if (type == std::filesystem::file_type::regular) {
 *     std::cout << "Regular file\n";
 * }
 *
 * @example
 * // Check type of a directory
 * auto dir_type = path_type("/usr/local/bin");
 * if (dir_type == std::filesystem::file_type::directory) {
 *     std::cout << "Directory\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto path_type(const std::filesystem::path& path)
    -> std::filesystem::file_type;

/**
 * @brief Check if a path is a regular file
 * @param path The path to check
 * @return True if the path is a regular file, false otherwise
 *
 * @example
 * // Check if a file exists and is regular
 * if (is_regular_file("/path/to/file.txt")) {
 *     std::cout << "Regular file exists\n";
 * } else {
 *     std::cerr << "Not a regular file\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_regular_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is a directory
 * @param path The path to check
 * @return True if the path is a directory, false otherwise
 *
 * @example
 * // Check if a directory exists
 * if (is_directory("/path/to/dir")) {
 *     std::cout << "Directory exists\n";
 * } else {
 *     std::cerr << "Not a directory\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_directory(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a symbolic link
 * @param path The path to check
 * @return True if the path is a symbolic link, false otherwise
 *
 * @example
 * // Check if a path is a symlink
 * if (is_symlink("/path/to/symlink")) {
 *     std::cout << "Symbolic link detected\n";
 * } else {
 *     std::cerr << "Not a symbolic link\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_symlink(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a FIFO (named pipe)
 * @param path The path to check
 * @return True if the path is a FIFO, false otherwise
 *
 * @example
 * // Check if a path is a FIFO
 * if (is_fifo("/path/to/fifo")) {
 *     std::cout << "FIFO detected\n";
 * } else {
 *     std::cerr << "Not a FIFO\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_fifo(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a socket
 * @param path The path to check
 * @return True if the path is a socket, false otherwise
 *
 * @example
 * // Check if a path is a socket
 * if (is_socket("/path/to/socket")) {
 *     std::cout << "Socket detected\n";
 * } else {
 *     std::cerr << "Not a socket\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_socket(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a block device
 * @param path The path to check
 * @return True if the path is a block device, false otherwise
 *
 * @example
 * // Check if a path is a block device
 * if (is_block_file("/dev/sda1")) {
 *     std::cout << "Block device detected\n";
 * } else {
 *     std::cerr << "Not a block device\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_block_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is a character device
 * @param path The path to check
 * @return True if the path is a character device, false otherwise
 *
 * @example
 * // Check if a path is a character device
 * if (is_character_file("/dev/tty")) {
 *     std::cout << "Character device detected\n";
 * } else {
 *     std::cerr << "Not a character device\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_character_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is of an unknown type
 * @param path The path to check
 * @return True if the path is of an unknown type, false otherwise
 *
 * @example
 * // Check if a path is of unknown type
 * if (is_other("/path/to/unknown")) {
 *     std::cout << "Unknown file type detected\n";
 * } else {
 *     std::cerr << "File type is known\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_other(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is empty
 * @param path The path to check
 * @return True if the path is empty, false otherwise
 *
 * @example
 * // Check if a directory is empty
 * if (is_empty("/path/to/dir")) {
 *     std::cout << "Directory is empty\n";
 * } else {
 *     std::cerr << "Directory is not empty\n";
 * }
 */
CPP_TOOLBOX_EXPORT auto is_empty(const std::filesystem::path& path) -> bool;

/**
 * @brief Get the creation time of a file
 * @param path The path to check
 * @return The creation time of the file
 * @throws std::filesystem::filesystem_error if the file doesn't exist or can't
 * be accessed
 *
 * @example
 * // Get creation time of a file
 * try {
 *     auto creation_time = get_creation_time("/path/to/file.txt");
 *     std::cout << "File created at: " << creation_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_creation_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the last access time of a file
 * @param path The path to check
 * @return The last access time as file_time_type
 * @throws std::filesystem::filesystem_error if the file doesn't exist or can't
 * be accessed
 *
 * @example
 * // Get last access time of a file
 * try {
 *     auto access_time = get_last_access_time("/path/to/file.txt");
 *     std::cout << "Last accessed: " << access_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_last_access_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the last write time of a file
 * @param path The path to check
 * @return The last write time as file_time_type
 * @throws std::filesystem::filesystem_error if the file doesn't exist or can't
 * be accessed
 *
 * @example
 * // Get last modification time of a file
 * try {
 *     auto write_time = get_last_write_time("/path/to/file.txt");
 *     std::cout << "Last modified: " << write_time << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_last_write_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the target of a symbolic link
 * @param path The path to the symbolic link
 * @return The target path as std::filesystem::path
 * @throws std::filesystem::filesystem_error if the path is not a symbolic link
 *
 * @example
 * // Get symlink target
 * try {
 *     auto target = get_symlink_target("/path/to/symlink");
 *     std::cout << "Symlink points to: " << target << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_symlink_target(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Traverse a directory
 * @param path The path to traverse
 * @return A vector of paths in the directory
 * @throws std::filesystem::filesystem_error if the path is not a directory
 *
 * @example
 * // List files in a directory
 * try {
 *     auto files = traverse_directory("/path/to/dir");
 *     for (const auto& file : files) {
 *         std::cout << file << "\n";
 *     }
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto traverse_directory(const std::filesystem::path& path)
    -> std::vector<std::filesystem::path>;

/**
 * @brief Recursively traverse a directory
 * @param path The path to traverse
 * @return A vector of paths in the directory and its subdirectories
 * @throws std::filesystem::filesystem_error if the path is not a directory
 *
 * @example
 * // Recursively list files in a directory
 * try {
 *     auto all_files = recursive_traverse_directory("/path/to/dir");
 *     for (const auto& file : all_files) {
 *         std::cout << file << "\n";
 *     }
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory(
    const std::filesystem::path& path) -> std::vector<std::filesystem::path>;

/**
 * @brief Traverse a directory and call a callback for each file
 * @param path The path to traverse
 * @param callback The callback to call for each file
 * @throws std::filesystem::filesystem_error if the path is not a directory
 *
 * @example
 * // Process each file in a directory
 * try {
 *     traverse_directory_files("/path/to/dir", [](const auto& file) {
 *         std::cout << "Processing: " << file << "\n";
 *     });
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief Recursively traverse a directory and call a callback for each file
 * @param path The path to traverse
 * @param callback The callback to call for each file
 * @throws std::filesystem::filesystem_error if the path is not a directory
 *
 * @example
 * // Process each file recursively
 * try {
 *     recursive_traverse_directory_files("/path/to/dir", [](const auto& file) {
 *         std::cout << "Processing: " << file << "\n";
 *     });
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief Get the current working directory
 * @return The current working directory as std::filesystem::path
 * @throws std::filesystem::filesystem_error if the current directory cannot be
 * determined
 *
 * @example
 * // Print current working directory
 * try {
 *     auto cwd = get_current_working_directory();
 *     std::cout << "Current directory: " << cwd << "\n";
 * } catch (const std::filesystem::filesystem_error& e) {
 *     std::cerr << "Error: " << e.what() << '\n';
 * }
 */
CPP_TOOLBOX_EXPORT auto get_current_working_directory()
    -> std::filesystem::path;

}  // namespace toolbox::file

#pragma once
#include <filesystem>
#include <functional>
#include <vector>

#include "cpp-toolbox/macro.hpp"

namespace toolbox::file
{

using file_time_type = std::filesystem::file_time_type;
using file_type = std::filesystem::file_type;

/**
 * @brief Convert a string to a path
 * @param str The string to convert
 * @return The converted path
 */
CPP_TOOLBOX_EXPORT auto string_to_path(const std::string& str)
    -> std::filesystem::path;

/**
 * @brief Convert a path to a string
 * @param path The path to convert
 * @return The converted string
 */
CPP_TOOLBOX_EXPORT auto path_to_string(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief Get the size of a file
 * @param path The path to the file
 * @return The size of the file
 */
CPP_TOOLBOX_EXPORT auto get_file_size(const std::filesystem::path& path)
    -> std::size_t;

/**
 * @brief Get the absolute path of a file
 * @param path The path to convert
 * @return The absolute path
 */
CPP_TOOLBOX_EXPORT auto get_absolute_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Get the relative path of a file
 * @param path The path to convert
 * @param base The base path
 * @return The relative path
 */
CPP_TOOLBOX_EXPORT auto get_relative_path(const std::filesystem::path& path,
                                          const std::filesystem::path& base)
    -> std::filesystem::path;

/**
 * @brief Get the parent path of a file
 * @param path The path to get the parent of
 * @return The parent path
 */
CPP_TOOLBOX_EXPORT auto get_parent_path(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Get the file name of a file
 * @param path The path to get the file name of
 * @return The file name
 */
CPP_TOOLBOX_EXPORT auto get_file_name(const std::filesystem::path& path)
    -> std::string;

/**
 * @brief Get the file extension of a file
 * @param path The path to get the file extension of
 * @return The file extension
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
 * @brief Delete a file
 * @param path The path to the file
 * @return True if the file was deleted successfully
 */
CPP_TOOLBOX_EXPORT auto delete_file(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a file exists
 * @param path The path to check
 * @return True if the file exists
 */
CPP_TOOLBOX_EXPORT auto file_exists(const std::filesystem::path& path) -> bool;

/**
 * @brief Create a directory
 * @param path The path to create
 * @return True if the directory was created successfully
 */
CPP_TOOLBOX_EXPORT auto create_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a directory exists
 * @param path The path to check
 * @return True if the directory exists
 */
CPP_TOOLBOX_EXPORT auto directory_exists(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Delete a directory
 * @param path The path to delete
 * @return True if the directory was deleted successfully
 */
CPP_TOOLBOX_EXPORT auto delete_directory(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Get the current directory
 * @return The current directory
 */
CPP_TOOLBOX_EXPORT auto get_current_directory() -> std::filesystem::path;

/**
 * @brief Get the type of a path
 * @param path The path to check
 * @return The type of the path
 */
CPP_TOOLBOX_EXPORT auto path_type(const std::filesystem::path& path)
    -> std::filesystem::file_type;

/**
 * @brief Check if a path is a regular file
 * @param path The path to check
 * @return True if the path is a regular file
 */
CPP_TOOLBOX_EXPORT auto is_regular_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is a directory
 * @param path The path to check
 * @return True if the path is a directory
 */
CPP_TOOLBOX_EXPORT auto is_directory(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a symbolic link
 * @param path The path to check
 * @return True if the path is a symbolic link
 */
CPP_TOOLBOX_EXPORT auto is_symlink(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a FIFO
 * @param path The path to check
 * @return True if the path is a FIFO
 */
CPP_TOOLBOX_EXPORT auto is_fifo(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a socket
 * @param path The path to check
 * @return True if the path is a socket
 */
CPP_TOOLBOX_EXPORT auto is_socket(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is a block file
 * @param path The path to check
 * @return True if the path is a block file
 */
CPP_TOOLBOX_EXPORT auto is_block_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is a character file
 * @param path The path to check
 * @return True if the path is a character file
 */
CPP_TOOLBOX_EXPORT auto is_character_file(const std::filesystem::path& path)
    -> bool;

/**
 * @brief Check if a path is of an unknown type
 * @param path The path to check
 * @return True if the path is of an unknown type
 */
CPP_TOOLBOX_EXPORT auto is_other(const std::filesystem::path& path) -> bool;

/**
 * @brief Check if a path is empty
 * @param path The path to check
 * @return True if the path is empty
 */
CPP_TOOLBOX_EXPORT auto is_empty(const std::filesystem::path& path) -> bool;

/**
 * @brief Get the creation time of a file
 * @param path The path to check
 * @return The creation time
 */
CPP_TOOLBOX_EXPORT auto get_creation_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the last access time of a file
 * @param path The path to check
 * @return The last access time
 */
CPP_TOOLBOX_EXPORT auto get_last_access_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the last write time of a file
 * @param path The path to check
 * @return The last write time
 */
CPP_TOOLBOX_EXPORT auto get_last_write_time(const std::filesystem::path& path)
    -> file_time_type;

/**
 * @brief Get the target of a symbolic link
 * @param path The path to check
 * @return The target path
 */
CPP_TOOLBOX_EXPORT auto get_symlink_target(const std::filesystem::path& path)
    -> std::filesystem::path;

/**
 * @brief Traverse a directory
 * @param path The path to traverse
 * @return A vector of paths in the directory
 */
CPP_TOOLBOX_EXPORT auto traverse_directory(const std::filesystem::path& path)
    -> std::vector<std::filesystem::path>;

/**
 * @brief Recursively traverse a directory
 * @param path The path to traverse
 * @return A vector of paths in the directory and its subdirectories
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory(
    const std::filesystem::path& path) -> std::vector<std::filesystem::path>;

/**
 * @brief Traverse a directory and call a callback for each file
 * @param path The path to traverse
 * @param callback The callback to call for each file
 */
CPP_TOOLBOX_EXPORT auto traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief Recursively traverse a directory and call a callback for each file
 *
 * @param path The path to traverse
 * @param callback The callback to call for each file
 *
 * @details This function will traverse the directory and its subdirectories
 * and call the callback for each file found.
 *
 * @note The callback will be called with the full path to the file.
 */
CPP_TOOLBOX_EXPORT auto recursive_traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void;

/**
 * @brief Get the current working directory
 *
 * @return The current working directory
 *
 * @note This function is used to get the current working directory of the
 * process. The current working directory is the directory from which the
 * process was executed.
 *
 * @see https://en.cppreference.com/w/cpp/filesystem/current_path
 */
CPP_TOOLBOX_EXPORT auto get_current_working_directory()
    -> std::filesystem::path;

}  // namespace toolbox::file

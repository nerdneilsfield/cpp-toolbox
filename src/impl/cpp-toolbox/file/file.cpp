#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "cpp-toolbox/file/file.hpp"

#include "cpp-toolbox/macro.hpp"

#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
// Define NOMINMAX before including windows.h to prevent min/max macro
// definitions
#  define NOMINMAX
#  include <windows.h>
#elif defined(CPP_TOOLBOX_PLATFORM_MACOS)
#  include <sys/stat.h>
#else
#  include <sys/stat.h>
#endif

namespace toolbox::file
{

auto string_to_path(const std::string& str) -> std::filesystem::path
{
  return {str};
}

auto path_to_string(const std::filesystem::path& path) -> std::string
{
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
  auto u8 = path.u8string();
  return std::string(u8.begin(), u8.end());
#else
  return path.string();
#endif
}

auto get_current_working_directory() -> std::filesystem::path
{
  return std::filesystem::current_path();
}

// auto set_current_working_directory(const std::filesystem::path& path) -> void
// {
//     std::filesystem::current_path(path);
// }

auto get_file_size(const std::filesystem::path& path) -> std::size_t
{
  return std::filesystem::file_size(path);
}

auto get_absolute_path(const std::filesystem::path& path)
    -> std::filesystem::path
{
  return std::filesystem::absolute(path);
}

auto get_relative_path(const std::filesystem::path& path,
                       const std::filesystem::path& base)
    -> std::filesystem::path
{
  return std::filesystem::relative(path, base);
}

auto get_parent_path(const std::filesystem::path& path) -> std::filesystem::path
{
  return path.parent_path();
}

auto get_file_name(const std::filesystem::path& path) -> std::string
{
  return path.filename().string();
}

auto get_file_extension(const std::filesystem::path& path) -> std::string
{
  return path.extension().string();
}

auto copy_file(const std::filesystem::path& src,
               const std::filesystem::path& dst) -> bool
{
  try {
    std::filesystem::copy_file(src, dst);
    return true;
  } catch (const std::filesystem::filesystem_error& /*e*/) {
    return false;
  }
}

auto move_file(const std::filesystem::path& src,
               const std::filesystem::path& dst) -> bool
{
  try {
    std::filesystem::rename(src, dst);
    return true;
  } catch (const std::filesystem::filesystem_error& /*e*/) {
    return false;
  }
}

auto delete_file(const std::filesystem::path& path) -> bool
{
  try {
    std::filesystem::remove(path);
    return true;
  } catch (const std::filesystem::filesystem_error& /*e*/) {
    return false;
  }
}

auto file_exists(const std::filesystem::path& path) -> bool
{
  return std::filesystem::exists(path);
}

auto create_directory(const std::filesystem::path& path) -> bool
{
  return std::filesystem::create_directory(path);
}

auto directory_exists(const std::filesystem::path& path) -> bool
{
  return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

auto delete_directory(const std::filesystem::path& path) -> bool
{
  try {
    std::filesystem::remove_all(path);
    return true;
  } catch (const std::filesystem::filesystem_error& /*e*/) {
    return false;
  }
}

auto get_current_directory() -> std::filesystem::path
{
  return std::filesystem::current_path();
}

auto path_type(const std::filesystem::path& path) -> std::filesystem::file_type
{
  return std::filesystem::status(path).type();
}

auto is_regular_file(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_regular_file(path);
}

auto is_directory(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_directory(path);
}

auto is_symlink(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_symlink(path);
}

auto is_fifo(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_fifo(path);
}

auto is_socket(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_socket(path);
}

auto is_block_file(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_block_file(path);
}

auto is_character_file(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_character_file(path);
}

auto is_other(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_other(path);
}

auto is_empty(const std::filesystem::path& path) -> bool
{
  return std::filesystem::is_empty(path);
}

auto get_creation_time(const std::filesystem::path& path) -> file_time_type
{
  try {
    if (!std::filesystem::exists(path)) {
      return file_time_type::min();
    }

#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
    HANDLE hFile = CreateFileW(path.c_str(),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
      return file_time_type::min();
    }

    FILETIME creationTime;
    if (GetFileTime(hFile, &creationTime, NULL, NULL)) {
      CloseHandle(hFile);
      // 转换 Windows FILETIME 到 file_time_type
      return file_time_type::clock::now();  // 这里需要正确转换
    }
    CloseHandle(hFile);
#elif defined(CPP_TOOLBOX_PLATFORM_MACOS)
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0) {
      auto system_time =
          std::chrono::system_clock::from_time_t(statbuf.st_birthtime);
      auto file_system_duration =
          std::chrono::duration_cast<std::filesystem::file_time_type::duration>(
              system_time.time_since_epoch());
      return std::filesystem::file_time_type(file_system_duration);
    }
#else
    struct stat statbuf {};
    if (stat(path.c_str(), &statbuf) == 0) {
      auto sctp = std::chrono::system_clock::from_time_t(statbuf.st_ctime);
      auto duration = sctp.time_since_epoch();
      return file_time_type(duration);
    }
#endif

    return file_time_type::min();
  } catch (...) {
    return file_time_type::min();
  }
}

auto get_last_access_time(const std::filesystem::path& path) -> file_time_type
{
  if (std::filesystem::exists(path)) {
    return std::filesystem::last_write_time(path);
  }
  return file_time_type::min();
}

auto get_last_write_time(const std::filesystem::path& path) -> file_time_type
{
  if (std::filesystem::exists(path)) {
    return std::filesystem::last_write_time(path);
  }
  return file_time_type::min();
}

auto get_symlink_target(const std::filesystem::path& path)
    -> std::filesystem::path
{
  return std::filesystem::read_symlink(path);
}

auto traverse_directory(const std::filesystem::path& path)
    -> std::vector<std::filesystem::path>
{
  std::vector<std::filesystem::path> result;
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    result.push_back(entry.path());
  }
  return result;
}

auto list_files_in_directory(const std::filesystem::path& path,
                             const std::string& extension)
    -> std::vector<std::filesystem::path>
{
  if (extension.empty()) {
    return traverse_directory(path);
  }
  std::vector<std::filesystem::path> result;
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == extension) {
      result.push_back(entry.path());
    }
  }
  return result;
}

auto list_files_in_directory(const std::filesystem::path& path,
                             const std::vector<std::string>& extensions)
    -> std::vector<std::filesystem::path>
{
  if (extensions.empty()) {
    return traverse_directory(path);
  }
  std::vector<std::filesystem::path> result;
  for (const auto& extension : extensions) {
    auto files = list_files_in_directory(path, extension);
    result.insert(result.end(), files.begin(), files.end());
  }
  return result;
}

auto recursive_traverse_directory(const std::filesystem::path& path)
    -> std::vector<std::filesystem::path>
{
  std::vector<std::filesystem::path> result;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
  {
    result.push_back(entry.path());
  }
  return result;
}

auto traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void
{
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (std::filesystem::is_regular_file(entry.path())) {
      callback(entry.path());
    }
  }
}

auto recursive_traverse_directory_files(
    const std::filesystem::path& path,
    std::function<void(const std::filesystem::path&)>& callback) -> void
{
  for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
  {
    if (std::filesystem::is_regular_file(entry.path())) {
      callback(entry.path());
    }
  }
}
}  // namespace toolbox::file

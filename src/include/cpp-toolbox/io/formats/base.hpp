#pragma once

#include <memory>  // For std::unique_ptr
#include <string>  // For std::string
#include <vector>  // For std::vector

#include <cpp-toolbox/file/file.hpp>

namespace toolbox::io
{

// Base class for data loaded from files.
class base_file_data_t
{
public:
  virtual ~base_file_data_t() =
      default;  // Virtual destructor is important for base classes
  // Rule of Five/Zero: If a virtual destructor is defined, consider the others.
  // Since this class has no resources needing special management, default is
  // fine.
  base_file_data_t(const base_file_data_t&) = default;
  base_file_data_t& operator=(const base_file_data_t&) = default;
  base_file_data_t(base_file_data_t&&) = default;
  base_file_data_t& operator=(base_file_data_t&&) = default;

protected:
  base_file_data_t() = default;  // Allow construction only by derived classes
};

// Base class for file format readers/writers.
class base_file_format_t
{
public:
  virtual ~base_file_format_t() = default;
  // This class is likely stateless or manages resources via RAII (like
  // unique_ptr in derived classes) Default copy/move operations are probably
  // fine, but deleting them is safer if it's meant to be non-copyable/movable.
  base_file_format_t(const base_file_format_t&) =
      delete;  // Typically format handlers are not copied
  base_file_format_t& operator=(const base_file_format_t&) = delete;
  base_file_format_t(base_file_format_t&&) = default;  // Moving might be okay
  base_file_format_t& operator=(base_file_format_t&&) = default;

  [[nodiscard]] virtual auto can_read(const std::string& path) const
      -> bool = 0;
  [[nodiscard]] virtual auto get_supported_extensions() const
      -> std::vector<std::string> = 0;
  virtual auto read(const std::string& path,
                    std::unique_ptr<base_file_data_t>& data) -> bool = 0;
  [[nodiscard]] virtual auto write(
      const std::string& path,
      const std::unique_ptr<base_file_data_t>& data) const -> bool = 0;

protected:
  base_file_format_t() = default;
};

}  // namespace toolbox::io

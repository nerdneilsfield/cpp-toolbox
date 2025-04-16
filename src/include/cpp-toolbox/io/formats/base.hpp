#pragma once

#include <memory>  // For std::unique_ptr
#include <string>  // For std::string
#include <vector>  // For std::vector

#include <cpp-toolbox/file/file.hpp>

namespace toolbox::io
{

/**
 * @brief 文件数据的基类 / Base class for data loaded from files
 *
 * @details
 * 这个类作为所有文件数据类型的基类,提供了虚析构函数和默认的拷贝/移动语义 / This
 * class serves as a base for all file data types, providing virtual destructor
 * and default copy/move semantics
 *
 * @code{.cpp}
 * // 定义一个派生的文件数据类 / Define a derived file data class
 * class text_file_data_t : public base_file_data_t {
 * public:
 *     std::string content;
 * };
 *
 * // 使用智能指针管理数据 / Use smart pointer to manage data
 * auto data = std::make_unique<text_file_data_t>();
 * data->content = "Hello World";
 * @endcode
 */
class base_file_data_t
{
public:
  virtual ~base_file_data_t() =
      default;  // 虚析构函数对基类很重要 / Virtual destructor is important for
                // base classes
  // Rule of Five/Zero: 如果定义了虚析构函数,需要考虑其他特殊成员函数 / If a
  // virtual destructor is defined, consider the others
  // 由于这个类没有需要特殊管理的资源,使用默认实现即可 / Since this class has no
  // resources needing special management, default is fine
  base_file_data_t(const base_file_data_t&) = default;
  base_file_data_t& operator=(const base_file_data_t&) = default;
  base_file_data_t(base_file_data_t&&) = default;
  base_file_data_t& operator=(base_file_data_t&&) = default;

protected:
  base_file_data_t() =
      default;  // 只允许派生类构造 / Allow construction only by derived classes
};

/**
 * @brief 文件格式读写器的基类 / Base class for file format readers/writers
 *
 * @details 定义了文件格式处理的通用接口,包括读写和格式检测功能 / Defines common
 * interface for file format handling, including read/write and format detection
 * capabilities
 *
 * @code{.cpp}
 * // 实现一个简单的文本文件格式处理器 / Implement a simple text file format
 * handler class text_format_t : public base_file_format_t { public: bool
 * can_read(const std::string& path) const override { return
 * path.ends_with(".txt");
 *     }
 *
 *     std::vector<std::string> get_supported_extensions() const override {
 *         return {".txt"};
 *     }
 *
 *     bool read(const std::string& path,
 *              std::unique_ptr<base_file_data_t>& data) override {
 *         // 读取实现 / Read implementation
 *         return true;
 *     }
 *
 *     bool write(const std::string& path,
 *               const std::unique_ptr<base_file_data_t>& data) const override {
 *         // 写入实现 / Write implementation
 *         return true;
 *     }
 * };
 * @endcode
 */
class base_file_format_t
{
public:
  virtual ~base_file_format_t() = default;
  // 这个类可能是无状态的或通过RAII管理资源(如派生类中的unique_ptr) / This class
  // is likely stateless or manages resources via RAII
  // 默认的拷贝/移动操作可能没问题,但删除它们更安全 / Default copy/move
  // operations are probably fine, but deleting them is safer
  base_file_format_t(const base_file_format_t&) =
      delete;  // 通常格式处理器不需要拷贝 / Typically format handlers are not
               // copied
  base_file_format_t& operator=(const base_file_format_t&) = delete;
  base_file_format_t(base_file_format_t&&) =
      default;  // 移动可能是可以的 / Moving might be okay
  base_file_format_t& operator=(base_file_format_t&&) = default;

  /**
   * @brief 检查是否可以读取指定路径的文件 / Check if file at given path can be
   * read
   * @param path 要检查的文件路径 / Path to file to check
   * @return 如果文件可以读取则返回true / Returns true if file can be read
   */
  [[nodiscard]] virtual auto can_read(const std::string& path) const
      -> bool = 0;

  /**
   * @brief 获取支持的文件扩展名列表 / Get list of supported file extensions
   * @return 支持的扩展名字符串向量 / Vector of supported extension strings
   */
  [[nodiscard]] virtual auto get_supported_extensions() const
      -> std::vector<std::string> = 0;

  /**
   * @brief 读取文件内容到数据对象 / Read file contents into data object
   * @param path 要读取的文件路径 / Path to file to read
   * @param data 用于存储读取数据的指针 / Pointer to store read data
   * @return 读取成功返回true / Returns true if read successful
   */
  virtual auto read(const std::string& path,
                    std::unique_ptr<base_file_data_t>& data) -> bool = 0;

  /**
   * @brief 将数据写入文件 / Write data to file
   * @param path 要写入的文件路径 / Path to file to write
   * @param data 要写入的数据 / Data to write
   * @return 写入成功返回true / Returns true if write successful
   */
  [[nodiscard]] virtual auto write(
      const std::string& path,
      const std::unique_ptr<base_file_data_t>& data) const -> bool = 0;

protected:
  base_file_format_t() = default;
};

}  // namespace toolbox::io

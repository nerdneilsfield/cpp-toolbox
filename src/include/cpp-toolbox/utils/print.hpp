#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cpp-toolbox/base/env.hpp"
#include "cpp-toolbox/cpp-toolbox_export.hpp"
#include "cpp-toolbox/macro.hpp"

#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  include <windows.h>
#endif

namespace cpp_toolbox::utils
{
/**
 * @enum align_t
 * @brief 对齐方式枚举/Alignment type enumeration
 */
enum class CPP_TOOLBOX_EXPORT align_t : std::uint8_t
{
  LEFT,  ///< 左对齐/Left align
  RIGHT,  ///< 右对齐/Right align
  CENTER  ///< 居中对齐/Center align
};

/**
 * @enum color_t
 * @brief 颜色支持枚举/Color support enumeration
 */
enum class CPP_TOOLBOX_EXPORT color_t : std::uint8_t
{
  DEFAULT,  ///< 默认颜色/Default color
  BLACK,  ///< 黑色/Black
  RED,  ///< 红色/Red
  GREEN,  ///< 绿色/Green
  YELLOW,  ///< 黄色/Yellow
  BLUE,  ///< 蓝色/Blue
  MAGENTA,  ///< 品红/Magenta
  CYAN,  ///< 青色/Cyan
  WHITE  ///< 白色/White
};

/**
 * @struct print_style_t
 * @brief 打印风格设置/Print style settings
 *
 * 用于自定义表格和容器打印的样式/Used to customize the style of table and
 * container printing
 */
struct CPP_TOOLBOX_EXPORT print_style_t
{
  std::string border_h = "-";  ///< 水平边框字符/Horizontal border character
  std::string border_v = "|";  ///< 垂直边框字符/Vertical border character
  std::string corner = "+";  ///< 角字符/Corner character
  std::string padding = " ";  ///< 单元格填充/Cell padding
  bool show_header = true;  ///< 是否显示表头/Show header or not
  bool show_border = true;  ///< 是否显示边框/Show border or not
  size_t min_width = 0;  ///< 最小列宽/Minimum column width
  align_t alignment = align_t::LEFT;  ///< 默认对齐方式/Default alignment
  color_t header_fg = color_t::DEFAULT;  ///< 表头前景色/Header foreground color
  color_t header_bg = color_t::DEFAULT;  ///< 表头背景色/Header background color
  color_t data_fg = color_t::DEFAULT;  ///< 数据前景色/Data foreground color
  color_t data_bg = color_t::DEFAULT;  ///< 数据背景色/Data background color
  color_t border_color = color_t::DEFAULT;  ///< 边框颜色/Border color
  bool use_colors = true;  ///< 是否使用颜色/Use color or not
};

/**
 * @brief 获取默认打印风格/Get default print style
 * @return const print_style_t& 默认风格引用/Reference to default style
 * @code
 * const auto& style = cpp_toolbox::utils::get_default_style();
 * @endcode
 */
inline auto get_default_style() -> const print_style_t&
{
  static const print_style_t style = print_style_t();
  return style;
}

/**
 * @class color_handler_t
 * @brief 平台无关的颜色处理类/Platform-independent color handler
 *
 * 用于处理终端颜色输出/Used for handling terminal color output
 */
class CPP_TOOLBOX_EXPORT color_handler_t
{
  inline static bool is_windows =
      false;  ///< 是否为Windows平台/Is Windows platform
  inline static bool has_color_support =
      false;  ///< 是否支持颜色/Has color support

  /**
   * @brief 初始化颜色支持/Initialize color support
   */
  static void initialize()
  {
    static bool initialized = false;
    if (!initialized) {
#ifdef _WIN32
      is_windows = true;
      // 在Windows上检查并启用ANSI支持/Check and enable ANSI support on Windows
      HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
      DWORD dwMode = 0;
      has_color_support = GetConsoleMode(hOut, &dwMode)
          && SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
      is_windows = false;
      // 使用 toolbox::base::get_environment_variable 替换 std::getenv/Use
      // toolbox::base::get_environment_variable instead of std::getenv
      const std::string term_str =
          toolbox::base::get_environment_variable("TERM");
      has_color_support =
          !term_str.empty()  // 显式检查非空/Explicitly check non-empty
          && (term_str == "xterm" || term_str == "xterm-color"
              || term_str == "xterm-256color" || term_str == "screen"
              || term_str == "screen-256color" || term_str == "tmux"
              || term_str == "tmux-256color" || term_str == "rxvt"
              || term_str == "rxvt-unicode" || term_str == "linux"
              || term_str == "cygwin");
#endif
      initialized = true;
    }
  }

public:
  /**
   * @brief 获取前景色ANSI代码/Get ANSI code for foreground color
   * @param color 颜色枚举/Color enum
   * @return std::string ANSI转义码/ANSI escape code
   */
  static auto get_fg_code(color_t color) -> std::string
  {
    initialize();
    if (!has_color_support || color == color_t::DEFAULT) {
      return "";
    }

    switch (color) {
      case color_t::BLACK:
        return "\033[30m";
      case color_t::RED:
        return "\033[31m";
      case color_t::GREEN:
        return "\033[32m";
      case color_t::YELLOW:
        return "\033[33m";
      case color_t::BLUE:
        return "\033[34m";
      case color_t::MAGENTA:
        return "\033[35m";
      case color_t::CYAN:
        return "\033[36m";
      case color_t::WHITE:
        return "\033[37m";
      default:
        return "";
    }
  }

  /**
   * @brief 获取背景色ANSI代码/Get ANSI code for background color
   * @param color 颜色枚举/Color enum
   * @return std::string ANSI转义码/ANSI escape code
   */
  static auto get_bg_code(color_t color) -> std::string
  {
    initialize();
    if (!has_color_support || color == color_t::DEFAULT) {
      return "";
    }

    switch (color) {
      case color_t::BLACK:
        return "\033[40m";
      case color_t::RED:
        return "\033[41m";
      case color_t::GREEN:
        return "\033[42m";
      case color_t::YELLOW:
        return "\033[43m";
      case color_t::BLUE:
        return "\033[44m";
      case color_t::MAGENTA:
        return "\033[45m";
      case color_t::CYAN:
        return "\033[46m";
      case color_t::WHITE:
        return "\033[47m";
      default:
        return "";
    }
  }

  /**
   * @brief 重置所有颜色/Reset all colors
   * @return std::string ANSI重置码/ANSI reset code
   */
  static auto reset() -> std::string
  {
    initialize();
    return has_color_support ? "\033[0m" : "";
  }

  /**
   * @brief 应用颜色到文本/Apply color to text
   * @param text 文本内容/Text content
   * @param fg 前景色/Foreground color
   * @param bg 背景色/Background color
   * @return std::string 带颜色的文本/Colored text
   * @code
   * std::string colored = color_handler_t::colorize("Hello", color_t::RED,
   * color_t::DEFAULT);
   * @endcode
   */
  static auto colorize(const std::string& text, color_t fg, color_t bg)
      -> std::string
  {
    initialize();
    if (!has_color_support
        || (fg == color_t::DEFAULT && bg == color_t::DEFAULT))
    {
      return text;
    }

    std::string result;
    result.reserve(text.size()
                   + 20);  // 预留ANSI代码空间/Reserve space for ANSI codes

    result += get_fg_code(fg);
    result += get_bg_code(bg);
    result += text;
    result += reset();

    return result;
  }

  /**
   * @brief 检查是否支持颜色/Check if color is supported
   * @return bool 是否支持/Whether color is supported
   * @code
   * if (color_handler_t::supports_color()) { ... }
   * @endcode
   */
  static auto supports_color() -> bool
  {
    initialize();
    return has_color_support;
  }
};

/**
 * @brief 获取字符串表示形式的通用函数/Generic function to get string
 * representation
 * @tparam T 任意类型/Any type
 * @param value 输入值/Input value
 * @return std::string 字符串/String
 * @code
 * std::string s = to_string_value(123); // "123"
 * @endcode
 */
template<typename T>
auto to_string_value(const T& value) -> std::string
{
  if constexpr (std::is_same_v<T, std::string>) {
    return value;
  } else {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
}

/**
 * @brief 处理对齐功能/Handle text alignment
 * @param text 输入文本/Input text
 * @param width 目标宽度/Target width
 * @param align 对齐方式/Alignment
 * @param padding 填充字符/Padding character
 * @return std::string 对齐后的文本/Aligned text
 * @code
 * std::string s = align_text("abc", 6, align_t::RIGHT); // "   abc"
 * @endcode
 */
inline auto align_text(const std::string& text,
                       size_t width,
                       align_t align,
                       [[maybe_unused]] const std::string& padding = " ")
    -> std::string
{
  if (text.length() >= width) {
    return text;
  }

  const size_t padding_length = width - text.length();

  switch (align) {
    case align_t::RIGHT: {
      return std::string(padding_length, ' ') + text;
    }
    case align_t::CENTER: {
      const size_t left_padding = padding_length / 2;
      const size_t right_padding = padding_length - left_padding;
      return std::string(left_padding, ' ') + text
          + std::string(right_padding, ' ');
    }
    case align_t::LEFT:
    default: {
      return text + std::string(padding_length, ' ');
    }
  }
}

/**
 * @class table
 * @brief 表格类/Table class
 *
 * 用于格式化打印表格/Used for formatted table printing
 * @code
 * cpp_toolbox::utils::table t;
 * t.set_headers({"Name", "Age"}).add_row("Alice", 18).add_row("Bob", 20);
 * std::cout << t;
 * @endcode
 */
class CPP_TOOLBOX_EXPORT table
{
private:
  std::vector<std::vector<std::string>> m_data;  ///< 表格数据/Table data
  std::vector<std::string> m_headers;  ///< 表头/Headers
  mutable std::vector<size_t> m_col_widths;  ///< 列宽/Column widths
  mutable std::vector<align_t> m_col_aligns;  ///< 列对齐方式/Column alignments
  print_style_t m_style;  ///< 打印风格/Print style

  /**
   * @brief 计算每列宽度/Calculate column widths
   */
  void calculate_col_widths() const
  {
    size_t num_columns = m_headers.size();
    m_col_widths.assign(num_columns, m_style.min_width);

    // 确保列对齐向量大小正确/Ensure column alignment vector size is correct
    if (m_col_aligns.size() < num_columns) {
      m_col_aligns.resize(num_columns, m_style.alignment);
    }

    // 检查表头宽度/Check header width
    for (size_t i = 0; i < m_headers.size(); ++i) {
      m_col_widths[i] = std::max(m_col_widths[i], m_headers[i].length());
    }

    // 检查数据宽度/Check data width
    for (const auto& row : m_data) {
      for (size_t i = 0; i < row.size() && i < m_col_widths.size(); ++i) {
        m_col_widths[i] = std::max(m_col_widths[i], row[i].length());
      }
    }
  }

  /**
   * @brief 打印水平边框/Print horizontal border
   * @param os 输出流/Output stream
   */
  void print_horizontal_border(std::ostream& os) const
  {
    if (!m_style.show_border) {
      return;
    }

    const std::string corner = m_style.use_colors
        ? color_handler_t::colorize(
              m_style.corner, m_style.border_color, color_t::DEFAULT)
        : m_style.corner;
    const std::string border_h = m_style.use_colors
        ? color_handler_t::colorize(
              m_style.border_h, m_style.border_color, color_t::DEFAULT)
        : m_style.border_h;

    os << corner;
    for (size_t width : m_col_widths) {
      os << std::string(width + 2 * m_style.padding.length(), border_h[0])
         << corner;
    }
    os << "\n";
  }

  /**
   * @brief 打印一行/Print a row
   * @param os 输出流/Output stream
   * @param row 行数据/Row data
   * @param is_header 是否为表头/Is header row
   */
  void print_row(std::ostream& os,
                 const std::vector<std::string>& row,
                 bool is_header = false) const
  {
    const std::string border_v = m_style.use_colors
        ? color_handler_t::colorize(
              m_style.border_v, m_style.border_color, color_t::DEFAULT)
        : m_style.border_v;

    if (m_style.show_border) {
      os << border_v;
    }

    for (size_t i = 0; i < m_headers.size(); ++i) {
      const std::string cell = (i < row.size()) ? row[i] : "";
      const std::string aligned_cell =
          align_text(cell, m_col_widths[i], m_col_aligns[i]);

      // 应用颜色/Apply color
      std::string colored_cell;
      if (m_style.use_colors) {
        if (is_header) {
          colored_cell = color_handler_t::colorize(
              aligned_cell, m_style.header_fg, m_style.header_bg);
        } else {
          colored_cell = color_handler_t::colorize(
              aligned_cell, m_style.data_fg, m_style.data_bg);
        }
      } else {
        colored_cell = aligned_cell;
      }

      os << m_style.padding << colored_cell << m_style.padding;

      if (m_style.show_border) {
        os << border_v;
      } else {
        os << " ";
      }
    }
    os << "\n";
  }

public:
  /**
   * @brief 构造函数/Constructor
   * @param style 打印风格/Print style
   */
  explicit table(print_style_t style = get_default_style())
      : m_style(std::move(style))
  {
  }

  /**
   * @brief 设置表头/Set table headers
   * @param hdrs 表头向量/Header vector
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.set_headers({"A", "B"});
   * @endcode
   */
  auto set_headers(std::vector<std::string> hdrs) -> table&
  {
    m_headers = std::move(hdrs);
    return *this;
  }

  /**
   * @brief 添加一行/Add a row
   * @param row 行数据/Row data
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.add_row({"1", "2"});
   * @endcode
   */
  auto add_row(std::vector<std::string> row) -> table&
  {
    m_data.push_back(std::move(row));
    return *this;
  }

  /**
   * @brief 添加一行（可变参数）/Add a row (variadic)
   * @tparam Args 参数类型/Parameter types
   * @param args 行数据/Row data
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.add_row("Tom", 18);
   * @endcode
   */
  template<typename... Args>
  auto add_row(Args&&... args) -> table&
  {
    std::vector<std::string> row;
    (row.push_back(to_string_value(std::forward<Args>(args))), ...);
    return add_row(std::move(row));
  }

  /**
   * @brief 设置特定列的对齐方式/Set alignment for a specific column
   * @param column_index 列索引/Column index
   * @param align 对齐方式/Alignment
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.set_column_align(1, align_t::RIGHT);
   * @endcode
   */
  auto set_column_align(size_t column_index, align_t align) -> table&
  {
    if (m_col_aligns.size() <= column_index) {
      m_col_aligns.resize(column_index + 1, m_style.alignment);
    }
    m_col_aligns[column_index] = align;
    return *this;
  }

  /**
   * @brief 设置所有列的对齐方式/Set alignment for all columns
   * @param align 对齐方式/Alignment
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.set_all_columns_align(align_t::CENTER);
   * @endcode
   */
  auto set_all_columns_align(align_t align) -> table&
  {
    m_col_aligns.clear();
    m_col_aligns.resize(m_headers.size(), align);
    return *this;
  }

  /**
   * @brief 修改样式/Set print style
   * @param new_style 新样式/New style
   * @return table& 当前对象引用/Reference to current object
   * @code
   * t.set_style(my_style);
   * @endcode
   */
  auto set_style(print_style_t new_style) -> table&
  {
    m_style = std::move(new_style);
    return *this;
  }

  /**
   * @brief 输出表格/Output table
   * @param os 输出流/Output stream
   * @param table 表格对象/Table object
   * @return std::ostream& 输出流引用/Reference to output stream
   * @code
   * std::cout << t;
   * @endcode
   */
  friend auto operator<<(std::ostream& os, const table& table) -> std::ostream&
  {
    // 确保有表头/Ensure header exists
    if (table.m_headers.empty()) {
      return os << "[Empty table]" << "\n";
    }

    // 计算列宽/Calculate column widths
    table.calculate_col_widths();

    // 打印表格/Print table
    table.print_horizontal_border(os);

    if (table.m_style.show_header && !table.m_headers.empty()) {
      table.print_row(os, table.m_headers, true);
      table.print_horizontal_border(os);
    }

    for (const auto& row : table.m_data) {
      table.print_row(os, row);
    }

    table.print_horizontal_border(os);

    return os;
  }
};

/**
 * @class container_printer_t
 * @brief 通用容器打印器基类/Generic container printer base class
 * @tparam Container 容器类型/Container type
 */
template<typename Container>
class CPP_TOOLBOX_EXPORT container_printer_t
{
private:
  const Container& m_container;  ///< 容器引用/Container reference
  std::string m_name;  ///< 容器名称/Container name
  print_style_t m_style;  ///< 打印风格/Print style

protected:
  /**
   * @brief 获取容器引用/Get container reference
   * @return const Container& 容器引用/Container reference
   */
  auto get_container() const -> const Container& { return m_container; }
  /**
   * @brief 获取名称/Get name
   * @return const std::string& 名称/Name
   */
  [[nodiscard]] auto get_name() const -> const std::string& { return m_name; }
  /**
   * @brief 获取打印风格/Get print style
   * @return const print_style_t& 打印风格/Print style
   */
  [[nodiscard]] auto get_style() const -> const print_style_t&
  {
    return m_style;
  }

  /**
   * @brief 获取容器大小的虚函数/Virtual function to get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] virtual auto get_size() const -> size_t = 0;

  /**
   * @brief 打印容器内容的虚函数/Virtual function to print container content
   * @param os 输出流/Output stream
   */
  virtual void print_content(std::ostream& os) const = 0;

public:
  /**
   * @brief 构造函数/Constructor
   * @param c 容器引用/Container reference
   * @param name 容器名称/Container name
   * @param style 打印风格/Print style
   */
  container_printer_t(const Container& c, std::string name, print_style_t style)
      : m_container(c)
      , m_name(std::move(name))
      , m_style(std::move(style))
  {
  }

  /**
   * @brief 析构函数/Destructor
   */
  virtual ~container_printer_t() = default;

  container_printer_t(const container_printer_t&) = delete;
  container_printer_t& operator=(const container_printer_t&) = delete;
  container_printer_t(container_printer_t&&) = delete;
  container_printer_t& operator=(container_printer_t&&) = delete;

  // 声明外部模板函数为友元 (注意 <>) / Declare external template function as
  // friend (note < >)
  template<typename C>  // C 代表容器类型 / C represents the container type
  friend auto operator<<(std::ostream& os,
                         const container_printer_t<C>& printer)
      -> std::ostream&;
};

// 在类外部定义 operator<< 模板函数 / Define the operator<< template function
// outside the class
/**
 * @brief 输出容器 / Output container
 * @tparam Container 容器类型 / Container type
 * @param os 输出流 / Output stream
 * @param printer 打印器 / Printer
 * @return std::ostream& 输出流引用 / Reference to output stream
 * @code
 * std::cout << print_vector(vec);
 * @endcode
 */
template<typename Container>
auto operator<<(std::ostream& os, const container_printer_t<Container>& printer)
    -> std::ostream&
{
  std::string name_colored = printer.get_style().use_colors
      ? color_handler_t::colorize(printer.get_name(),
                                  printer.get_style().header_fg,
                                  printer.get_style().header_bg)
      : printer.get_name();

  os << name_colored << " [" << printer.get_size() << "] = {" << "\n";
  printer.print_content(os);  // 调用虚函数 / Call virtual function
  os << "}" << "\n";
  return os;
}

/**
 * @class vector_printer_t
 * @brief Vector 打印器/Vector printer
 * @tparam T 元素类型/Element type
 * @code
 * std::vector<int> v{1,2,3};
 * std::cout << print_vector(v);
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT vector_printer_t
    : public container_printer_t<std::vector<T>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    for (size_t i = 0; i < this->get_container().size(); ++i) {
      const std::string index = "[" + std::to_string(i) + "]: ";
      const std::string value = to_string_value(this->get_container()[i]);

      const std::string colored_index = this->get_style().use_colors
          ? color_handler_t::colorize(
                index, this->get_style().header_fg, color_t::DEFAULT)
          : index;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_index << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
    }
  }

public:
  using container_printer_t<std::vector<T>>::container_printer_t;
};

/**
 * @class map_printer_t
 * @brief Map 打印器/Map printer
 * @tparam K 键类型/Key type
 * @tparam V 值类型/Value type
 * @code
 * std::map<std::string, int> m{{"a",1},{"b",2}};
 * std::cout << print_map(m);
 * @endcode
 */
template<typename K, typename V>
class CPP_TOOLBOX_EXPORT map_printer_t
    : public container_printer_t<std::map<K, V>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    size_t i = 0;
    for (const auto& [key, value] : this->get_container()) {
      const std::string key_str = to_string_value(key);
      const std::string value_str = to_string_value(value);
      const std::string arrow = " => ";

      const std::string colored_key = this->get_style().use_colors
          ? color_handler_t::colorize(
                key_str, this->get_style().header_fg, color_t::DEFAULT)
          : key_str;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value_str, this->get_style().data_fg, color_t::DEFAULT)
          : value_str;

      os << "  " << colored_key << arrow << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
      i++;
    }
  }

public:
  using container_printer_t<std::map<K, V>>::container_printer_t;
};

/**
 * @class unorderedmap_printer_t
 * @brief Unordered Map 打印器/Unordered map printer
 * @tparam K 键类型/Key type
 * @tparam V 值类型/Value type
 * @code
 * std::unordered_map<int, std::string> m{{1,"a"},{2,"b"}};
 * std::cout << print_unordered_map(m);
 * @endcode
 */
template<typename K, typename V>
class CPP_TOOLBOX_EXPORT unorderedmap_printer_t
    : public container_printer_t<std::unordered_map<K, V>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    size_t i = 0;
    for (const auto& [key, value] : this->get_container()) {
      const std::string key_str = to_string_value(key);
      const std::string value_str = to_string_value(value);
      const std::string arrow = " => ";

      const std::string colored_key = this->get_style().use_colors
          ? color_handler_t::colorize(
                key_str, this->get_style().header_fg, color_t::DEFAULT)
          : key_str;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value_str, this->get_style().data_fg, color_t::DEFAULT)
          : value_str;

      os << "  " << colored_key << arrow << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
      i++;
    }
  }

public:
  using container_printer_t<std::unordered_map<K, V>>::container_printer_t;
};

/**
 * @class set_printer_t
 * @brief Set 打印器/Set printer
 * @tparam T 元素类型/Element type
 * @code
 * std::set<int> s{1,2,3};
 * std::cout << print_set(s);
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT set_printer_t : public container_printer_t<std::set<T>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    size_t i = 0;
    for (const auto& item : this->get_container()) {
      const std::string value = to_string_value(item);

      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
      i++;
    }
  }

public:
  using container_printer_t<std::set<T>>::container_printer_t;
};

/**
 * @class unorderedset_printer_t
 * @brief Unordered Set 打印器/Unordered set printer
 * @tparam T 元素类型/Element type
 * @code
 * std::unordered_set<std::string> s{"a","b"};
 * std::cout << print_unordered_set(s);
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT unorderedset_printer_t
    : public container_printer_t<std::unordered_set<T>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    size_t i = 0;
    for (const auto& item : this->get_container()) {
      const std::string value = to_string_value(item);

      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
      i++;
    }
  }

public:
  using container_printer_t<std::unordered_set<T>>::container_printer_t;
};

/**
 * @class deque_printer_t
 * @brief Deque 打印器/Deque printer
 * @tparam T 元素类型/Element type
 * @code
 * std::deque<double> d{1.1,2.2};
 * std::cout << print_deque(d);
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT deque_printer_t
    : public container_printer_t<std::deque<T>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    for (size_t i = 0; i < this->get_container().size(); ++i) {
      const std::string index = "[" + std::to_string(i) + "]: ";
      const std::string value = to_string_value(this->get_container()[i]);

      const std::string colored_index = this->get_style().use_colors
          ? color_handler_t::colorize(
                index, this->get_style().header_fg, color_t::DEFAULT)
          : index;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_index << colored_value;
      if (i < this->get_container().size() - 1) {
        os << ",";
      }
      os << "\n";
    }
  }

public:
  using container_printer_t<std::deque<T>>::container_printer_t;
};

/**
 * @class list_printer_t
 * @brief List 打印器/List printer
 * @tparam T 元素类型/Element type
 * @code
 * std::list<int> l{1,2,3};
 * std::cout << print_list(l);
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT list_printer_t
    : public container_printer_t<std::list<T>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    size_t i = 0;
    const auto& container = this->get_container();  // Cache container reference
    for (const auto& item : container) {  // 使用 getter
      const std::string index = "[" + std::to_string(i) + "]: ";  // const
      const std::string value = to_string_value(item);  // const

      const std::string colored_index = this->get_style().use_colors
          ? color_handler_t::colorize(
                index, this->get_style().header_fg, color_t::DEFAULT)
          : index;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_index << colored_value;
      // 使用 std::next 获取下一个迭代器来检查是否是最后一个元素
      // Cast i + 1 to difference_type to avoid sign conversion warning
      using diff_t = typename std::list<T>::difference_type;
      if (std::next(container.begin(), static_cast<diff_t>(i + 1))
          != container.end())
      {
        os << ",";
      }
      os << "\n";
      i++;
    }
  }

public:
  using container_printer_t<std::list<T>>::container_printer_t;
};

/**
 * @class array_printer_t
 * @brief Array 打印器/Array printer
 * @tparam T 元素类型/Element type
 * @tparam N 数组大小/Array size
 * @code
 * std::array<int,3> arr{{1,2,3}};
 * std::cout << print_array(arr);
 * @endcode
 */
template<typename T, size_t N>
class CPP_TOOLBOX_EXPORT array_printer_t
    : public container_printer_t<std::array<T, N>>
{
protected:
  /**
   * @brief 获取容器大小/Get container size
   * @return size_t 容器大小/Container size
   */
  [[nodiscard]] auto get_size() const -> size_t override { return N; }

  /**
   * @brief 打印内容/Print content
   * @param os 输出流/Output stream
   */
  void print_content(std::ostream& os) const override
  {
    for (size_t i = 0; i < N; ++i) {
      const std::string index = "[" + std::to_string(i) + "]: ";
      const std::string value = to_string_value(this->get_container()[i]);

      const std::string colored_index = this->get_style().use_colors
          ? color_handler_t::colorize(
                index, this->get_style().header_fg, color_t::DEFAULT)
          : index;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_index << colored_value;
      if (i < N - 1) {
        os << ",";
      }
      os << "\n";
    }
  }

public:
  using container_printer_t<std::array<T, N>>::container_printer_t;
};

/**
 * @class queue_printer_t
 * @brief 队列打印器/Queue printer
 * @tparam T 队列元素类型/Element type of the queue
 * @code
 * // 示例1：打印 int 类型队列
 * std::queue<int> q;
 * q.push(10); q.push(20); q.push(30);
 * std::cout << queue_printer_t<int>(q, "MyQueue") << std::endl;
 *
 * // 示例2：自定义打印风格
 * print_style_t style;
 * style.use_colors = true;
 * std::queue<std::string> qs;
 * qs.push("hello"); qs.push("world");
 * std::cout << queue_printer_t<std::string>(qs, "StrQueue", style) <<
 * std::endl;
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT queue_printer_t
    : public container_printer_t<std::queue<T>>
{
protected:
  /**
   * @brief 获取队列大小/Get queue size
   * @return size_t 队列中元素数量/Number of elements in the queue
   */
  [[nodiscard]] auto get_size() const -> size_t override
  {
    return this->get_container().size();
  }

  /**
   * @brief 打印队列内容/Print queue content
   * @param os 输出流/Output stream
   *
   * @details
   * 依次打印队列中的每个元素，格式为 [索引]: 元素值，每行一个元素/Prints each
   * element in the queue in the format [index]: value, one per line.
   */
  void print_content(std::ostream& os) const override
  {
    // 由于 std::queue 不支持随机访问，这里需要复制一份队列进行遍历
    // Since std::queue does not support random access, we need to copy the
    // queue for traversal.
    std::queue<T> q_copy = this->get_container();
    size_t i = 0;
    const size_t total_size = q_copy.size();  // 先获取总大小
    while (!q_copy.empty()) {
      const std::string index = "[" + std::to_string(i) + "]: ";
      const std::string value =
          to_string_value(q_copy.front());  // 获取队首元素

      const std::string colored_index = this->get_style().use_colors
          ? color_handler_t::colorize(
                index, this->get_style().header_fg, color_t::DEFAULT)
          : index;
      const std::string colored_value = this->get_style().use_colors
          ? color_handler_t::colorize(
                value, this->get_style().data_fg, color_t::DEFAULT)
          : value;

      os << "  " << colored_index << colored_value;
      if (i < total_size - 1) {  // 使用总大小判断是否需要逗号
        os << ",";
      }
      os << "\n";
      q_copy.pop();  // 弹出队首元素
      ++i;
    }
  }

public:
  // 继承基类构造函数/Inherit base class constructors
  using container_printer_t<std::queue<T>>::container_printer_t;
};

/**
 * @brief 打印vector的工厂函数/Factory function to print vector
 * @tparam T 元素类型/Element type
 * @param vec vector对象/Vector object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return vector_printer_t<T> 打印器/Printer
 * @code
 * std::vector<int> v{1,2,3};
 * std::cout << print_vector(v, "MyVec");
 * @endcode
 */
template<typename T>
auto print_vector(const std::vector<T>& vec,
                  const std::string& name = "Vector",
                  const print_style_t& style = get_default_style())
    -> vector_printer_t<T>
{
  return vector_printer_t<T>(vec, name, style);
}

/**
 * @brief 打印map的工厂函数/Factory function to print map
 * @tparam K 键类型/Key type
 * @tparam V 值类型/Value type
 * @param m map对象/Map object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return map_printer_t<K,V> 打印器/Printer
 * @code
 * std::map<int,int> m{{1,2}};
 * std::cout << print_map(m);
 * @endcode
 */
template<typename K, typename V>
auto print_map(const std::map<K, V>& m,
               const std::string& name = "Map",
               const print_style_t& style = get_default_style())
    -> map_printer_t<K, V>
{
  return map_printer_t<K, V>(m, name, style);
}

/**
 * @brief 打印unordered_map的工厂函数/Factory function to print unordered_map
 * @tparam K 键类型/Key type
 * @tparam V 值类型/Value type
 * @param m unordered_map对象/Unordered map object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return unorderedmap_printer_t<K,V> 打印器/Printer
 * @code
 * std::unordered_map<int,int> m{{1,2}};
 * std::cout << print_unordered_map(m);
 * @endcode
 */
template<typename K, typename V>
auto print_unordered_map(const std::unordered_map<K, V>& m,
                         const std::string& name = "Unordered Map",
                         const print_style_t& style = get_default_style())
    -> unorderedmap_printer_t<K, V>
{
  return unorderedmap_printer_t<K, V>(m, name, style);
}

/**
 * @brief 打印set的工厂函数/Factory function to print set
 * @tparam T 元素类型/Element type
 * @param s set对象/Set object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return set_printer_t<T> 打印器/Printer
 * @code
 * std::set<int> s{1,2};
 * std::cout << print_set(s);
 * @endcode
 */
template<typename T>
auto print_set(const std::set<T>& s,
               const std::string& name = "Set",
               const print_style_t& style = get_default_style())
    -> set_printer_t<T>
{
  return set_printer_t<T>(s, name, style);
}

/**
 * @brief 打印unordered_set的工厂函数/Factory function to print unordered_set
 * @tparam T 元素类型/Element type
 * @param s unordered_set对象/Unordered set object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return unorderedset_printer_t<T> 打印器/Printer
 * @code
 * std::unordered_set<int> s{1,2};
 * std::cout << print_unordered_set(s);
 * @endcode
 */
template<typename T>
auto print_unordered_set(const std::unordered_set<T>& s,
                         const std::string& name = "Unordered Set",
                         const print_style_t& style = get_default_style())
    -> unorderedset_printer_t<T>
{
  return unorderedset_printer_t<T>(s, name, style);
}

/**
 * @brief 打印deque的工厂函数/Factory function to print deque
 * @tparam T 元素类型/Element type
 * @param d deque对象/Deque object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return deque_printer_t<T> 打印器/Printer
 * @code
 * std::deque<int> d{1,2};
 * std::cout << print_deque(d);
 * @endcode
 */
template<typename T>
auto print_deque(const std::deque<T>& d,
                 const std::string& name = "Deque",
                 const print_style_t& style = get_default_style())
    -> deque_printer_t<T>
{
  return deque_printer_t<T>(d, name, style);
}

/**
 * @brief 打印list的工厂函数/Factory function to print list
 * @tparam T 元素类型/Element type
 * @param l list对象/List object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return list_printer_t<T> 打印器/Printer
 * @code
 * std::list<int> l{1,2};
 * std::cout << print_list(l);
 * @endcode
 */
template<typename T>
auto print_list(const std::list<T>& l,
                const std::string& name = "List",
                const print_style_t& style = get_default_style())
    -> list_printer_t<T>
{
  return list_printer_t<T>(l, name, style);
}

/**
 * @brief 打印array的工厂函数/Factory function to print array
 * @tparam T 元素类型/Element type
 * @tparam N 数组大小/Array size
 * @param arr array对象/Array object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return array_printer_t<T,N> 打印器/Printer
 * @code
 * std::array<int,2> arr{{1,2}};
 * std::cout << print_array(arr);
 * @endcode
 */
template<typename T, size_t N>
auto print_array(const std::array<T, N>& arr,
                 const std::string& name = "Array",
                 const print_style_t& style = get_default_style())
    -> array_printer_t<T, N>
{
  return array_printer_t<T, N>(arr, name, style);
}

/**
 * @brief 打印queue的工厂函数/Factory function to print queue
 * @tparam T 队列元素类型/Element type of the queue
 * @param q queue对象/Queue object
 * @param name 名称/Name
 * @param style 打印风格/Print style
 * @return queue_printer_t<T> 打印器/Printer
 * @code
 * std::queue<int> q;
 * q.push(10); q.push(20); q.push(30);
 * std::cout << print_queue(q, "MyQueue");
 * @endcode
 */
template<typename T>
auto print_queue(const std::queue<T>& q,
                 const std::string& name = "Queue",
                 const print_style_t& style = get_default_style())
    -> queue_printer_t<T>
{
  return queue_printer_t<T>(q, name, style);
}

}  // namespace cpp_toolbox::utils

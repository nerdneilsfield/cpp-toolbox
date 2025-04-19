#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
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
#include "cpp-toolbox/type_traits.hpp"
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace toolbox::utils
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
 * const auto& style = toolbox::utils::get_default_style();
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
  // 专门处理 std::string 本身
  if constexpr (std::is_same_v<T, std::string>) {
    return value;
  }
  // 如果是可迭代容器（且不是 string），就把它当作列表打印
  else if constexpr (toolbox::traits::is_iterable_v<T>)
  {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (auto const& elem : value) {
      if (!first) {
        oss << ", ";
      }
      oss << to_string_value(elem);  // 递归；如果 elem 也是容器会继续走这里
      first = false;
    }
    oss << "]";
    return oss.str();
  }
  // 否则尝试用 operator<< 打印
  else
  {
    std::ostringstream oss;
    oss << value;  // 只有当 stream << T 可用才会编译
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
 * @class table_t
 * @brief 格式化打印表格工具/Formatted table printing utility
 *
 * @details 支持以下高级特性/Supports the following advanced features:
 * - 自动列宽/Auto column width
 * - 最小/最大宽度约束/Min/max width constraints
 * - 内容换行或截断/Content wrapping and truncation
 * - 单元格合并(水平/垂直)/Cell merging (horizontal/vertical)
 * - 斑马纹(隔行变色)/Zebra striping (alternating row colors)
 * - 条件高亮/Conditional highlighting
 * - 文件/终端颜色输出切换/File/terminal color output switching
 *
 * @code
 * table_t tbl;
 * tbl.set_headers({"Name", "Age", "Role"});
 * tbl.add_row("Alice", 25, "Engineer");
 * tbl.add_row("Bob", 30, "Manager");
 * std::cout << tbl;
 * // Output:
 * // +--------+-----+----------+
 * // | Name   | Age | Role     |
 * // +--------+-----+----------+
 * // | Alice  | 25  | Engineer |
 * // | Bob    | 30  | Manager  |
 * // +--------+-----+----------+
 * @endcode
 */
class CPP_TOOLBOX_EXPORT table_t;

CPP_TOOLBOX_EXPORT std::ostream& operator<<(std::ostream& os,
                                            const table_t& tbl);

class CPP_TOOLBOX_EXPORT table_t
{
public:
  /** @brief 构造函数 / Constructor */
  explicit table_t(const print_style_t& style = get_default_style());

  /**
   * @brief 设置表头 / Set table_t headers
   * @param hdrs 表头字符串向量 / Vector of header strings
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_headers(const std::vector<std::string>& hdrs);

  /**
   * @brief 添加一行数据 / Add a data row
   * @param row 字符串向量，每个元素对应一列 / Vector of strings, each for a
   * column
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& add_row(const std::vector<std::string>& row);

  /**
   * @brief 添加一行数据（可变参数）/ Add a row with variadic arguments
   * @tparam Args 参数类型/Parameter types
   * @param args 行数据，每个参数对应一列的值 / Row data, each argument
   * corresponds to a column value
   * @return table_t& 当前对象的引用 / Reference to this table_t
   * @code
   * t.add_row("Alice", 30, "Engineer");
   * @endcode
   */
  template<typename... Args,
           // SFINAE: Disable this template if called with a single
           // std::vector<std::string> to ensure the non-template overload
           // add_row(const std::vector<std::string>&) is chosen instead.
           std::enable_if_t<
               !(sizeof...(Args) == 1
                 && std::is_same_v<
                     std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>,
                     std::vector<std::string>>),
               int> = 0>
  auto add_row(Args&&... args) -> table_t&
  {
    // No need for static_assert check anymore due to SFINAE constraint above.

    std::vector<std::string> row_data;
    row_data.reserve(sizeof...(args));
    // Convert all arguments to string and collect them
    (row_data.push_back(to_string_value(std::forward<Args>(args))), ...);

    // Call the non-template add_row(const std::vector<std::string>&) overload.
    // Passing 'row_data' as an lvalue ensures the correct overload is selected.
    return this->add_row(row_data);
  }

  /**
   * @brief 设置指定列的对齐方式 / Set alignment for a specific column
   * @param column_index 列索引 / Column index
   * @param align 对齐方式 / Alignment
   * @return table& 当前对象的引用 / Reference to this table
   */
  table_t& set_column_align(size_t column_index, align_t align);

  /**
   * @brief 设置所有列的对齐方式 / Set alignment for all columns
   * @param align 对齐方式 / Alignment
   * @return table& 当前对象的引用 / Reference to this table
   */
  table_t& set_all_columns_align(align_t align);

  /**
   * @brief 为指定列设置固定宽度 / Set fixed width for a column
   * @param col 列索引，从0开始 / Column index (0-based)
   * @param width 固定字符宽度 / Fixed width in characters
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_column_width(size_t col, size_t width);

  /**
   * @brief 为指定列设置最小和最大宽度约束 / Set min and max width constraints
   * for a column
   * @param col 列索引，从0开始 / Column index (0-based)
   * @param min_width 最小宽度 / Minimum width
   * @param max_width 最大宽度 / Maximum width
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_column_min_max(size_t col, size_t min_width, size_t max_width);

  /**
   * @brief 为指定行设置固定行高 / Set fixed height for a row
   * @param row 行索引，从0开始 / Row index (0-based)
   * @param height 固定行数 / Fixed number of printed lines
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_row_height(size_t row, size_t height);

  /**
   * @brief 启用或禁用自动内容换行 / Enable or disable automatic content
   * wrapping
   * @param enable true: 启用换行, false: 截断 / true to wrap, false to truncate
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& enable_wrap(bool enable);

  /**
   * @brief 设置截断时的省略符 / Set ellipsis string when truncating
   * @param ellipsis 省略符文本 / Ellipsis text (e.g. "...")
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_wrap_ellipsis(const std::string& ellipsis);

  /**
   * @brief 合并单元格 / Merge cells
   * @param start_row 起始行索引 / Starting row index
   * @param start_col 起始列索引 / Starting column index
   * @param row_span 垂直合并跨度 / Number of rows to span vertically
   * @param col_span 水平合并跨度 / Number of columns to span horizontally
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& span_cells(size_t start_row,
                      size_t start_col,
                      size_t row_span,
                      size_t col_span);

  /**
   * @brief 启用或禁用斑马纹 / Enable or disable zebra striping
   * @param enable true: 启用斑马纹, false: 禁用 / true to enable zebra
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& enable_zebra(bool enable);

  /**
   * @brief 设置斑马纹的奇偶行背景色 / Set zebra background colors for odd/even
   * rows
   * @param odd_bg 奇数行背景色 / Background color for odd rows
   * @param even_bg 偶数行背景色 / Background color for even rows
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_zebra_colors(color_t odd_bg, color_t even_bg);

  /**
   * @brief 设置条件高亮回调 / Set conditional highlight callback
   * @param cb 回调函数：接收(row, col, cell_value)并返回print_style_t /
   * Callback receiving (row, col, value) returning style
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_highlight_callback(
      std::function<print_style_t(size_t, size_t, const std::string&)> cb);

  /**
   * @brief 控制终端输出时是否保留颜色码 / Control whether to keep ANSI colors
   * for terminal output
   * @param enable true: 保留颜色, false: 不保留 / true to keep colors, false to
   * strip
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_output_color(bool enable);

  /**
   * @brief 控制写入文件时是否保留颜色码 / Control whether to keep ANSI colors
   * for file output
   * @param enable true: 保留颜色, false: 不保留 / true to keep colors, false to
   * strip
   * @return table_t& 当前对象的引用 / Reference to this table_t
   */
  table_t& set_file_output_color(bool enable);

  /**
   * @brief 打印表格 / Stream operator for printing table_t
   * @param os 输出流 / Output stream
   * @param tbl 要打印的table_t对象 / Table object to print
   * @return std::ostream& 输出流引用 / Reference to output stream
   */
  friend CPP_TOOLBOX_EXPORT std::ostream& operator<<(std::ostream& os,
                                                     const table_t& tbl);

private:
  /** @brief 表头 / Table headers */
  std::vector<std::string> m_headers;

  /** @brief 数据行 / Data rows */
  std::vector<std::vector<std::string>> m_data;

  /** @brief 打印风格 / Base print style */
  print_style_t m_style;

  /** @brief 列固定宽度映射 (col -> width) / Map of column to fixed width */
  std::unordered_map<size_t, size_t> m_col_fixed_width;

  /** @brief 列最小/最大宽度映射 (col -> {min, max}) / Map of column to
   * (min,max) widths */
  std::unordered_map<size_t, std::pair<size_t, size_t>> m_col_min_max;

  /** @brief 行固定高度映射 (row -> height) / Map of row to fixed height */
  std::unordered_map<size_t, size_t> m_row_fixed_height;

  /** @brief 每列对齐方式 / Column alignments */
  std::vector<align_t> m_col_aligns;

  /** @brief 是否启用换行 / Wrap enabled flag */
  bool m_wrap_enabled;

  /** @brief 换行截断省略符 / Ellipsis for truncation */
  std::string m_wrap_ellipsis;

  /**
   * @brief 合并单元格记录结构 / Structure for merged cell spans
   * row, col为起始单元格索引；row_span, col_span分别为垂直和水平跨度
   */
  struct Span
  {
    size_t row, col, row_span, col_span;
  };
  std::vector<Span> m_spans;

  /** @brief 是否启用斑马纹 / Zebra striping enabled flag */
  bool m_zebra_enabled;

  /** @brief 奇数/偶数行背景色 / Background colors for odd/even rows */
  color_t m_zebra_odd_bg;
  color_t m_zebra_even_bg;

  /** @brief 条件高亮回调 / Conditional highlight callback */
  std::function<print_style_t(size_t, size_t, const std::string&)>
      m_highlight_cb;

  /** @brief 终端输出时是否保留颜色 / Keep ANSI colors when printing to terminal
   */
  bool m_out_color;

  /** @brief 文件输出时是否保留颜色 / Keep ANSI colors when writing to file */
  bool m_file_color;

  /**
   * @brief 计算各列宽度 / Calculate column widths considering fixed/min/max
   * @remarks 会更新m_col_widths向量 / Updates m_col_widths vector
   */
  void calculate_col_widths() const;

  /**
   * @brief 打印水平边框 / Print horizontal border line
   * @param os 输出流 / Output stream
   */
  void print_horizontal_border(std::ostream& os) const;

  /**
   * @brief 打印单行（支持换行/截断及合并） / Print a logical row with
   * wrap/truncate and spans
   * @param os 输出流 / Output stream
   * @param row_data 行数据 / Row data vector
   * @param is_header 是否为表头行 / True if header row
   * @param row_index 行索引 / Row index for zebra/highlight
   */
  void print_wrapped_row(std::ostream& os,
                         const std::vector<std::string>& row_data,
                         bool is_header,
                         size_t row_index) const;

  /**
   * @brief 检查指定单元格是否被合并跨度覆盖 / Check if cell is covered by a
   * span
   * @param row 行索引 / Row index
   * @param col 列索引 / Column index
   * @return true: 被覆盖，需要跳过打印 / true if should skip printing
   */
  bool is_spanned_cell(size_t row, size_t col) const;

  /** @brief 已计算的列宽度 / Computed column widths */
  mutable std::vector<size_t> m_col_widths;
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

  os << name_colored << " [" << printer.get_size() << "] = {"
     << "\n";
  printer.print_content(os);  // 调用虚函数 / Call virtual function
  os << "}"
     << "\n";
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

}  // namespace toolbox::utils

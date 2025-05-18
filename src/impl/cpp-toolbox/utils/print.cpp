#include <iomanip>  // std::setw
#include <iostream>  // std::ostream
#include <sstream>

#include "cpp-toolbox/utils/print.hpp"

namespace toolbox::utils
{

/**
 * @brief 按空白拆分单词 / Split text into words by whitespace
 * @param text 输入文本 / Input text
 * @return std::vector<std::string> 单词列表 / Vector of words
 */
static std::vector<std::string> split_words(const std::string& text)
{
  std::istringstream iss(text);
  return {std::istream_iterator<std::string>(iss),
          std::istream_iterator<std::string>()};
}

/**
 * @brief 按列宽换行并截断文本 / Wrap and truncate text by width
 * @param text 输入文本 / Input text
 * @param width 最大字符宽度 / Maximum characters per line
 * @param wrap_enabled 是否启用换行 / true to enable wrapping, false to truncate
 * @param ellipsis 截断省略符 / Ellipsis text (e.g. "...")
 * @return std::vector<std::string> 换行或截断后的文本行 / Wrapped or truncated
 * lines
 */
static std::vector<std::string> wrap_and_truncate(const std::string& text,
                                                  size_t width,
                                                  bool wrap_enabled,
                                                  const std::string& ellipsis)
{
  std::vector<std::string> lines;
  if (width == 0) {
    lines.emplace_back("");
    return lines;
  }
  if (!wrap_enabled) {
    if (text.size() <= width) {
      lines.emplace_back(text);
    } else {
      std::string s = text.substr(0, width - ellipsis.size()) + ellipsis;
      lines.emplace_back(s);
    }
    return lines;
  }
  auto words = split_words(text);
  std::string current;
  for (const auto& w : words) {
    if (current.empty()) {
      if (w.size() <= width) {
        current = w;
      } else {
        // 单词超长，截断并添加省略号 / Truncate long word
        current = w.substr(0, width - ellipsis.size()) + ellipsis;
        lines.emplace_back(current);
        current.clear();
      }
    } else {
      if (current.size() + 1 + w.size() <= width) {
        current += ' ' + w;
      } else {
        lines.emplace_back(current);
        if (w.size() <= width) {
          current = w;
        } else {
          current = w.substr(0, width - ellipsis.size()) + ellipsis;
          lines.emplace_back(current);
          current.clear();
        }
      }
    }
  }
  if (!current.empty()) {
    lines.emplace_back(current);
  }
  if (lines.empty()) {
    lines.emplace_back("");
  }
  return lines;
}

/** @brief 构造函数 / Constructor */
table_t::table_t(const print_style_t& style)
    : m_style(style)
    , m_wrap_enabled(false)
    , m_wrap_ellipsis("...")
    , m_zebra_enabled(false)
    , m_zebra_odd_bg(color_t::DEFAULT)
    , m_zebra_even_bg(color_t::DEFAULT)
    , m_highlight_cb(nullptr)
    , m_out_color(true)
    , m_file_color(false)
    , m_title()
    , m_footer()
{
}

/** @brief 设置表头 / Set table headers */
table_t& table_t::set_headers(const std::vector<std::string>& hdrs)
{
  m_headers = hdrs;  // 复制表头 / Copy headers
  return *this;
}

/** @brief 添加一行数据 / Add a data row */
table_t& table_t::add_row(const std::vector<std::string>& row)
{
  m_data.push_back(row);
  return *this;
}

/** @brief 设置表格标题 / Set table title */
table_t& table_t::set_title(const std::string& title)
{
  m_title = title;
  return *this;
}

/** @brief 设置表格尾部文本 / Set table footer */
table_t& table_t::set_footer(const std::string& footer)
{
  m_footer = footer;
  return *this;
}

/** @brief 设置列固定宽度 / Set fixed column width */
table_t& table_t::set_column_width(size_t col, size_t width)
{
  m_col_fixed_width[col] = width;
  return *this;
}

/** @brief 设置列最小最大宽度 / Set column min/max widths */
table_t& table_t::set_column_min_max(size_t col, size_t min_w, size_t max_w)
{
  m_col_min_max[col] = {min_w, max_w};
  return *this;
}

/** @brief 设置行固定高度 / Set fixed row height */
table_t& table_t::set_row_height(size_t row, size_t height)
{
  m_row_fixed_height[row] = height;
  return *this;
}

/** @brief 启用/禁用换行 / Enable or disable wrapping */
table_t& table_t::enable_wrap(bool enable)
{
  m_wrap_enabled = enable;
  return *this;
}

/** @brief 设置省略符 / Set truncation ellipsis */
table_t& table_t::set_wrap_ellipsis(const std::string& ell)
{
  m_wrap_ellipsis = ell;
  return *this;
}

/** @brief 合并单元格 / Merge cells */
table_t& table_t::span_cells(size_t r, size_t c, size_t rs, size_t cs)
{
  m_spans.push_back({r, c, rs, cs});
  return *this;
}

/** @brief 启用/禁用斑马纹 / Enable or disable zebra striping */
table_t& table_t::enable_zebra(bool enable)
{
  m_zebra_enabled = enable;
  return *this;
}

/** @brief 设置斑马纹颜色 / Set zebra colors */
table_t& table_t::set_zebra_colors(color_t odd_bg, color_t even_bg)
{
  m_zebra_odd_bg = odd_bg;
  m_zebra_even_bg = even_bg;
  return *this;
}

/** @brief 设置条件高亮回调 / Set highlight callback */
table_t& table_t::set_highlight_callback(
    std::function<print_style_t(size_t, size_t, const std::string&)> cb)
{
  m_highlight_cb = std::move(cb);
  return *this;
}

/** @brief 控制终端颜色输出 / Set terminal color output */
table_t& table_t::set_output_color(bool enable)
{
  m_out_color = enable;
  return *this;
}

/** @brief 控制文件颜色输出 / Set file color output */
table_t& table_t::set_file_output_color(bool enable)
{
  m_file_color = enable;
  return *this;
}

/** @brief 检查单元格是否被span覆盖 / Check if cell is covered by a span */
bool table_t::is_spanned_cell(size_t row, size_t col) const
{
  for (const auto& span : m_spans) {
    if (row >= span.row && row < span.row + span.row_span && col >= span.col
        && col < span.col + span.col_span)
    {
      // 仅起始单元格打印，其他跳过
      return row != span.row || col != span.col;
    }
  }
  return false;
}

/**
 * @brief 计算列宽 / Calculate column widths
 *
 * 根据表头、数据以及用户设置的固定/最小/最大宽度计算最终列宽
 */
void table_t::calculate_col_widths() const
{
  size_t cols = m_headers.size();
  m_col_widths.assign(cols, 0);
  // 初始化宽度为表头长度
  for (size_t i = 0; i < cols; ++i) {
    m_col_widths[i] = std::max(m_col_widths[i], m_headers[i].length());
  }
  // 扫描数据行
  for (const auto& row : m_data) {
    for (size_t i = 0; i < cols && i < row.size(); ++i) {
      m_col_widths[i] = std::max(m_col_widths[i], row[i].length());
    }
  }
  // 应用用户约束
  for (const auto& [col, w] : m_col_fixed_width) {
    if (col < cols) {
      m_col_widths[col] = w;
    }
  }
  for (const auto& [col, mm] : m_col_min_max) {
    if (col < cols) {
      auto& w = m_col_widths[col];
      w = std::max(w, mm.first);
      w = std::min(w, mm.second);
    }
  }
}

/** @brief 打印水平边框 / Print horizontal border */
void table_t::print_horizontal_border(std::ostream& os, table_t::border_pos_t pos) const
{
  if (!m_style.show_border) {
    return;
  }

  auto left = m_style.box.top_left;
  auto join = m_style.box.center;
  auto right = m_style.box.top_right;

  switch (pos) {
    case table_t::border_pos_t::TOP:
      left = m_style.box.top_left;
      join = m_style.box.top_joint;
      right = m_style.box.top_right;
      break;
    case table_t::border_pos_t::MIDDLE:
      left = m_style.box.left_joint;
      join = m_style.box.center;
      right = m_style.box.right_joint;
      break;
    case table_t::border_pos_t::BOTTOM:
      left = m_style.box.bottom_left;
      join = m_style.box.bottom_joint;
      right = m_style.box.bottom_right;
      break;
  }

  auto border_h = m_style.border_h;
  if (m_style.use_colors && m_out_color) {
    left = color_handler_t::colorize(left, m_style.border_color, color_t::DEFAULT);
    join = color_handler_t::colorize(join, m_style.border_color, color_t::DEFAULT);
    right = color_handler_t::colorize(right, m_style.border_color, color_t::DEFAULT);
    border_h = color_handler_t::colorize(border_h, m_style.border_color, color_t::DEFAULT);
  }

  os << left;
  for (size_t i = 0; i < m_col_widths.size(); ++i) {
    for (size_t j = 0; j < m_col_widths[i] + (2 * m_style.padding.length()); ++j) {
      os << border_h;
    }
    os << (i == m_col_widths.size() - 1 ? right : join);
  }
  os << "\n";
}

/**
 * @brief 打印行（含换行、截断、合并） / Print row with wrap/truncate and spans
 * @param os 输出流 / Output stream
 * @param row_data 行数据 / Row data vector
 * @param is_header 是否表头 / True if header row
 * @param row_index 逻辑行索引 / Logical row index (including header)
 */
void table_t::print_wrapped_row(std::ostream& os,
                                const std::vector<std::string>& row_data,
                                bool is_header,
                                size_t row_index) const
{
  size_t cols = m_headers.size();
  // 生成每个单元格的换行/截断内容 / Generate wrapped/truncated lines per cell
  std::vector<std::vector<std::string>> cell_lines(cols);
  // 标记跨列合并范围 / Process spans
  for (size_t c = 0; c < cols; ++c) {
    if (is_spanned_cell(row_index, c))
      continue;
    // 检查是否为span起始 / Find span start
    size_t span_cols = 1;
    for (auto& span : m_spans) {
      if (span.row == row_index && span.col == c) {
        span_cols = span.col_span;
        break;
      }
    }
    // 计算文本可用宽度 / Compute available text width
    size_t text_width = 0;
    for (size_t j = c; j < c + span_cols && j < cols; ++j) {
      text_width += m_col_widths[j];
    }
    // 获取单元格原始文本 / Original cell text
    std::string cell = (c < row_data.size() ? row_data[c] : "");
    // 执行换行/截断 / Wrap or truncate
    cell_lines[c] =
        wrap_and_truncate(cell, text_width, m_wrap_enabled, m_wrap_ellipsis);
    // 跳过合并列 / Skip merged columns
    c += span_cols - 1;
  }
  // 计算物理行高 / Determine physical row height
  size_t height = 0;
  for (auto& lines : cell_lines) {
    height = std::max(height, lines.size());
  }
  if (auto it = m_row_fixed_height.find(row_index);
      it != m_row_fixed_height.end())
  {
    height = it->second;
  }
  // 按子行打印 / Print each sub-row
  for (size_t sub = 0; sub < height; ++sub) {
    // 左边框 / Left border
    if (m_style.show_border) {
      auto v = m_style.border_v;
      if (m_style.use_colors && m_out_color) {
        v = color_handler_t::colorize(
            v, m_style.border_color, color_t::DEFAULT);
      }
      os << v;
    }
    // 遍历列 / Iterate columns
    for (size_t c = 0; c < cols; ++c) {
      if (is_spanned_cell(row_index, c))
        continue;
      // 查找span起始 / Find span start
      size_t span_cols = 1;
      for (auto& span : m_spans) {
        if (span.row == row_index && span.col == c) {
          span_cols = span.col_span;
          break;
        }
      }
      // 文本宽度 / Text width
      size_t text_width = 0;
      for (size_t j = c; j < c + span_cols && j < cols; ++j) {
        text_width += m_col_widths[j];
      }
      // 取对应子行文本 / Get line or empty
      const auto& lines = cell_lines[c];
      std::string line = (sub < lines.size() ? lines[sub] : "");

      // Determine alignment for this column
      align_t current_align = m_style.alignment;  // Default
      if (c < m_col_aligns.size()) {  // Check if m_col_aligns has an entry for
                                      // this column
        current_align = m_col_aligns[c];
      }

      // 对齐 / Align text
      std::string aligned =
          align_text(line, text_width, current_align);  // Use current_align

      // 应用条件高亮和斑马纹 / Apply highlight and zebra
      print_style_t cell_style = m_style;
      if (!is_header && m_highlight_cb) {
        cell_style = m_highlight_cb(row_index, c, line);
      }
      if (m_zebra_enabled && !is_header) {
        cell_style.data_bg =
            (row_index % 2 == 1 ? m_zebra_odd_bg : m_zebra_even_bg);
      }
      // 上色 / Colorize
      if (cell_style.use_colors && m_out_color) {
        aligned = color_handler_t::colorize(
            aligned, cell_style.data_fg, cell_style.data_bg);
      }
      // 填充并输出 / Print cell
      os << m_style.padding << aligned << m_style.padding;
      // 右边框 / Right border
      if (m_style.show_border) {
        auto v = m_style.border_v;
        if (m_style.use_colors && m_out_color) {
          v = color_handler_t::colorize(
              v, m_style.border_color, color_t::DEFAULT);
        }
        os << v;
      }
      // 跳过span列 / Skip merged cols
      c += span_cols - 1;
    }
    os << '\n';
  }
}

/**
 * @brief operator<< 重载 / Stream operator overload
 */
std::ostream& operator<<(std::ostream& os, const table_t& tbl)
{
  if (tbl.m_headers.empty()) {
    return os << "[Empty table]\n";
  }
  if (!tbl.m_title.empty()) {
    os << tbl.m_title << '\n';
  }
  tbl.calculate_col_widths();
  tbl.print_horizontal_border(os, table_t::border_pos_t::TOP);
  if (tbl.m_style.show_header) {
    tbl.print_wrapped_row(os, tbl.m_headers, /*is_header=*/true, 0);
    tbl.print_horizontal_border(os, table_t::border_pos_t::MIDDLE);
  }
  for (size_t i = 0; i < tbl.m_data.size(); ++i) {
    tbl.print_wrapped_row(os,
                          tbl.m_data[i],
                          /*is_header=*/false,
                          i + (tbl.m_style.show_header ? 1 : 0));
  }
  tbl.print_horizontal_border(os, table_t::border_pos_t::BOTTOM);
  if (!tbl.m_footer.empty()) {
    os << tbl.m_footer << '\n';
  }
  return os;
}

/**
 * @brief 将表格渲染为字符串 / Render table as a string
 */
auto table_t::to_string(bool with_color) const -> std::string
{
  table_t tmp = *this;
  tmp.m_out_color = with_color;
  tmp.m_style.use_colors = with_color;
  std::ostringstream oss;
  oss << tmp;
  return oss.str();
}

/** @brief 设置指定列的对齐方式 / Set alignment for a specific column */
table_t& table_t::set_column_align(size_t column_index, align_t align)
{
  if (m_col_aligns.size() <= column_index) {
    m_col_aligns.resize(column_index + 1, m_style.alignment);
  }
  m_col_aligns[column_index] = align;
  return *this;
}

/** @brief 设置所有列的对齐方式 / Set alignment for all columns */
table_t& table_t::set_all_columns_align(align_t align)
{
  m_col_aligns.assign(m_headers.size(), align);
  return *this;
}

}  // namespace toolbox::utils
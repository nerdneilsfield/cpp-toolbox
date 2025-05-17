#pragma once

#include <string>
#include <vector>

#include "cpp-toolbox/utils/print.hpp"

namespace toolbox::utils
{

class CPP_TOOLBOX_EXPORT plot_t
{
public:
  enum class axis_scale_t
  {
    LINEAR,
    LOG
  };

  enum class style_t : char
  {
    STAR = '*',
    DOT = '.',
    PLUS = '+',
    CROSS = 'x'
  };

  struct axis_t
  {
    axis_scale_t scale {axis_scale_t::LINEAR};
    std::string label;
  };

  struct series_t
  {
    std::vector<double> xs;
    std::vector<double> ys;
    color_t color {color_t::DEFAULT};
    bool line {true};
    style_t style {style_t::STAR};
  };

  /** 添加折线数据 / Add line series */
  void add_line_series(const std::vector<double>& xs,
                       const std::vector<double>& ys,
                       color_t color = color_t::DEFAULT,
                       style_t style = style_t::STAR);
  /** 添加散点数据 / Add scatter series */
  void add_scatter_series(const std::vector<double>& xs,
                          const std::vector<double>& ys,
                          color_t color = color_t::DEFAULT,
                          style_t style = style_t::DOT);

  /// 设置 X 轴 / Configure the X axis
  void set_x_axis(axis_scale_t scale = axis_scale_t::LINEAR,
                  std::string label = "");
  /// 设置 Y 轴 / Configure the Y axis
  void set_y_axis(axis_scale_t scale = axis_scale_t::LINEAR,
                  std::string label = "");
  /// 设置标题 / Set plot title
  void set_title(std::string title);

  /// 开启或关闭轴网格 / Toggle axis grid
  void enable_axis_grid(bool enable = true, color_t color = color_t::WHITE);
  /// 开启或关闭全局网格 / Toggle global grid
  void enable_global_grid(bool enable = true, color_t color = color_t::CYAN);

  /** 渲染为字符串 / Render as string */
  [[nodiscard]] std::string render(size_t width = 60, size_t height = 20) const;

private:
  std::vector<series_t> m_series;
  axis_t m_x_axis {};
  axis_t m_y_axis {};
  std::string m_title {};
  bool m_show_axis_grid {false};
  bool m_show_global_grid {false};
  color_t m_axis_grid_color {color_t::WHITE};
  color_t m_global_grid_color {color_t::CYAN};
};

}  // namespace toolbox::utils

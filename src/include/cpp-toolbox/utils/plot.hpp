#pragma once

#include <vector>
#include <string>

#include "cpp-toolbox/utils/print.hpp"

namespace toolbox::utils {

class CPP_TOOLBOX_EXPORT plot_t {
public:
  struct series_t {
    std::vector<double> xs;
    std::vector<double> ys;
    color_t color{color_t::DEFAULT};
    bool line{true};
    char symbol{'*'};
  };

  /** 添加折线数据 / Add line series */
  void add_line_series(const std::vector<double>& xs,
                       const std::vector<double>& ys,
                       color_t color = color_t::DEFAULT,
                       char symbol = '*');
  /** 添加散点数据 / Add scatter series */
  void add_scatter_series(const std::vector<double>& xs,
                          const std::vector<double>& ys,
                          color_t color = color_t::DEFAULT,
                          char symbol = 'o');

  /** 渲染为字符串 / Render as string */
  [[nodiscard]] std::string render(size_t width = 60, size_t height = 20) const;

private:
  std::vector<series_t> m_series;
};

} // namespace toolbox::utils

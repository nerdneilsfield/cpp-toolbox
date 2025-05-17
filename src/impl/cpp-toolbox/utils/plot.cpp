#include <algorithm>
#include <cmath>
#include <sstream>

#include "cpp-toolbox/utils/plot.hpp"

namespace toolbox::utils
{

void plot_t::add_line_series(const std::vector<double>& xs,
                             const std::vector<double>& ys,
                             color_t color,
                             style_t style)
{
  m_series.push_back(series_t {xs, ys, color, true, style});
}

void plot_t::add_scatter_series(const std::vector<double>& xs,
                                const std::vector<double>& ys,
                                color_t color,
                                style_t style)
{
  m_series.push_back(series_t {xs, ys, color, false, style});
}

void plot_t::set_x_axis(axis_scale_t scale, std::string label)
{
  m_x_axis.scale = scale;
  m_x_axis.label = std::move(label);
}

void plot_t::set_y_axis(axis_scale_t scale, std::string label)
{
  m_y_axis.scale = scale;
  m_y_axis.label = std::move(label);
}

void plot_t::set_title(std::string title)
{
  m_title = std::move(title);
}

void plot_t::enable_axis_grid(bool enable, color_t color)
{
  m_show_axis_grid = enable;
  m_axis_grid_color = color;
}

void plot_t::enable_global_grid(bool enable, color_t color)
{
  m_show_global_grid = enable;
  m_global_grid_color = color;
}

std::string plot_t::render(size_t width, size_t height) const
{
  if (m_series.empty() || width == 0 || height == 0)
    return "";

  double min_x = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::lowest();
  double min_y = min_x;
  double max_y = max_x;

  for (const auto& s : m_series) {
    for (double v : s.xs) {
      if (m_x_axis.scale == axis_scale_t::LOG) {
        if (v > 0) {
          v = std::log10(v);
        } else {
          continue;
        }
      }
      min_x = std::min(min_x, v);
      max_x = std::max(max_x, v);
    }
    for (double v : s.ys) {
      if (m_y_axis.scale == axis_scale_t::LOG) {
        if (v > 0) {
          v = std::log10(v);
        } else {
          continue;
        }
      }
      min_y = std::min(min_y, v);
      max_y = std::max(max_y, v);
    }
  }
  if (std::abs(max_x - min_x) < 1e-12) {
    min_x -= 1.0;
    max_x += 1.0;
  }
  if (std::abs(max_y - min_y) < 1e-12) {
    min_y -= 1.0;
    max_y += 1.0;
  }

  std::vector<std::string> grid(height, std::string(width, ' '));
  std::vector<std::vector<color_t>> colors(
      height, std::vector<color_t>(width, color_t::DEFAULT));

  auto map_x = [&](double x)
  {
    if (m_x_axis.scale == axis_scale_t::LOG && x > 0)
      x = std::log10(x);
    return static_cast<int>((x - min_x) / (max_x - min_x)
                            * (static_cast<double>(width - 1)));
  };
  auto map_y = [&](double y)
  {
    if (m_y_axis.scale == axis_scale_t::LOG && y > 0)
      y = std::log10(y);
    return static_cast<int>((y - min_y) / (max_y - min_y)
                            * (static_cast<double>(height - 1)));
  };

  for (const auto& s : m_series) {
    size_t n = std::min(s.xs.size(), s.ys.size());
    if (n == 0)
      continue;
    int prev_x = map_x(s.xs[0]);
    int prev_y = map_y(s.ys[0]);
    for (size_t i = 0; i < n; ++i) {
      int gx = map_x(s.xs[i]);
      int gy = map_y(s.ys[i]);
      size_t row = height - 1 - static_cast<size_t>(gy);
      size_t col = static_cast<size_t>(gx);
      char sym = static_cast<char>(s.style);
      if (row < height && col < width) {
        grid[row][col] = sym;
        colors[row][col] = s.color;
      }
      if (s.line && i > 0) {
        int x0 = prev_x;
        int y0 = prev_y;
        int x1 = gx;
        int y1 = gy;
        int dx = std::abs(x1 - x0);
        int dy = -std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
          size_t r = height - 1 - static_cast<size_t>(y0);
          size_t c = static_cast<size_t>(x0);
          if (r < height && c < width) {
            grid[r][c] = sym;
            colors[r][c] = s.color;
          }
          if (x0 == x1 && y0 == y1)
            break;
          int e2 = 2 * err;
          if (e2 >= dy) {
            err += dy;
            x0 += sx;
          }
          if (e2 <= dx) {
            err += dx;
            y0 += sy;
          }
        }
      }
      prev_x = gx;
      prev_y = gy;
    }
  }

  const size_t tick_count = 4;
  size_t step_x = width / (tick_count + 1);
  size_t step_y = height / (tick_count + 1);

  if (m_show_global_grid) {
    for (size_t i = 1; i <= tick_count; ++i) {
      size_t col = i * step_x;
      if (col >= width)
        break;
      for (size_t r = 0; r < height; ++r) {
        if (grid[r][col] == ' ')
          grid[r][col] = '|';
        colors[r][col] = m_global_grid_color;
      }
    }
    for (size_t i = 1; i <= tick_count; ++i) {
      size_t row = height - 1 - i * step_y;
      if (row >= height)
        break;
      for (size_t c = 0; c < width; ++c) {
        if (grid[row][c] == ' ')
          grid[row][c] = '-';
        colors[row][c] = m_global_grid_color;
      }
    }
  }

  if (m_show_axis_grid) {
    for (size_t c = 0; c < width; ++c) {
      if (grid[height - 1][c] == ' ')
        grid[height - 1][c] = '-';
      colors[height - 1][c] = m_axis_grid_color;
    }
    for (size_t r = 0; r < height; ++r) {
      if (grid[r][0] == ' ')
        grid[r][0] = '|';
      colors[r][0] = m_axis_grid_color;
    }
    for (size_t i = 1; i <= tick_count; ++i) {
      size_t col = i * step_x;
      if (col >= width)
        break;
      if (grid[height - 1][col] == ' ')
        grid[height - 1][col] = '+';
      colors[height - 1][col] = m_axis_grid_color;
    }
    for (size_t i = 1; i <= tick_count; ++i) {
      size_t row = height - 1 - i * step_y;
      if (row >= height)
        break;
      if (grid[row][0] == ' ')
        grid[row][0] = '+';
      colors[row][0] = m_axis_grid_color;
    }
  }

  std::ostringstream oss;
  if (!m_title.empty()) {
    oss << m_title << '\n';
  }
  for (size_t r = 0; r < height; ++r) {
    for (size_t c = 0; c < width; ++c) {
      std::string ch(1, grid[r][c]);
      if (colors[r][c] != color_t::DEFAULT) {
        oss << color_handler_t::colorize(ch, colors[r][c], color_t::DEFAULT);
      } else {
        oss << ch;
      }
    }
    if (r + 1 < height)
      oss << '\n';
  }
  oss << '\n';
  if (!m_x_axis.label.empty()) {
    oss << "x: " << m_x_axis.label;
    if (m_x_axis.scale == axis_scale_t::LOG)
      oss << " [log]";
    oss << '\n';
  }
  if (!m_y_axis.label.empty()) {
    oss << "y: " << m_y_axis.label;
    if (m_y_axis.scale == axis_scale_t::LOG)
      oss << " [log]";
    oss << '\n';
  }
  return oss.str();
}

}  // namespace toolbox::utils

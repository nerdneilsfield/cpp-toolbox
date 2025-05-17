#include "cpp-toolbox/utils/plot.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace toolbox::utils {

void plot_t::add_line_series(const std::vector<double>& xs,
                             const std::vector<double>& ys,
                             color_t color,
                             char symbol)
{
  m_series.push_back(series_t{xs, ys, color, true, symbol});
}

void plot_t::add_scatter_series(const std::vector<double>& xs,
                                const std::vector<double>& ys,
                                color_t color,
                                char symbol)
{
  m_series.push_back(series_t{xs, ys, color, false, symbol});
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
      min_x = std::min(min_x, v);
      max_x = std::max(max_x, v);
    }
    for (double v : s.ys) {
      min_y = std::min(min_y, v);
      max_y = std::max(max_y, v);
    }
  }
  if (min_x == max_x) {
    min_x -= 1.0;
    max_x += 1.0;
  }
  if (min_y == max_y) {
    min_y -= 1.0;
    max_y += 1.0;
  }

  std::vector<std::string> grid(height, std::string(width, ' '));
  std::vector<std::vector<color_t>> colors(height, std::vector<color_t>(width, color_t::DEFAULT));

  auto map_x = [&](double x) {
    return static_cast<int>((x - min_x) / (max_x - min_x) * (static_cast<double>(width - 1))); };
  auto map_y = [&](double y) {
    return static_cast<int>((y - min_y) / (max_y - min_y) * (static_cast<double>(height - 1))); };

  for (const auto& s : m_series) {
    size_t n = std::min(s.xs.size(), s.ys.size());
    if (n == 0) continue;
    int prev_x = map_x(s.xs[0]);
    int prev_y = map_y(s.ys[0]);
    for (size_t i = 0; i < n; ++i) {
      int gx = map_x(s.xs[i]);
      int gy = map_y(s.ys[i]);
      size_t row = height - 1 - static_cast<size_t>(gy);
      size_t col = static_cast<size_t>(gx);
      if (row < height && col < width) {
        grid[row][col] = s.symbol;
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
            grid[r][c] = s.symbol;
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

  std::ostringstream oss;
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
  return oss.str();
}

} // namespace toolbox::utils

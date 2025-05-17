#include <iostream>
#include <vector>

#include "cpp-toolbox/utils/plot.hpp"

int main() {
  using namespace toolbox::utils;

  plot_t plot;
  std::vector<double> xs1{0, 1, 2, 3, 4};
  std::vector<double> ys1{1, 3, 2, 4, 3};
  plot.add_line_series(xs1, ys1, color_t::GREEN, '*');

  std::vector<double> xs2{0.5, 1.5, 2.5, 3.5};
  std::vector<double> ys2{1.5, 2.0, 3.0, 2.5};
  plot.add_scatter_series(xs2, ys2, color_t::RED, 'o');

  std::cout << plot.render(40, 10) << "\n";
  return 0;
}

#include <array>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cpp-toolbox/utils/print.hpp"

int main()
{
  using namespace cpp_toolbox::utils;

  // --- 表格打印示例 (Table Printing Example) ---
  std::cout << "--- Table Example ---" << std::endl;
  table t;
  t.set_headers({"Name", "Age", "City"})
      .add_row("Alice", 30, "New York")
      .add_row("Bob", 24, "Los Angeles")
      .add_row("Charlie", 35, "Chicago");

  // 设置第二列（Age）右对齐 (Set second column (Age) to right align)
  t.set_column_align(1, align_t::RIGHT);

  std::cout << t << std::endl;

  // --- 容器打印示例 (Container Printing Example) ---
  std::cout << "--- Container Examples (Default Style) ---" << std::endl;

  // Vector
  std::vector<int> vec = {1, 2, 3, 4, 5};
  std::cout << print_vector(vec, "MyVector") << std::endl;

  // Map
  std::map<std::string, double> map_data = {{"pi", 3.14}, {"e", 2.71}};
  std::cout << print_map(map_data, "ConstantsMap") << std::endl;

  // Unordered Map
  std::unordered_map<int, std::string> umap_data = {{1, "one"}, {2, "two"}};
  std::cout << print_unordered_map(umap_data, "NumberNames") << std::endl;

  // Set
  std::set<char> set_data = {'c', 'a', 'b'};
  std::cout << print_set(set_data, "CharSet") << std::endl;

  // Unordered Set
  std::unordered_set<float> uset_data = {1.1f, 2.2f, 1.1f};
  std::cout << print_unordered_set(uset_data, "FloatSet") << std::endl;

  // Deque
  std::deque<bool> deque_data = {true, false, true};
  std::cout << print_deque(deque_data, "BoolDeque") << std::endl;

  // List
  std::list<std::string> list_data = {"apple", "banana", "cherry"};
  std::cout << print_list(list_data, "FruitList") << std::endl;

  // Array
  std::array<short, 4> array_data = {10, 20, 30, 40};
  std::cout << print_array(array_data, "ShortArray") << std::endl;

  // Queue
  std::queue<int> queue_data;
  queue_data.push(100);
  queue_data.push(200);
  queue_data.push(300);
  std::cout << print_queue(queue_data, "IntQueue") << std::endl;

  // --- 自定义样式示例 (Custom Style Example) ---
  std::cout << "--- Custom Style Examples ---" << std::endl;

  print_style_t custom_style;
  custom_style.border_h = "=";
  custom_style.border_v = "*";
  custom_style.corner = "#";
  custom_style.padding = "";  // 无填充 (No padding)
  custom_style.show_header = false;
  custom_style.alignment = align_t::CENTER;

  // 自定义样式的表格 (Table with custom style)
  table custom_table(custom_style);
  custom_table.set_headers({"X", "Y"}).add_row(1, 2).add_row(3, 4);
  std::cout << custom_table << std::endl;

  // 自定义样式的Vector (Vector with custom style and colors)
  print_style_t color_style;
  color_style.header_fg = color_t::YELLOW;
  color_style.header_bg = color_t::BLUE;
  color_style.data_fg = color_t::GREEN;
  color_style.border_color = color_t::RED;

  std::vector<std::string> color_vec = {"Red", "Green", "Blue"};
  std::cout << print_vector(color_vec, "ColorVector", color_style) << std::endl;

  // 禁用颜色的Map (Map with colors disabled)
  print_style_t no_color_style = color_style;
  no_color_style.use_colors = false;
  std::cout << print_map(map_data, "NoColorMap", no_color_style) << std::endl;

  return 0;
}

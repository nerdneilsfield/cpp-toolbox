#include <array>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cpp-toolbox/utils/print.hpp"

int main()
{
  using namespace toolbox::utils;

  // --- 示例 1: 基本表格与对齐 / Basic table and alignment ---
  std::cout << "--- Example 1: Basic Table and Alignment ---" << "\n";
  table_t t1;
  t1.set_headers({"Name", "Age", "City"})
      .add_row("Alice", 30, "New York")
      .add_row("Bob", 24, "Los Angeles")
      .add_row("Charlie", 35, "Chicago");
  // 第二列右对齐 / Right-align second column
  t1.set_column_align(1, align_t::RIGHT);
  std::cout << t1 << "\n";

  // --- 示例 2: 换行与截断 / Wrapping and Truncation ---
  std::cout << "--- Example 2: Wrapping and Truncation ---" << "\n";
  table_t t2;
  t2.set_headers({"Description", "Value"});
  t2.enable_wrap(true)
      .set_wrap_ellipsis("...")
      // 第0列宽度最多20，至少10 / Column 0 width between [10,20]
      .set_column_min_max(0, 10, 20);
  t2.add_row("This is a very long description that needs wrapping", 12345)
      .add_row("Short desc", 67890);
  std::cout << t2 << "\n";

  // --- 示例 3: 单元格合并 / Cell Spanning ---
  std::cout << "--- Example 3: Cell Spanning ---" << "\n";
  table_t t3;
  t3.set_headers({"Q1", "Q2", "Q3", "Q4"});
  // 合并第一行的Q1-Q3 / Span Q1 to Q3 horizontally
  t3.span_cells(0, 0, 1, 3);
  t3.add_row("Total", "10", "20", "30");
  t3.add_row("East", 5, 7, 8);
  std::cout << t3 << "\n";

  // --- 示例 4: 斑马纹与条件高亮 / Zebra Striping and Highlight ---
  std::cout << "--- Example 4: Zebra Striping and Highlight ---" << "\n";
  table_t t4;
  t4.set_headers({"Item", "Price"});
  t4.enable_zebra(true)
      .set_zebra_colors(color_t::DEFAULT, color_t::BLUE)
      .set_highlight_callback(
          [](size_t row, size_t col, const std::string& val)
          {
            print_style_t style = get_default_style();
            if (col == 1 && std::regex_match(val, std::regex("\\d+"))) {
              int price = std::stoi(val);
              if (price > 50)
                style.data_fg = color_t::RED;  // 高价标红 / High price red
            }
            return style;
          });
  t4.add_row("Pen", 5).add_row("Notebook", 60).add_row("Backpack", 45);
  std::cout << t4 << "\n";

  // --- 示例 5: 固定宽度与行高 / Fixed Width and Row Height ---
  std::cout << "--- Example 5: Fixed Width and Row Height ---" << "\n";
  table_t t5;
  t5.set_headers({"Col A", "Col B"})
      // 固定第0列宽度 8, 行高为 2 / Fixed width for col0 = 8, row height = 2
      .set_column_width(0, 8)
      .set_row_height(1, 2)
      .enable_wrap(true);
  t5.add_row("WrapTestExample", "Data1").add_row("AnotherLongText", "Data2");
  std::cout << t5 << "\n";

  // --- 示例 6: 写入文件，无颜色 / File output without ANSI colors ---
  std::cout << "--- Example 6: File Output (no colors) ---" << "\n";
  table_t t6 = t4;  // 复用之前的 t4
  t6.set_file_output_color(false);
  std::ofstream ofs("table_output.txt");
  ofs << t6;
  ofs.close();
  std::cout << "Written to table_output.txt" << "\n";

  // --- 容器打印示例 (Container Printing Example) ---
  std::cout << "--- Container Examples (Default Style) ---" << "\n";

  // Vector
  std::vector<int> vec = {1, 2, 3, 4, 5};
  std::cout << print_vector(vec, "MyVector") << "\n";

  // Map
  std::map<std::string, double> map_data = {{"pi", 3.14}, {"e", 2.71}};
  std::cout << print_map(map_data, "ConstantsMap") << "\n";

  // Unordered Map
  std::unordered_map<int, std::string> umap_data = {{1, "one"}, {2, "two"}};
  std::cout << print_unordered_map(umap_data, "NumberNames") << "\n";

  // Set
  std::set<char> set_data = {'c', 'a', 'b'};
  std::cout << print_set(set_data, "CharSet") << "\n";

  // Unordered Set
  std::unordered_set<float> uset_data = {1.1f, 2.2f, 1.1f};
  std::cout << print_unordered_set(uset_data, "FloatSet") << "\n";

  // Deque
  std::deque<bool> deque_data = {true, false, true};
  std::cout << print_deque(deque_data, "BoolDeque") << "\n";

  // List
  std::list<std::string> list_data = {"apple", "banana", "cherry"};
  std::cout << print_list(list_data, "FruitList") << "\n";

  // Array
  std::array<short, 4> array_data = {10, 20, 30, 40};
  std::cout << print_array(array_data, "ShortArray") << "\n";

  // Queue
  std::queue<int> queue_data;
  queue_data.push(100);
  queue_data.push(200);
  queue_data.push(300);
  std::cout << print_queue(queue_data, "IntQueue") << "\n";

  // --- 自定义样式示例 (Custom Style Example) ---
  std::cout << "--- Custom Style Examples ---" << "\n";

  print_style_t custom_style;
  custom_style.border_h = "=";
  custom_style.border_v = "*";
  custom_style.corner = "#";
  custom_style.padding = "";  // 无填充 (No padding)
  custom_style.show_header = false;
  custom_style.alignment = align_t::CENTER;

  // 自定义样式的表格 (Table with custom style)
  table_t custom_table(custom_style);
  custom_table.set_headers({"X", "Y"}).add_row(1, 2).add_row(3, 4);
  std::cout << custom_table << "\n";

  // 自定义样式的Vector (Vector with custom style and colors)
  print_style_t color_style;
  color_style.header_fg = color_t::YELLOW;
  color_style.header_bg = color_t::BLUE;
  color_style.data_fg = color_t::GREEN;
  color_style.border_color = color_t::RED;

  std::vector<std::string> color_vec = {"Red", "Green", "Blue"};
  std::cout << print_vector(color_vec, "ColorVector", color_style) << "\n";

  // 禁用颜色的Map (Map with colors disabled)
  print_style_t no_color_style = color_style;
  no_color_style.use_colors = false;
  std::cout << print_map(map_data, "NoColorMap", no_color_style) << "\n";

  return 0;
}

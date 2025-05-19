#include <iomanip>
#include <limits>  // For numeric_limits
#include <numeric>  // For std::iota in vector<double> benchmark
#include <random>  // For generating random points
#include <sstream>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

// Include the headers to be benchmarked
#include <cpp-toolbox/types/minmax.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/types/point_utils.hpp>  // Contains point generation functions
#include <cpp-toolbox/utils/print.hpp>
#include <cpp-toolbox/utils/timer.hpp>

// --- Helper Function to generate random points ---
std::vector<toolbox::types::point_t<double>> generate_random_points(
    size_t count)
{
  std::vector<toolbox::types::point_t<double>> points;
  points.reserve(count);

  std::mt19937 gen(12345);  // Fixed seed for reproducibility
  std::uniform_real_distribution<double> distrib(-1000.0, 1000.0);

  for (size_t i = 0; i < count; ++i) {
    points.emplace_back(distrib(gen), distrib(gen), distrib(gen));
  }
  return points;
}

// --- Benchmarks ---

TEST_CASE("Benchmark Types Utilities")
{
  // --- Setup Common Data ---
  const size_t data_size = 10'000'000;  // Ten million elements -
                                        // 增加数据集大小以获得更准确的计时结果

  // Define bounds for point generation
  toolbox::types::minmax_t<toolbox::types::point_t<double>> bounds(
      toolbox::types::point_t<double>(-1000.0, -1000.0, -1000.0),
      toolbox::types::point_t<double>(1000.0, 1000.0, 1000.0));

  // Pre-generate data for minmax benchmarks to isolate calculation time
  auto points_vector_for_minmax =
      toolbox::types::generate_random_points(data_size, bounds);
  REQUIRE(points_vector_for_minmax.size() == data_size);
  toolbox::types::point_cloud_t<double> point_cloud;
  point_cloud.points = points_vector_for_minmax;  // Copy points
  REQUIRE(point_cloud.size() == data_size);
  std::vector<double> double_vector(data_size);
  std::iota(double_vector.begin(), double_vector.end(), 0.0);
  REQUIRE(double_vector.size() == data_size);

  // --- Benchmark MinMax Calculation Sections ---

  SECTION("MinMax Calculation: Vector of Points")
  {
    BENCHMARK("Sequential MinMax (vector<point_t>)")
    {
      auto result = toolbox::types::calculate_minmax(points_vector_for_minmax);
      return result.min.x;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (vector<point_t>)")
    {
      auto result =
          toolbox::types::calculate_minmax_parallel(points_vector_for_minmax);
      return result.min.x;  // Prevent optimization
    };
  }

  SECTION("MinMax Calculation: PointCloud")
  {
    BENCHMARK("Sequential MinMax (point_cloud_t)")
    {
      auto result = toolbox::types::calculate_minmax(point_cloud);
      return result.min.x;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (point_cloud_t)")
    {
      auto result = toolbox::types::calculate_minmax_parallel(point_cloud);
      return result.min.x;  // Prevent optimization
    };
  }

  SECTION("MinMax Calculation: Vector of Doubles")
  {
    BENCHMARK("Sequential MinMax (vector<double>)")
    {
      auto result = toolbox::types::calculate_minmax(double_vector);
      return result.min;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (vector<double>)")
    {
      auto result = toolbox::types::calculate_minmax_parallel(double_vector);
      return result.min;  // Prevent optimization
    };
  }

  // --- Benchmark Point Generation Section ---
  SECTION("Point Generation")
  {
    // Use a slightly smaller size for generation benchmark if needed,
    // as generation might be slower overall. Sticking to data_size for now.
    const size_t generation_size = data_size;

    BENCHMARK("Sequential Generation (generate_random_points)")
    {
      // Generate points inside the benchmark
      auto points =
          toolbox::types::generate_random_points(generation_size, bounds);
      return points.size();  // Return size to prevent optimization
    };

    BENCHMARK("Parallel Generation (generate_random_points_parallel)")
    {
      // Generate points inside the benchmark
      auto points = toolbox::types::generate_random_points_parallel(
          generation_size, bounds);
      return points.size();  // Return size to prevent optimization
    };
  }

  // --- Timing Table ---
  SECTION("Timing Table")
  {
    // 使用 toolbox::utils::stop_watch_timer_t 进行更准确的计时
    // Use toolbox::utils::stop_watch_timer_t for more accurate timing
    auto measure = [&](auto&& func)
    {
      const int iters = 5;  // 增加迭代次数以获得更稳定的结果
      double total_ms = 0.0;
      for (int i = 0; i < iters; ++i) {
        toolbox::utils::stop_watch_timer_t timer;
        timer.start();
        func();
        timer.stop();
        total_ms += timer.elapsed_time_ms();  // 直接获取毫秒值
      }
      return total_ms / static_cast<double>(iters);
    };

    // Measure MinMax calculations - 增加迭代次数以获得更准确的计时
    // Increase iteration count to get more accurate timing
    const int iterations = 50;  // 增加迭代次数

    double vector_points_seq =
        measure(
            [&]()
            {
              // 执行多次以增加总执行时间
              // Execute multiple times to increase total execution time
              auto result =
                  toolbox::types::calculate_minmax(points_vector_for_minmax);
              for (int i = 1; i < iterations; ++i) {
                // 重复计算多次，但不保存结果，只是为了增加执行时间
                // Repeat calculation multiple times to increase execution time
                auto temp =
                    toolbox::types::calculate_minmax(points_vector_for_minmax);
                // 使用 volatile 防止编译器优化掉这个计算
                // Use volatile to prevent compiler from optimizing out this
                // calculation
                volatile float x = temp.min.x;
                (void)
                    x;  // 防止未使用变量警告 / Prevent unused variable warning
              }
              return result.min.x;  // Return to prevent optimization
            })
        / static_cast<double>(iterations);
    double vector_points_par = measure(
        [&]()
        {
          auto result = toolbox::types::calculate_minmax_parallel(
              points_vector_for_minmax);
          return result.min.x;  // Return to prevent optimization
        });

    double point_cloud_seq =
        measure(
            [&]()
            {
              // 执行多次以增加总执行时间
              // Execute multiple times to increase total execution time
              auto result = toolbox::types::calculate_minmax(point_cloud);
              for (int i = 1; i < iterations; ++i) {
                auto temp = toolbox::types::calculate_minmax(point_cloud);
                volatile float dummy = temp.min.x;
                (void)dummy;  // 防止未使用变量警告
              }
              return result.min.x;  // Return to prevent optimization
            })
        / static_cast<double>(iterations);
    double point_cloud_par = measure(
        [&]()
        {
          auto result = toolbox::types::calculate_minmax_parallel(point_cloud);
          return result.min.x;  // Return to prevent optimization
        });

    double vector_doubles_seq =
        measure(
            [&]()
            {
              // 执行多次以增加总执行时间
              // Execute multiple times to increase total execution time
              auto result = toolbox::types::calculate_minmax(double_vector);
              for (int i = 1; i < iterations; ++i) {
                auto temp = toolbox::types::calculate_minmax(double_vector);
                volatile double dummy = temp.min;
                (void)dummy;  // 防止未使用变量警告
              }
              return result.min;  // Return to prevent optimization
            })
        / static_cast<double>(iterations);
    double vector_doubles_par = measure(
        [&]()
        {
          auto result =
              toolbox::types::calculate_minmax_parallel(double_vector);
          return result.min;  // Return to prevent optimization
        });

    // Measure point generation
    const size_t generation_size = data_size;
    double gen_seq = measure(
        [&]()
        {
          auto result =
              toolbox::types::generate_random_points(generation_size, bounds);
          return result.size();  // Return to prevent optimization
        });
    double gen_par = measure(
        [&]()
        {
          auto result = toolbox::types::generate_random_points_parallel(
              generation_size, bounds);
          return result.size();  // Return to prevent optimization
        });

    // Create and display the results table
    toolbox::utils::table_t table;
    table.set_headers(
        {"Benchmark", "Sequential (ms)", "Parallel (ms)", "Speedup"});
    auto add_row =
        [&](const std::string& name, double serial_ms, double parallel_ms)
    {
      // 添加调试输出，显示实际的计时值
      // Add debug output to show actual timing values
      std::cout << "DEBUG - " << name << " - Serial: " << serial_ms
                << " ms, Parallel: " << parallel_ms
                << " ms, Speedup: " << (serial_ms / parallel_ms) << "\n";

      // 已经是毫秒值，不需要转换
      // Already in milliseconds, no conversion needed
      std::ostringstream serial_str;
      serial_str.setf(std::ios::fixed);
      serial_str << std::setprecision(3) << serial_ms;

      std::ostringstream parallel_str;
      parallel_str.setf(std::ios::fixed);
      parallel_str << std::setprecision(3) << parallel_ms;

      // 计算加速比
      // Calculate speedup
      double speedup = 1.0;
      if (parallel_ms > 0.001) {  // 避免除以非常小的值 / Avoid division by very
                                  // small values
        speedup = serial_ms / parallel_ms;
      }

      std::ostringstream speedup_str;
      speedup_str.setf(std::ios::fixed);
      speedup_str << std::setprecision(2) << speedup;

      table.add_row(
          name, serial_str.str(), parallel_str.str(), speedup_str.str());
    };

    add_row("MinMax (vector<point_t>)", vector_points_seq, vector_points_par);
    add_row("MinMax (point_cloud_t)", point_cloud_seq, point_cloud_par);
    add_row("MinMax (vector<double>)", vector_doubles_seq, vector_doubles_par);
    add_row("Point Generation", gen_seq, gen_par);

    std::cout << table << "\n";

    REQUIRE(vector_points_seq > 0.0);
    REQUIRE(vector_points_par > 0.0);
    REQUIRE(point_cloud_seq > 0.0);
    REQUIRE(point_cloud_par > 0.0);
    REQUIRE(vector_doubles_seq > 0.0);
    REQUIRE(vector_doubles_par > 0.0);
    REQUIRE(gen_seq > 0.0);
    REQUIRE(gen_par > 0.0);
  }
}

// Reminder: Ensure cpp-toolbox library (especially thread pool) is linked
// correctly and include directories are set up in your CMakeLists.txt for this
// benchmark target.

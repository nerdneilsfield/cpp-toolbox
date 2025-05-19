#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/pcl/filters/random_downsampling.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/types/minmax.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/types/point_utils.hpp>
#include <cpp-toolbox/utils/print.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

using toolbox::pcl::random_downsampling_t;
using toolbox::pcl::voxel_grid_downsampling_t;
using toolbox::types::generate_random_points_parallel;
using toolbox::types::minmax_t;
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

TEST_CASE("Downsampling Filters Benchmark", "[benchmark][pcl]")
{
  constexpr std::size_t point_count = 1000000;

  minmax_t<point_t<float>> bounds({-20.0F, -100.0F, -100.0F},
                                  {20.0F, 100.0F, 100.0F});

  auto cloud = std::make_shared<point_cloud_t<float>>();
  cloud->points = generate_random_points_parallel<float>(point_count, bounds);
  REQUIRE(cloud->size() == point_count);

  SECTION("Correctness Random Downsampling")
  {
    random_downsampling_t<float> filter(0.3F);
    filter.set_input(cloud);
    toolbox::utils::random_t::instance().seed(42);
    auto serial_result = filter.filter();
    toolbox::utils::random_t::instance().seed(42);
    filter.enable_parallel(true);
    auto parallel_result = filter.filter();
    REQUIRE(parallel_result.points.size()
            == serial_result.points
                   .size());  // First, ensure total sizes are the same
    const size_t num_to_compare =
        std::min(static_cast<size_t>(20), serial_result.points.size());
    for (size_t i = 0; i < num_to_compare; ++i) {
      REQUIRE(parallel_result.points[i] == serial_result.points[i]);
    }
  }

  SECTION("Correctness Voxel Grid Downsampling")
  {
    voxel_grid_downsampling_t<float> filter(0.5F);
    filter.set_input(cloud);
    auto serial_result = filter.filter();
    filter.enable_parallel(true);
    auto parallel_result = filter.filter();
    REQUIRE(parallel_result.size() == serial_result.size());
    for (auto i = 0; i < 20; ++i) {
      const auto& p = serial_result.points[i];
      auto it = std::find_if(parallel_result.points.begin(),
                             parallel_result.points.end(),
                             [&](const auto& q)
                             {
                               return std::fabs(p.x - q.x) < 1e-6f
                                   && std::fabs(p.y - q.y) < 1e-6f
                                   && std::fabs(p.z - q.z) < 1e-6f;
                             });
      REQUIRE(it != parallel_result.points.end());
    }
  }

  SECTION("Benchmark Random Downsampling")
  {
    random_downsampling_t<float> filter(0.3F);
    filter.set_input(cloud);
    BENCHMARK("Serial Random Downsampling")
    {
      toolbox::utils::random_t::instance().seed(7);
      filter.enable_parallel(false);
      return filter.filter().size();
    };
    BENCHMARK("Parallel Random Downsampling")
    {
      toolbox::utils::random_t::instance().seed(7);
      filter.enable_parallel(true);
      return filter.filter().size();
    };
  }

  SECTION("Benchmark Voxel Grid Downsampling")
  {
    voxel_grid_downsampling_t<float> filter(0.5F);
    filter.set_input(cloud);
    BENCHMARK("Serial Voxel Grid Downsampling")
    {
      filter.enable_parallel(false);
      return filter.filter().size();
    };
    BENCHMARK("Parallel Voxel Grid Downsampling")
    {
      filter.enable_parallel(true);
      return filter.filter().size();
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

    // Measure random downsampling
    random_downsampling_t<float> random_filter(0.3F);
    random_filter.set_input(cloud);

    toolbox::utils::random_t::instance().seed(7);
    random_filter.enable_parallel(false);
    double random_serial = measure([&]() { random_filter.filter(); });

    toolbox::utils::random_t::instance().seed(7);
    random_filter.enable_parallel(true);
    double random_parallel = measure([&]() { random_filter.filter(); });

    // Measure voxel grid downsampling
    voxel_grid_downsampling_t<float> voxel_filter(0.5F);
    voxel_filter.set_input(cloud);

    voxel_filter.enable_parallel(false);
    double voxel_serial = measure([&]() { voxel_filter.filter(); });

    voxel_filter.enable_parallel(true);
    double voxel_parallel = measure([&]() { voxel_filter.filter(); });

    // Create and display the results table
    toolbox::utils::table_t table;
    table.set_headers({"Benchmark", "Serial (ms)", "Parallel (ms)", "Speedup"});
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

    add_row("Random Downsampling", random_serial, random_parallel);
    add_row("Voxel Grid Downsampling", voxel_serial, voxel_parallel);

    std::cout << table << "\n";

    REQUIRE(random_serial > 0.0);
    REQUIRE(random_parallel > 0.0);
    REQUIRE(voxel_serial > 0.0);
    REQUIRE(voxel_parallel > 0.0);
  }
}

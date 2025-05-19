#include <cmath>
#include <memory>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/pcl/filters/random_downsampling.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/types/minmax.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/types/point_utils.hpp>
#include <cpp-toolbox/utils/random.hpp>

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
}

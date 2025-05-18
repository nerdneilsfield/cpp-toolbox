#include <cmath>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/filters/random_downsampling.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

using toolbox::pcl::random_downsampling_t;
using toolbox::pcl::voxel_grid_downsampling_t;
using toolbox::types::point_cloud_t;

TEST_CASE("Downsampling Filters Benchmark", "[benchmark][pcl]")
{
  constexpr std::size_t point_count = 200000;
  point_cloud_t<float> cloud;
  cloud.points.reserve(point_count);
  auto& rng = toolbox::utils::random_t::instance();
  rng.seed(12345);
  for (std::size_t i = 0; i < point_count; ++i) {
    cloud.points.emplace_back(rng.random_float<float>(-1000.0f, 1000.0f),
                              rng.random_float<float>(-1000.0f, 1000.0f),
                              rng.random_float<float>(-1000.0f, 1000.0f));
  }

  SECTION("Correctness Random Downsampling")
  {
    random_downsampling_t<float> filter(0.3F);
    filter.set_input(cloud);
    rng.seed(42);
    auto serial_result = filter.filter();
    rng.seed(42);
    filter.enable_parrallel(true);
    auto parallel_result = filter.filter();
    REQUIRE(parallel_result.points == serial_result.points);
  }

  SECTION("Correctness Voxel Grid Downsampling")
  {
    voxel_grid_downsampling_t<float> filter(0.5F);
    filter.set_input(cloud);
    auto serial_result = filter.filter();
    filter.enable_parrallel(true);
    auto parallel_result = filter.filter();
    REQUIRE(parallel_result.size() == serial_result.size());
    for (const auto& p : serial_result.points) {
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
      rng.seed(7);
      filter.enable_parrallel(false);
      return filter.filter().size();
    };
    BENCHMARK("Parallel Random Downsampling")
    {
      rng.seed(7);
      filter.enable_parrallel(true);
      return filter.filter().size();
    };
  }

  SECTION("Benchmark Voxel Grid Downsampling")
  {
    voxel_grid_downsampling_t<float> filter(0.5F);
    filter.set_input(cloud);
    BENCHMARK("Serial Voxel Grid Downsampling")
    {
      filter.enable_parrallel(false);
      return filter.filter().size();
    };
    BENCHMARK("Parallel Voxel Grid Downsampling")
    {
      filter.enable_parrallel(true);
      return filter.filter().size();
    };
  }
}

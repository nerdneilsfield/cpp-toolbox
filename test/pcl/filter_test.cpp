#include <algorithm>
#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/pcl/filters/random_downsampling.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

using toolbox::pcl::random_downsampling_t;
using toolbox::pcl::voxel_grid_downsampling_t;
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

TEST_CASE("Random downsampling filter", "[pcl][filter][random]")
{
  point_cloud_t<float> cloud;
  for (int i = 0; i < 10; ++i) {
    cloud.points.emplace_back(
        static_cast<float>(i), static_cast<float>(i), static_cast<float>(i));
  }

  toolbox::utils::random_t::instance().seed(42);

  random_downsampling_t<float> filter(0.5F);
  filter.set_input(cloud);
  auto result = filter.filter();

  REQUIRE(result.size() == 5);
  for (const auto& p : result.points) {
    auto it = std::find_if(cloud.points.begin(),
                           cloud.points.end(),
                           [&](const auto& q)
                           {
                             return std::fabs(q.x - p.x) < 1e-6f
                                 && std::fabs(q.y - p.y) < 1e-6f
                                 && std::fabs(q.z - p.z) < 1e-6f;
                           });
    REQUIRE(it != cloud.points.end());
  }

  toolbox::utils::random_t::instance().seed(42);
  auto out_ptr = std::make_shared<point_cloud_t<float>>();
  filter.filter(out_ptr);
  REQUIRE(out_ptr->size() == result.size());
  for (std::size_t i = 0; i < result.size(); ++i) {
    REQUIRE(std::fabs(out_ptr->points[i].x - result.points[i].x) < 1e-6f);
  }

  toolbox::utils::random_t::instance().seed(42);
  filter.enable_parrallel(true);
  auto parallel_result = filter.filter();
  REQUIRE(parallel_result.points == result.points);
}

TEST_CASE("Voxel grid downsampling filter", "[pcl][filter][voxel]")
{
  point_cloud_t<float> cloud;
  cloud.points.emplace_back(0.1f, 0.1f, 0.1f);
  cloud.points.emplace_back(0.9f, 0.9f, 0.9f);
  cloud.points.emplace_back(1.1f, 1.1f, 1.1f);
  cloud.points.emplace_back(1.9f, 1.9f, 1.9f);

  voxel_grid_downsampling_t<float> filter(1.0f);
  filter.set_input(cloud);
  auto result = filter.filter();

  REQUIRE(result.size() == 2);
  auto check = [](const point_t<float>& p)
  {
    return (std::fabs(p.x - 0.5f) < 1e-6f && std::fabs(p.y - 0.5f) < 1e-6f
            && std::fabs(p.z - 0.5f) < 1e-6f)
        || (std::fabs(p.x - 1.5f) < 1e-6f && std::fabs(p.y - 1.5f) < 1e-6f
            && std::fabs(p.z - 1.5f) < 1e-6f);
  };
  REQUIRE(check(result.points[0]));
  REQUIRE(check(result.points[1]));

  filter.enable_parrallel(true);
  auto parallel_result = filter.filter();
  REQUIRE(parallel_result.size() == result.size());
  for (const auto& p : result.points) {
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

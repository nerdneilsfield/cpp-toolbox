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
  filter.enable_parallel(true);
  auto parallel_result = filter.filter();
  REQUIRE(parallel_result.points == result.points);
}

TEST_CASE("Voxel grid downsampling filter", "[pcl][filter][voxel]")
{
  SECTION("Basic voxel grid test")
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

    filter.enable_parallel(true);
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

  SECTION("Voxel grid with multiple points per voxel")
  {
    // 创建一个点云，每个体素内有多个点
    point_cloud_t<float> cloud;

    // 体素1: 应该生成质心 (0.5, 0.5, 0.5)
    cloud.points.emplace_back(0.1f, 0.2f, 0.3f);
    cloud.points.emplace_back(0.4f, 0.5f, 0.6f);
    cloud.points.emplace_back(0.7f, 0.8f, 0.9f);

    // 体素2: 应该生成质心 (1.5, 1.5, 1.5)
    cloud.points.emplace_back(1.3f, 1.3f, 1.3f);
    cloud.points.emplace_back(1.7f, 1.7f, 1.7f);

    // 体素3: 应该生成质心 (2.5, 2.5, 2.5)
    cloud.points.emplace_back(2.5f, 2.5f, 2.5f);

    voxel_grid_downsampling_t<float> filter(1.0f);
    filter.set_input(cloud);
    auto result = filter.filter();

    // 应该有3个体素
    REQUIRE(result.size() == 3);

    // 验证每个体素的质心位置
    // 注意：输出点的顺序可能不确定，所以需要查找匹配的点
    auto find_point =
        [&result](float x, float y, float z, float tolerance = 1e-5f) -> bool
    {
      for (const auto& p : result.points) {
        if (std::fabs(p.x - x) < tolerance && std::fabs(p.y - y) < tolerance
            && std::fabs(p.z - z) < tolerance)
        {
          return true;
        }
      }
      return false;
    };

    // 验证三个体素的质心
    REQUIRE(
        find_point(0.4f,
                   0.5f,
                   0.6f));  // (0.1+0.4+0.7)/3, (0.2+0.5+0.8)/3, (0.3+0.6+0.9)/3
    REQUIRE(
        find_point(1.5f, 1.5f, 1.5f));  // (1.3+1.7)/2, (1.3+1.7)/2, (1.3+1.7)/2
    REQUIRE(find_point(2.5f, 2.5f, 2.5f));  // 单点体素
  }

  SECTION("Voxel grid with normals and colors")
  {
    point_cloud_t<float> cloud;

    // 添加点
    cloud.points.emplace_back(0.1f, 0.2f, 0.3f);
    cloud.points.emplace_back(0.4f, 0.5f, 0.6f);

    // 添加法线
    cloud.normals.emplace_back(1.0f, 0.0f, 0.0f);
    cloud.normals.emplace_back(0.0f, 1.0f, 0.0f);

    // 添加颜色
    cloud.colors.emplace_back(1.0f, 0.0f, 0.0f);  // 红色
    cloud.colors.emplace_back(0.0f, 1.0f, 0.0f);  // 绿色

    voxel_grid_downsampling_t<float> filter(1.0f);
    filter.set_input(cloud);
    auto result = filter.filter();

    // 应该只有一个体素
    REQUIRE(result.size() == 1);

    // 验证点坐标（质心）
    REQUIRE(std::fabs(result.points[0].x - 0.25f) < 1e-6f);
    REQUIRE(std::fabs(result.points[0].y - 0.35f) < 1e-6f);
    REQUIRE(std::fabs(result.points[0].z - 0.45f) < 1e-6f);

    // 验证法线存在
    REQUIRE_FALSE(result.normals.empty());

    // 打印实际的法线值，以便调试
    INFO("Actual normal: (" << result.normals[0].x << ", "
                            << result.normals[0].y << ", "
                            << result.normals[0].z << ")");

    // 验证法线是否是平均值
    const float tolerance = 1e-5f;
    REQUIRE(std::fabs(result.normals[0].x - 0.5f) < tolerance);
    REQUIRE(std::fabs(result.normals[0].y - 0.5f) < tolerance);
    REQUIRE(std::fabs(result.normals[0].z - 0.0f) < tolerance);

    // 验证颜色（应该是平均值）
    REQUIRE_FALSE(result.colors.empty());
    REQUIRE(std::fabs(result.colors[0].x - 0.5f) < 1e-6f);  // 红色分量
    REQUIRE(std::fabs(result.colors[0].y - 0.5f) < 1e-6f);  // 绿色分量
    REQUIRE(std::fabs(result.colors[0].z - 0.0f) < 1e-6f);  // 蓝色分量
  }

  SECTION("Voxel grid with different voxel sizes")
  {
    // 创建一个均匀分布的点云
    point_cloud_t<float> cloud;

    // 使用整数索引避免浮点数循环的精度问题
    const float step = 0.5f;
    const float max_coord = 10.0f;
    const int steps = static_cast<int>(max_coord / step);

    for (int ix = 0; ix < steps; ++ix) {
      float x = ix * step;
      for (int iy = 0; iy < steps; ++iy) {
        float y = iy * step;
        for (int iz = 0; iz < steps; ++iz) {
          float z = iz * step;
          cloud.points.emplace_back(x, y, z);
        }
      }
    }

    // 测试不同的体素大小
    std::vector<float> voxel_sizes = {0.5f, 1.0f, 2.0f, 5.0f};
    std::vector<size_t> expected_counts = {
        8000, 1000, 125, 8};  // 理论上的体素数量

    for (size_t i = 0; i < voxel_sizes.size(); ++i) {
      voxel_grid_downsampling_t<float> filter(voxel_sizes[i]);
      filter.set_input(cloud);
      auto result = filter.filter();

      // 验证结果点数是否接近预期
      // 注意：由于边界效应，实际数量可能与理论值有小偏差
      const float tolerance = 0.1f;  // 允许10%的误差
      const auto min_expected = static_cast<size_t>(
          static_cast<float>(expected_counts[i]) * (1.0f - tolerance));
      const auto max_expected = static_cast<size_t>(
          static_cast<float>(expected_counts[i]) * (1.0f + tolerance));

      INFO("Voxel size: " << voxel_sizes[i]
                          << ", Expected count: " << expected_counts[i]
                          << ", Actual count: " << result.size());
      REQUIRE(result.size() >= min_expected);
      REQUIRE(result.size() <= max_expected);

      // 验证每个点是否在合理的范围内
      // 由于体素下采样是取平均值，而不是几何中心，所以我们只需要验证点在体素内即可
      for (const auto& point : result.points) {
        // 计算该点应该在哪个体素
        const int voxel_x =
            static_cast<int>(std::floor(point.x / voxel_sizes[i]));
        const int voxel_y =
            static_cast<int>(std::floor(point.y / voxel_sizes[i]));
        const int voxel_z =
            static_cast<int>(std::floor(point.z / voxel_sizes[i]));

        // 计算该体素的边界
        const float min_x = voxel_x * voxel_sizes[i];
        const float min_y = voxel_y * voxel_sizes[i];
        const float min_z = voxel_z * voxel_sizes[i];
        const float max_x = (voxel_x + 1) * voxel_sizes[i];
        const float max_y = (voxel_y + 1) * voxel_sizes[i];
        const float max_z = (voxel_z + 1) * voxel_sizes[i];

        // 验证点在体素内或非常接近体素边界
        const float epsilon = 1e-5f;
        REQUIRE(point.x >= min_x - epsilon);
        REQUIRE(point.x <= max_x + epsilon);
        REQUIRE(point.y >= min_y - epsilon);
        REQUIRE(point.y <= max_y + epsilon);
        REQUIRE(point.z >= min_z - epsilon);
        REQUIRE(point.z <= max_z + epsilon);
      }
    }
  }

  SECTION("Empty point cloud handling")
  {
    point_cloud_t<float> empty_cloud;
    voxel_grid_downsampling_t<float> filter(1.0f);
    filter.set_input(empty_cloud);
    auto result = filter.filter();

    // 空点云应该返回空结果
    REQUIRE(result.empty());
  }

  SECTION("Serial vs Parallel consistency")
  {
    // 创建一个较大的随机点云
    point_cloud_t<float> cloud;
    const size_t num_points = 10000;

    // 使用随机数生成器创建点
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

    for (size_t i = 0; i < num_points; ++i) {
      cloud.points.emplace_back(dist(gen), dist(gen), dist(gen));
    }

    voxel_grid_downsampling_t<float> filter(1.0f);
    filter.set_input(cloud);

    // 串行处理
    filter.enable_parallel(false);
    auto serial_result = filter.filter();

    // 并行处理
    filter.enable_parallel(true);
    auto parallel_result = filter.filter();

    // 验证点数相同
    REQUIRE(parallel_result.size() == serial_result.size());

    // 验证每个点都能在另一个结果中找到匹配
    // 注意：由于哈希表的不确定性，点的顺序可能不同
    size_t matched_points = 0;
    const float tolerance = 1e-5f;

    for (const auto& p : serial_result.points) {
      auto it = std::find_if(parallel_result.points.begin(),
                             parallel_result.points.end(),
                             [&](const auto& q)
                             {
                               return std::fabs(p.x - q.x) < tolerance
                                   && std::fabs(p.y - q.y) < tolerance
                                   && std::fabs(p.z - q.z) < tolerance;
                             });
      if (it != parallel_result.points.end()) {
        matched_points++;
      }
    }

    // 所有点都应该能找到匹配
    REQUIRE(matched_points == serial_result.size());
  }
}

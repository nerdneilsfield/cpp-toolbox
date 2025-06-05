#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/registration/registration.hpp>
#include <cpp-toolbox/io/formats/pcd.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

#include <Eigen/Geometry>
#include <memory>

// 基准测试数据目录
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "test/data"
#endif

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::utils;
using namespace toolbox::io;

// 创建不同规模的测试点云
template<typename T>
std::shared_ptr<point_cloud_t<T>> create_benchmark_cloud(std::size_t num_points)
{
  auto cloud = std::make_shared<point_cloud_t<T>>();
  cloud->points.reserve(num_points);
  
  random_t rng;
  
  // 创建有结构的点云（不完全随机）
  std::size_t grid_size = static_cast<std::size_t>(std::cbrt(num_points));
  T step = 20.0 / grid_size;
  
  for (std::size_t i = 0; i < grid_size; ++i) {
    for (std::size_t j = 0; j < grid_size; ++j) {
      for (std::size_t k = 0; k < grid_size; ++k) {
        if (cloud->points.size() >= num_points) break;
        
        point_t<T> p;
        p.x = -10.0 + i * step + rng.random<T>(-0.1, 0.1);
        p.y = -10.0 + j * step + rng.random<T>(-0.1, 0.1);
        p.z = -10.0 + k * step + rng.random<T>(-0.1, 0.1);
        cloud->points.push_back(p);
      }
    }
  }
  
  // 填充剩余的点
  while (cloud->points.size() < num_points) {
    point_t<T> p;
    p.x = rng.random<T>(-10.0, 10.0);
    p.y = rng.random<T>(-10.0, 10.0);
    p.z = rng.random<T>(-10.0, 10.0);
    cloud->points.push_back(p);
  }
  
  
  return cloud;
}

// 添加法线
template<typename T>
void add_normals_benchmark(std::shared_ptr<point_cloud_t<T>> cloud)
{
  cloud->normals.reserve(cloud->size());
  
  for (std::size_t i = 0; i < cloud->size(); ++i) {
    const auto& p = cloud->points[i];
    
    // 简单的法线估计（实际应用中应该用更复杂的方法）
    Eigen::Vector3<T> point(p.x, p.y, p.z);
    Eigen::Vector3<T> normal = point.normalized();
    
    point_t<T> n;
    n.x = normal[0];
    n.y = normal[1];
    n.z = normal[2];
    
    cloud->normals.push_back(n);
  }
}

// 变换点云
template<typename T>
std::shared_ptr<point_cloud_t<T>> transform_cloud_benchmark(
    const point_cloud_t<T>& cloud,
    const Eigen::Matrix<T, 4, 4>& transform)
{
  auto transformed = std::make_shared<point_cloud_t<T>>();
  transformed->points.reserve(cloud.size());
  
  for (const auto& p : cloud.points) {
    Eigen::Vector4<T> pt(p.x, p.y, p.z, 1.0);
    Eigen::Vector4<T> pt_transformed = transform * pt;
    
    point_t<T> p_new;
    p_new.x = pt_transformed[0];
    p_new.y = pt_transformed[1];
    p_new.z = pt_transformed[2];
    
    transformed->points.push_back(p_new);
  }
  
  
  // 变换法线
  if (!cloud.normals.empty()) {
    transformed->normals.reserve(cloud.normals.size());
    Eigen::Matrix<T, 3, 3> rotation = transform.template block<3, 3>(0, 0);
    
    for (const auto& n : cloud.normals) {
      Eigen::Vector3<T> normal(n.x, n.y, n.z);
      Eigen::Vector3<T> normal_transformed = rotation * normal;
      normal_transformed.normalize();
      
      point_t<T> n_new;
      n_new.x = normal_transformed[0];
      n_new.y = normal_transformed[1];
      n_new.z = normal_transformed[2];
      
      transformed->normals.push_back(n_new);
    }
  }
  
  return transformed;
}

// 创建基准测试变换
template<typename T>
Eigen::Matrix<T, 4, 4> create_benchmark_transform()
{
  Eigen::Matrix<T, 4, 4> transform = Eigen::Matrix<T, 4, 4>::Identity();
  
  // 适中的变换
  T angle = 0.1;  // ~5.7度
  Eigen::AngleAxis<T> rotation(angle, Eigen::Vector3<T>(1, 1, 1).normalized());
  transform.template block<3, 3>(0, 0) = rotation.matrix();
  
  transform(0, 3) = 0.5;
  transform(1, 3) = 0.3;
  transform(2, 3) = 0.2;
  
  return transform;
}

TEST_CASE("细配准算法性能基准 / Fine registration performance benchmark", "[benchmark][pcl][fine_registration]")
{
  using T = float;
  
  // 测试不同规模的点云
  std::vector<std::size_t> cloud_sizes = {1000, 5000, 10000};
  
  for (auto num_points : cloud_sizes) {
    // 准备测试数据
    auto source = create_benchmark_cloud<T>(num_points);
    add_normals_benchmark(source);
    
    auto transform = create_benchmark_transform<T>();
    auto target = transform_cloud_benchmark(*source, transform);
    
    BENCHMARK(std::string("Point-to-Point ICP - " + std::to_string(num_points) + " points").c_str())
    {
      point_to_point_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(30);
      
      fine_registration_result_t<T> result;
      return icp.align(result);
    };
    
    BENCHMARK(std::string("Point-to-Plane ICP - " + std::to_string(num_points) + " points").c_str())
    {
      point_to_plane_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(30);
      
      fine_registration_result_t<T> result;
      return icp.align(result);
    };
    
    BENCHMARK(std::string("Generalized ICP - " + std::to_string(num_points) + " points").c_str())
    {
      generalized_icp_t<T> gicp;
      gicp.set_source(source);
      gicp.set_target(target);
      gicp.set_max_iterations(30);
      gicp.set_k_correspondences(20);
      
      fine_registration_result_t<T> result;
      return gicp.align(result);
    };
    
    BENCHMARK(std::string("AA-ICP - " + std::to_string(num_points) + " points").c_str())
    {
      aa_icp_t<T> aa_icp;
      aa_icp.set_source(source);
      aa_icp.set_target(target);
      aa_icp.set_max_iterations(30);
      aa_icp.set_anderson_m(5);
      
      fine_registration_result_t<T> result;
      return aa_icp.align(result);
    };
    
    BENCHMARK(std::string("NDT - " + std::to_string(num_points) + " points").c_str())
    {
      ndt_t<T> ndt;
      ndt.set_source(source);
      ndt.set_target(target);
      ndt.set_resolution(1.0);
      ndt.set_max_iterations(30);
      
      fine_registration_result_t<T> result;
      return ndt.align(result);
    };
  }
}

TEST_CASE("细配准并行性能 / Fine registration parallel performance", "[benchmark][pcl][fine_registration][parallel]")
{
  using T = float;
  
  const std::size_t num_points = 10000;
  auto source = create_benchmark_cloud<T>(num_points);
  auto transform = create_benchmark_transform<T>();
  auto target = transform_cloud_benchmark(*source, transform);
  
  SECTION("Point-to-Point ICP 并行对比 / Parallel comparison")
  {
    BENCHMARK("P2P ICP - 串行 / Serial")
    {
      point_to_point_icp_t<T> icp(false);  // 禁用并行
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(20);
      
      fine_registration_result_t<T> result;
      return icp.align(result);
    };
    
    BENCHMARK("P2P ICP - 并行 / Parallel")
    {
      point_to_point_icp_t<T> icp(true);  // 启用并行
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(20);
      
      fine_registration_result_t<T> result;
      return icp.align(result);
    };
  }
}

TEST_CASE("细配准参数影响 / Fine registration parameter impact", "[benchmark][pcl][fine_registration][params]")
{
  using T = float;
  
  const std::size_t num_points = 5000;
  auto source = create_benchmark_cloud<T>(num_points);
  add_normals_benchmark(source);
  auto transform = create_benchmark_transform<T>();
  auto target = transform_cloud_benchmark(*source, transform);
  
  SECTION("迭代次数影响 / Iteration count impact")
  {
    std::vector<std::size_t> max_iterations = {10, 30, 50};
    
    for (auto max_iter : max_iterations) {
      BENCHMARK(std::string("P2P ICP - " + std::to_string(max_iter) + " iterations").c_str())
      {
        point_to_point_icp_t<T> icp;
        icp.set_source(source);
        icp.set_target(target);
        icp.set_max_iterations(max_iter);
        
        fine_registration_result_t<T> result;
        return icp.align(result);
      };
    }
  }
  
  SECTION("NDT分辨率影响 / NDT resolution impact")
  {
    std::vector<T> resolutions = {0.5, 1.0, 2.0};
    
    for (auto res : resolutions) {
      BENCHMARK(std::string("NDT - resolution " + std::to_string(res)).c_str())
      {
        ndt_t<T> ndt;
        ndt.set_source(source);
        ndt.set_target(target);
        ndt.set_resolution(res);
        ndt.set_max_iterations(30);
        
        fine_registration_result_t<T> result;
        return ndt.align(result);
      };
    }
  }
  
  SECTION("AA-ICP历史窗口影响 / AA-ICP history window impact")
  {
    std::vector<std::size_t> m_values = {0, 3, 5, 10};
    
    for (auto m : m_values) {
      BENCHMARK(std::string("AA-ICP - m=" + std::to_string(m)).c_str())
      {
        aa_icp_t<T> aa_icp;
        aa_icp.set_source(source);
        aa_icp.set_target(target);
        aa_icp.set_max_iterations(30);
        aa_icp.set_anderson_m(m);
        
        fine_registration_result_t<T> result;
        return aa_icp.align(result);
      };
    }
  }
}

TEST_CASE("实际点云数据测试 / Real point cloud data test", "[benchmark][pcl][fine_registration][real]")
{
  using T = float;
  
  // 尝试加载实际的PCD文件
  std::string pcd_file = std::string(TEST_DATA_DIR) + "/bunny.pcd";
  
  auto source_unique = read_pcd<T>(pcd_file);
  if (source_unique) {
    auto source = std::shared_ptr<point_cloud_t<T>>(std::move(source_unique));
    // 成功加载真实数据
    INFO("使用真实点云数据 bunny.pcd，点数：" << source->size());
    
    // 创建目标点云
    auto transform = create_benchmark_transform<T>();
    auto target = transform_cloud_benchmark(*source, transform);
    
    // 降采样以加快测试
    if (source->size() > 10000) {
      // 简单的均匀降采样
      auto downsampled_source = std::make_shared<point_cloud_t<T>>();
      auto downsampled_target = std::make_shared<point_cloud_t<T>>();
      
      std::size_t step = source->size() / 10000;
      for (std::size_t i = 0; i < source->size(); i += step) {
        downsampled_source->points.push_back(source->points[i]);
        downsampled_target->points.push_back(target->points[i]);
      }
      
      
      source = std::move(downsampled_source);
      target = std::move(downsampled_target);
    }
    
    BENCHMARK("真实数据 - Point-to-Point ICP / Real data - P2P ICP")
    {
      point_to_point_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(50);
      
      fine_registration_result_t<T> result;
      return icp.align(result);
    };
    
    BENCHMARK("真实数据 - NDT / Real data - NDT")
    {
      ndt_t<T> ndt;
      ndt.set_source(source);
      ndt.set_target(target);
      ndt.set_resolution(0.05);  // 根据bunny模型的尺度调整
      ndt.set_max_iterations(50);
      
      fine_registration_result_t<T> result;
      return ndt.align(result);
    };
  } else {
    // 使用合成数据
    INFO("无法加载真实数据，使用合成点云");
  }
}

TEST_CASE("收敛速度比较 / Convergence speed comparison", "[benchmark][pcl][fine_registration][convergence]")
{
  using T = float;
  
  const std::size_t num_points = 5000;
  auto source = create_benchmark_cloud<T>(num_points);
  add_normals_benchmark(source);
  auto transform = create_benchmark_transform<T>();
  auto target = transform_cloud_benchmark(*source, transform);
  
  // 统计达到特定误差阈值所需的迭代次数
  const T error_threshold = 1e-4;
  
  SECTION("收敛迭代次数 / Convergence iterations")
  {
    {
      point_to_point_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(100);
      icp.set_euclidean_fitness_epsilon(error_threshold);
      icp.set_record_history(true);
      
      fine_registration_result_t<T> result;
      icp.align(result);
      
      INFO("P2P ICP 收敛迭代次数: " << result.iterations_performed);
    }
    
    {
      point_to_plane_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(100);
      icp.set_euclidean_fitness_epsilon(error_threshold);
      icp.set_record_history(true);
      
      fine_registration_result_t<T> result;
      icp.align(result);
      
      INFO("P2L ICP 收敛迭代次数: " << result.iterations_performed);
    }
    
    {
      aa_icp_t<T> aa_icp;
      aa_icp.set_source(source);
      aa_icp.set_target(target);
      aa_icp.set_max_iterations(100);
      aa_icp.set_euclidean_fitness_epsilon(error_threshold);
      aa_icp.set_record_history(true);
      
      fine_registration_result_t<T> result;
      aa_icp.align(result);
      
      INFO("AA-ICP 收敛迭代次数: " << result.iterations_performed);
    }
  }
}
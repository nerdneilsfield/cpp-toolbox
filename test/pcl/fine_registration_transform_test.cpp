#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/pcl/registration/registration.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::utils;

// 辅助函数
template<typename T>
Eigen::Matrix<T, 4, 4> create_test_transform(T tx, T ty, T tz, T rx, T ry, T rz)
{
  Eigen::Matrix<T, 4, 4> transform = Eigen::Matrix<T, 4, 4>::Identity();
  
  // 旋转矩阵（使用欧拉角）
  Eigen::AngleAxis<T> rollAngle(rx, Eigen::Matrix<T, 3, 1>::UnitX());
  Eigen::AngleAxis<T> pitchAngle(ry, Eigen::Matrix<T, 3, 1>::UnitY());
  Eigen::AngleAxis<T> yawAngle(rz, Eigen::Matrix<T, 3, 1>::UnitZ());
  
  Eigen::Quaternion<T> q = yawAngle * pitchAngle * rollAngle;
  transform.template block<3, 3>(0, 0) = q.matrix();
  
  // 平移
  transform(0, 3) = tx;
  transform(1, 3) = ty;
  transform(2, 3) = tz;
  
  return transform;
}

TEST_CASE("Debug transformation issue", "[debug][transform]")
{
  using T = float;
  
  SECTION("Test transform and inverse")
  {
    // 创建简单的测试点云
    auto source = std::make_shared<point_cloud_t<T>>();
    source->points.push_back({0.0f, 0.0f, 0.0f});
    source->points.push_back({1.0f, 0.0f, 0.0f});
    source->points.push_back({0.0f, 1.0f, 0.0f});
    
    // 创建变换：小平移
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 3) = 0.1f;  // x平移
    transform(1, 3) = 0.2f;  // y平移
    transform(2, 3) = 0.3f;  // z平移
    
    std::cout << "Original transform:\n" << transform << std::endl;
    std::cout << "Inverse transform:\n" << transform.inverse() << std::endl;
    
    // 应用变换到源点云得到目标点云
    auto target = std::make_shared<point_cloud_t<T>>();
    for (const auto& p : source->points) {
      Eigen::Vector4f pt(p.x, p.y, p.z, 1.0f);
      Eigen::Vector4f transformed_pt = transform * pt;
      target->points.push_back({transformed_pt[0], transformed_pt[1], transformed_pt[2]});
    }
    
    std::cout << "\nSource points:" << std::endl;
    for (const auto& p : source->points) {
      std::cout << "  (" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
    }
    
    std::cout << "\nTarget points (after transform):" << std::endl;
    for (const auto& p : target->points) {
      std::cout << "  (" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
    }
    
    // 测试1：使用单位矩阵作为初始猜测
    {
      std::cout << "\n--- Test 1: Identity initial guess ---" << std::endl;
      point_to_point_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(50);
      icp.set_max_correspondence_distance(2.0);
      icp.set_record_history(true);
      
      fine_registration_result_t<T> result;
      bool success = icp.align(result);
      
      std::cout << "Success: " << success << std::endl;
      std::cout << "Iterations: " << result.iterations_performed << std::endl;
      std::cout << "Final error: " << result.final_error << std::endl;
      if (!result.history.empty()) {
        std::cout << "First iter correspondences: " << result.history[0].num_correspondences << std::endl;
      }
      
      if (success) {
        std::cout << "Result transformation:\n" << result.transformation << std::endl;
        std::cout << "Expected (inverse of transform):\n" << transform.inverse() << std::endl;
        auto error = (result.transformation - transform.inverse()).norm();
        std::cout << "Transformation error: " << error << std::endl;
      }
    }
    
    // 测试2：使用逆变换作为初始猜测
    {
      std::cout << "\n--- Test 2: Inverse transform initial guess ---" << std::endl;
      point_to_point_icp_t<T> icp;
      icp.set_source(source);
      icp.set_target(target);
      icp.set_max_iterations(50);
      icp.set_max_correspondence_distance(2.0);
      icp.set_record_history(true);
      
      fine_registration_result_t<T> result;
      bool success = icp.align(transform.inverse(), result);
      
      std::cout << "Success: " << success << std::endl;
      std::cout << "Iterations: " << result.iterations_performed << std::endl;
      std::cout << "Final error: " << result.final_error << std::endl;
      if (!result.history.empty()) {
        std::cout << "First iter correspondences: " << result.history[0].num_correspondences << std::endl;
      }
      
      // 使用逆变换作为初始猜测时，应用到源点云后的点
      std::cout << "\nSource points after initial transform:" << std::endl;
      for (const auto& p : source->points) {
        Eigen::Vector4f pt(p.x, p.y, p.z, 1.0f);
        Eigen::Vector4f transformed_pt = transform.inverse() * pt;
        std::cout << "  (" << transformed_pt[0] << ", " << transformed_pt[1] << ", " << transformed_pt[2] << ")" << std::endl;
      }
    }
  }
  
  SECTION("Debug correspondence search with transformation")
  {
    // 创建测试点云 - 使用与主测试相同的方法
    random_t rng;
    auto source = std::make_shared<point_cloud_t<T>>();
    source->points.reserve(10);
    
    for (std::size_t i = 0; i < 10; ++i) {
      point_t<T> p;
      p.x = rng.random<T>(-1.0, 1.0);
      p.y = rng.random<T>(-1.0, 1.0);
      p.z = rng.random<T>(-1.0, 1.0);
      source->points.push_back(p);
    }
    
    // 先测试没有旋转的情况
    auto transform = create_test_transform<T>(0.1, 0.2, 0.3, 0.0, 0.0, 0.0);
    
    // 变换源点云得到目标点云
    auto target = std::make_shared<point_cloud_t<T>>();
    for (const auto& p : source->points) {
      Eigen::Vector4f pt(p.x, p.y, p.z, 1.0f);
      Eigen::Vector4f transformed_pt = transform * pt;
      target->points.push_back({transformed_pt[0], transformed_pt[1], transformed_pt[2]});
    }
    
    std::cout << "\nDebug correspondence with create_test_transform:" << std::endl;
    std::cout << "Transform from create_test_transform:\n" << transform << std::endl;
    
    // 计算点云之间的距离
    T min_dist = std::numeric_limits<T>::max();
    T max_dist = 0;
    for (size_t i = 0; i < source->size(); ++i) {
      const auto& s = source->points[i];
      const auto& t = target->points[i];
      T dist = std::sqrt((s.x - t.x) * (s.x - t.x) + 
                         (s.y - t.y) * (s.y - t.y) + 
                         (s.z - t.z) * (s.z - t.z));
      min_dist = std::min(min_dist, dist);
      max_dist = std::max(max_dist, dist);
    }
    
    std::cout << "Distance between corresponding points:" << std::endl;
    std::cout << "  Min: " << min_dist << ", Max: " << max_dist << std::endl;
    
    // 测试ICP
    point_to_point_icp_t<T> icp;
    icp.set_source(source);
    icp.set_target(target);
    icp.set_max_iterations(50);
    icp.set_max_correspondence_distance(2.0);
    icp.set_record_history(true);
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "ICP result:" << std::endl;
    std::cout << "  Success: " << success << std::endl;
    std::cout << "  Iterations: " << result.iterations_performed << std::endl;
    std::cout << "  Final error: " << result.final_error << std::endl;
    std::cout << "  Termination reason: " << result.termination_reason << std::endl;
    
    if (!result.history.empty()) {
      std::cout << "  First iter correspondences: " << result.history[0].num_correspondences << std::endl;
    }
  }
}
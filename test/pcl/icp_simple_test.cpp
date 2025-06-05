#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cpp-toolbox/pcl/registration/point_to_point_icp.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;

TEST_CASE("Simple ICP test", "[simple][icp]")
{
  using T = float;
  
  SECTION("Minimal test case")
  {
    // 创建非常简单的点云
    auto source = std::make_shared<point_cloud_t<T>>();
    source->points.push_back({0.0f, 0.0f, 0.0f});
    source->points.push_back({1.0f, 0.0f, 0.0f});
    source->points.push_back({0.0f, 1.0f, 0.0f});
    
    // 目标是源的副本（完美对齐）
    auto target = std::make_shared<point_cloud_t<T>>(*source);
    
    std::cout << "\n=== Minimal test case ===" << std::endl;
    std::cout << "Points: " << source->size() << std::endl;
    
    point_to_point_icp_t<T> icp;
    icp.set_source(source);
    icp.set_target(target);
    icp.set_max_iterations(5);
    icp.set_max_correspondence_distance(1.0);
    icp.set_record_history(true);
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "Success: " << success << std::endl;
    std::cout << "Iterations: " << result.iterations_performed << std::endl;
    std::cout << "Converged: " << result.converged << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
    std::cout << "Termination reason: " << result.termination_reason << std::endl;
    
    REQUIRE(success);
    REQUIRE(result.converged);
    REQUIRE_THAT(result.final_error, Catch::Matchers::WithinAbs(0.0, 1e-6));
  }
  
  SECTION("Small translation")
  {
    // 创建源点云
    auto source = std::make_shared<point_cloud_t<T>>();
    source->points.push_back({0.0f, 0.0f, 0.0f});
    source->points.push_back({1.0f, 0.0f, 0.0f});
    source->points.push_back({0.0f, 1.0f, 0.0f});
    source->points.push_back({1.0f, 1.0f, 0.0f});
    
    // 创建平移后的目标点云
    auto target = std::make_shared<point_cloud_t<T>>();
    for (const auto& p : source->points) {
      target->points.push_back({p.x + 0.1f, p.y + 0.1f, p.z});
    }
    
    std::cout << "\n=== Small translation test ===" << std::endl;
    std::cout << "Translation: (0.1, 0.1, 0)" << std::endl;
    
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
    std::cout << "Converged: " << result.converged << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
    std::cout << "Termination reason: " << result.termination_reason << std::endl;
    
    if (!result.history.empty()) {
      std::cout << "History:" << std::endl;
      for (const auto& state : result.history) {
        std::cout << "  Iter " << state.iteration 
                  << ": correspondences=" << state.num_correspondences
                  << ", error=" << state.error << std::endl;
      }
    }
    
    // 检查找到的变换
    std::cout << "Found transformation:\n" << result.transformation << std::endl;
    
    // 期望的变换
    Eigen::Matrix4f expected = Eigen::Matrix4f::Identity();
    expected(0, 3) = 0.1f;
    expected(1, 3) = 0.1f;
    std::cout << "Expected transformation:\n" << expected << std::endl;
    
    auto error = (result.transformation - expected).norm();
    std::cout << "Transformation error: " << error << std::endl;
    
    REQUIRE(success);
    REQUIRE(error < 0.01);
  }
}
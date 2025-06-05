#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/pcl/registration/registration.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::utils;

TEST_CASE("Debug fine registration", "[debug][fine_registration]")
{
  using T = float;
  
  SECTION("Simple perfect alignment debug")
  {
    // Create a very simple test cloud
    auto cloud = std::make_shared<point_cloud_t<T>>();
    cloud->points.push_back({0.0f, 0.0f, 0.0f});
    cloud->points.push_back({1.0f, 0.0f, 0.0f});
    cloud->points.push_back({0.0f, 1.0f, 0.0f});
    cloud->points.push_back({0.0f, 0.0f, 1.0f});
    
    std::cout << "Source cloud size: " << cloud->size() << std::endl;
    
    point_to_point_icp_t<T> icp;
    icp.set_source(cloud);
    icp.set_target(cloud);  // Same cloud - perfect alignment
    icp.set_max_iterations(1);
    icp.set_max_correspondence_distance(1.0);
    icp.set_transformation_epsilon(1e-10);  // Very small to avoid early convergence
    icp.set_record_history(true);  // Enable history recording
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "Align success: " << success << std::endl;
    std::cout << "Converged: " << result.converged << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
    std::cout << "Iterations: " << result.iterations_performed << std::endl;
    std::cout << "Termination reason: " << result.termination_reason << std::endl;
    
    REQUIRE(success);
  }
  
  SECTION("Debug correspondence finding")
  {
    // Create source cloud
    auto source = std::make_shared<point_cloud_t<T>>();
    source->points.push_back({0.0f, 0.0f, 0.0f});
    source->points.push_back({1.0f, 0.0f, 0.0f});
    source->points.push_back({0.0f, 1.0f, 0.0f});
    
    // Create target cloud with small transformation
    auto target = std::make_shared<point_cloud_t<T>>();
    target->points.push_back({0.1f, 0.1f, 0.0f});
    target->points.push_back({1.1f, 0.1f, 0.0f});
    target->points.push_back({0.1f, 1.1f, 0.0f});
    
    std::cout << "\nDebug correspondence finding:" << std::endl;
    std::cout << "Source size: " << source->size() << ", Target size: " << target->size() << std::endl;
    
    point_to_point_icp_t<T> icp;
    icp.set_source(source);
    icp.set_target(target);
    icp.set_max_iterations(5);
    icp.set_max_correspondence_distance(1.0);
    icp.set_record_history(true);
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "Align success: " << success << std::endl;
    std::cout << "Converged: " << result.converged << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
    std::cout << "Iterations: " << result.iterations_performed << std::endl;
    std::cout << "Termination reason: " << result.termination_reason << std::endl;
    
    if (!result.history.empty()) {
      std::cout << "First iteration correspondences: " << result.history[0].num_correspondences << std::endl;
    }
  }
  
  SECTION("Debug KD-tree search")
  {
    // Create a very simple case
    auto source = std::make_shared<point_cloud_t<T>>();
    auto target = std::make_shared<point_cloud_t<T>>();
    
    // Add same points to both clouds
    for (int i = 0; i < 5; ++i) {
      point_t<T> p;
      p.x = static_cast<T>(i);
      p.y = 0.0f;
      p.z = 0.0f;
      source->points.push_back(p);
      target->points.push_back(p);
    }
    
    std::cout << "\nDebug KD-tree search:" << std::endl;
    std::cout << "Source/Target size: " << source->size() << std::endl;
    
    // 直接测试KD树
    kdtree_t<T> knn_searcher;
    knn_searcher.set_input(target);
    
    std::vector<std::size_t> indices;
    std::vector<T> distances;
    knn_searcher.kneighbors(source->points[0], 1, indices, distances);
    
    std::cout << "KNN search for point (0,0,0):" << std::endl;
    std::cout << "  Found " << indices.size() << " neighbors" << std::endl;
    if (!indices.empty()) {
      std::cout << "  Nearest neighbor index: " << indices[0] << std::endl;
      std::cout << "  Distance squared: " << distances[0] << std::endl;
    }
    
    // 测试ICP
    point_to_point_icp_t<T> icp;
    icp.set_source(source);
    icp.set_target(target);
    icp.set_max_iterations(2);
    icp.set_max_correspondence_distance(1.0);
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "ICP align success: " << success << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
  }
  
  SECTION("Debug random cloud issue")
  {
    // 使用主测试相同的方法创建随机点云
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
    
    auto target = std::make_shared<point_cloud_t<T>>(*source);  // 创建副本
    
    std::cout << "\nDebug random cloud issue:" << std::endl;
    std::cout << "Source size: " << source->size() << ", Target size: " << target->size() << std::endl;
    std::cout << "First source point: (" << source->points[0].x << ", " 
              << source->points[0].y << ", " << source->points[0].z << ")" << std::endl;
    std::cout << "First target point: (" << target->points[0].x << ", " 
              << target->points[0].y << ", " << target->points[0].z << ")" << std::endl;
    
    point_to_point_icp_t<T> icp;
    icp.set_source(source);
    icp.set_target(target);
    icp.set_max_iterations(2);
    icp.set_max_correspondence_distance(1.0);
    icp.set_record_history(true);
    
    fine_registration_result_t<T> result;
    bool success = icp.align(result);
    
    std::cout << "ICP align success: " << success << std::endl;
    std::cout << "Final error: " << result.final_error << std::endl;
    std::cout << "Iterations: " << result.iterations_performed << std::endl;
    if (!result.history.empty()) {
      std::cout << "Correspondences found: " << result.history[0].num_correspondences << std::endl;
    }
  }
}
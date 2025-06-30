#include <iostream>
#include <cmath>
#include <chrono>
#include <cpp-toolbox/types/point_utils.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <Eigen/Core>

using namespace toolbox::types;

int main()
{
  // Create a simple point cloud
  point_cloud_t<float> cloud;
  cloud.points.emplace_back(1.0f, 0.0f, 0.0f);
  cloud.points.emplace_back(0.0f, 1.0f, 0.0f);
  cloud.points.emplace_back(0.0f, 0.0f, 1.0f);
  cloud.points.emplace_back(1.0f, 1.0f, 1.0f);
  
  std::cout << "Original point cloud:\n";
  for (const auto& pt : cloud.points) {
    std::cout << "  (" << pt.x << ", " << pt.y << ", " << pt.z << ")\n";
  }
  
  // Create a transformation matrix (rotation + translation)
  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
  
  // Rotate 45 degrees around Z axis
  float angle = M_PI / 4;  // 45 degrees
  transform(0, 0) = std::cos(angle);
  transform(0, 1) = -std::sin(angle);
  transform(1, 0) = std::sin(angle);
  transform(1, 1) = std::cos(angle);
  
  // Translate by (10, 20, 30)
  transform(0, 3) = 10.0f;
  transform(1, 3) = 20.0f;
  transform(2, 3) = 30.0f;
  
  std::cout << "\nTransformation matrix:\n" << transform << "\n";
  
  // Sequential transformation
  std::cout << "\nSequential transformation:\n";
  auto transformed_seq = transform_point_cloud(cloud, transform);
  for (const auto& pt : transformed_seq.points) {
    std::cout << "  (" << pt.x << ", " << pt.y << ", " << pt.z << ")\n";
  }
  
  // Parallel transformation (for larger clouds)
  std::cout << "\nParallel transformation:\n";
  auto transformed_par = transform_point_cloud_parallel(cloud, transform);
  for (const auto& pt : transformed_par.points) {
    std::cout << "  (" << pt.x << ", " << pt.y << ", " << pt.z << ")\n";
  }
  
  // In-place transformation
  std::cout << "\nIn-place transformation:\n";
  point_cloud_t<float> cloud_copy = cloud;
  transform_point_cloud_inplace(cloud_copy, transform);
  for (const auto& pt : cloud_copy.points) {
    std::cout << "  (" << pt.x << ", " << pt.y << ", " << pt.z << ")\n";
  }
  
  // Large cloud example
  std::cout << "\nLarge cloud transformation (parallel):\n";
  minmax_t<point_t<float>> bounds({0, 0, 0}, {100, 100, 100});
  auto random_points = generate_random_points_parallel<float>(10000, bounds);
  
  // Convert vector to point cloud
  point_cloud_t<float> large_cloud;
  large_cloud.points = std::move(random_points);
  
  std::cout << "Generated " << large_cloud.size() << " random points\n";
  
  auto start = std::chrono::high_resolution_clock::now();
  auto transformed_large = transform_point_cloud_parallel(large_cloud, transform);
  auto end = std::chrono::high_resolution_clock::now();
  
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "Parallel transformation of " << large_cloud.size() 
            << " points took " << duration.count() << " microseconds\n";
  
  return 0;
}
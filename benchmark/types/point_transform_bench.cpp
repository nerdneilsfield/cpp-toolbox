#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/types/point_utils.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/io/formats/pcd.hpp>

#include <Eigen/Core>
#include <cmath>
#include <random>

// 基准测试数据目录
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "test/data"
#endif

using namespace toolbox::types;
using namespace toolbox::io;

// Helper to create a transformation matrix with rotation and translation
template<typename T>
Eigen::Matrix<T, 4, 4> create_test_transform()
{
  Eigen::Matrix<T, 4, 4> transform = Eigen::Matrix<T, 4, 4>::Identity();
  
  // Rotation around Z axis (30 degrees)
  T angle = M_PI / 6;
  transform(0, 0) = std::cos(angle);
  transform(0, 1) = -std::sin(angle);
  transform(1, 0) = std::sin(angle);
  transform(1, 1) = std::cos(angle);
  
  // Rotation around X axis (15 degrees)
  T angle_x = M_PI / 12;
  Eigen::Matrix<T, 3, 3> rot_x;
  rot_x << 1, 0, 0,
           0, std::cos(angle_x), -std::sin(angle_x),
           0, std::sin(angle_x), std::cos(angle_x);
  
  transform.template block<3, 3>(0, 0) = rot_x * transform.template block<3, 3>(0, 0);
  
  // Translation
  transform(0, 3) = 10.5;
  transform(1, 3) = -5.3;
  transform(2, 3) = 2.8;
  
  return transform;
}

// Helper to generate large synthetic point cloud
template<typename T>
point_cloud_t<T> generate_large_cloud(std::size_t num_points)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);
  
  std::mt19937 gen(42);  // Fixed seed for reproducibility
  std::uniform_real_distribution<T> dist(-100.0, 100.0);
  std::normal_distribution<T> noise(0.0, 0.1);
  
  // Generate points in various patterns
  for (std::size_t i = 0; i < num_points; ++i) {
    T t = static_cast<T>(i) / static_cast<T>(num_points);
    
    if (i % 3 == 0) {
      // Spiral pattern
      T radius = 10.0 + t * 50.0;
      T theta = t * 20 * M_PI;
      cloud.points.emplace_back(
          radius * std::cos(theta) + noise(gen),
          radius * std::sin(theta) + noise(gen),
          t * 50.0 + noise(gen)
      );
    } else if (i % 3 == 1) {
      // Grid pattern
      T x = std::floor(t * 100) - 50;
      T y = (t * 10000) - std::floor(t * 100) * 100 - 50;
      cloud.points.emplace_back(
          x + noise(gen),
          y + noise(gen),
          std::sin(x * 0.1) * std::cos(y * 0.1) * 10 + noise(gen)
      );
    } else {
      // Random points
      cloud.points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
  }
  
  return cloud;
}

TEST_CASE("Point cloud transformation benchmarks", "[benchmark][types][transform]")
{
  // Create transformation matrix
  auto transform_f = create_test_transform<float>();
  auto transform_d = create_test_transform<double>();
  
  SECTION("Small synthetic cloud (1K points)")
  {
    auto cloud_f = generate_large_cloud<float>(1000);
    auto cloud_d = generate_large_cloud<double>(1000);
    
    BENCHMARK("Sequential transform (float, 1K)") {
      return transform_point_cloud(cloud_f, transform_f);
    };
    
    BENCHMARK("Sequential transform in-place (float, 1K)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Parallel transform (float, 1K)") {
      return transform_point_cloud_parallel(cloud_f, transform_f);
    };
    
    BENCHMARK("Parallel transform in-place (float, 1K)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
      return cloud_copy;
    };
  }
  
  SECTION("Medium synthetic cloud (100K points)")
  {
    auto cloud_f = generate_large_cloud<float>(100000);
    auto cloud_d = generate_large_cloud<double>(100000);
    
    BENCHMARK("Sequential transform (float, 100K)") {
      return transform_point_cloud(cloud_f, transform_f);
    };
    
    BENCHMARK("Sequential transform in-place (float, 100K)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Parallel transform (float, 100K)") {
      return transform_point_cloud_parallel(cloud_f, transform_f);
    };
    
    BENCHMARK("Parallel transform in-place (float, 100K)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Sequential transform (double, 100K)") {
      return transform_point_cloud(cloud_d, transform_d);
    };
    
    BENCHMARK("Parallel transform (double, 100K)") {
      return transform_point_cloud_parallel(cloud_d, transform_d);
    };
  }
  
  SECTION("Large synthetic cloud (1M points)")
  {
    auto cloud_f = generate_large_cloud<float>(1000000);
    
    BENCHMARK("Sequential transform (float, 1M)") {
      return transform_point_cloud(cloud_f, transform_f);
    };
    
    BENCHMARK("Sequential transform in-place (float, 1M)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Parallel transform (float, 1M)") {
      return transform_point_cloud_parallel(cloud_f, transform_f);
    };
    
    BENCHMARK("Parallel transform in-place (float, 1M)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
      return cloud_copy;
    };
  }
  
  SECTION("Very large synthetic cloud (10M points)")
  {
    auto cloud_f = generate_large_cloud<float>(10000000);
    
    BENCHMARK("Sequential transform (float, 10M)") {
      return transform_point_cloud(cloud_f, transform_f);
    };
    
    BENCHMARK("Sequential transform in-place (float, 10M)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Parallel transform (float, 10M)") {
      return transform_point_cloud_parallel(cloud_f, transform_f);
    };
    
    BENCHMARK("Parallel transform in-place (float, 10M)") {
      auto cloud_copy = cloud_f;
      transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
      return cloud_copy;
    };
  }
}

TEST_CASE("Real point cloud transformation benchmarks", "[benchmark][types][transform][kitti]")
{
  auto transform_f = create_test_transform<float>();
  
  SECTION("KITTI dataset point cloud")
  {
    // Load KITTI point cloud
    std::string kitti_file = std::string(TEST_DATA_DIR) + "/000000.bin";
    point_cloud_t<float> kitti_cloud;
    
    try {
      auto loaded = read_kitti_bin<float>(kitti_file);
      if (loaded) {
        kitti_cloud = *loaded;
        
        INFO("Loaded KITTI cloud with " << kitti_cloud.size() << " points");
        
        BENCHMARK("KITTI Sequential transform") {
          return transform_point_cloud(kitti_cloud, transform_f);
        };
        
        BENCHMARK("KITTI Sequential transform in-place") {
          auto cloud_copy = kitti_cloud;
          transform_point_cloud_inplace(cloud_copy, transform_f);
          return cloud_copy;
        };
        
        BENCHMARK("KITTI Parallel transform") {
          return transform_point_cloud_parallel(kitti_cloud, transform_f);
        };
        
        BENCHMARK("KITTI Parallel transform in-place") {
          auto cloud_copy = kitti_cloud;
          transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
          return cloud_copy;
        };
      }
    } catch (const std::exception& e) {
      WARN("Could not load KITTI dataset: " << e.what());
    }
  }
  
  SECTION("PCD format point cloud") 
  {
    // Try to load a PCD file if available
    std::string pcd_file = std::string(TEST_DATA_DIR) + "/bunny.pcd";
    point_cloud_t<float> pcd_cloud;
    
    try {
      auto loaded = read_pcd<float>(pcd_file);
      if (loaded) {
        pcd_cloud = *loaded;
        
        INFO("Loaded PCD cloud with " << pcd_cloud.size() << " points");
        
        BENCHMARK("PCD Sequential transform") {
          return transform_point_cloud(pcd_cloud, transform_f);
        };
        
        BENCHMARK("PCD Sequential transform in-place") {
          auto cloud_copy = pcd_cloud;
          transform_point_cloud_inplace(cloud_copy, transform_f);
          return cloud_copy;
        };
        
        BENCHMARK("PCD Parallel transform") {
          return transform_point_cloud_parallel(pcd_cloud, transform_f);
        };
        
        BENCHMARK("PCD Parallel transform in-place") {
          auto cloud_copy = pcd_cloud;
          transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
          return cloud_copy;
        };
      }
    } catch (const std::exception& e) {
      WARN("Could not load PCD file: " << e.what());
    }
  }
}

TEST_CASE("Transformation scaling benchmarks", "[benchmark][types][transform][scaling]")
{
  auto transform_f = create_test_transform<float>();
  
  // Test how performance scales with cloud size
  std::vector<std::size_t> sizes = {1000, 5000, 10000, 50000, 100000, 500000, 1000000};
  
  for (auto size : sizes) {
    auto cloud = generate_large_cloud<float>(size);
    
    std::string size_str = std::to_string(size / 1000) + "K";
    if (size >= 1000000) {
      size_str = std::to_string(size / 1000000) + "M";
    }
    
    BENCHMARK("Sequential " + size_str + " points") {
      return transform_point_cloud(cloud, transform_f);
    };
    
    BENCHMARK("Parallel " + size_str + " points") {
      return transform_point_cloud_parallel(cloud, transform_f);
    };
    
    BENCHMARK("Sequential in-place " + size_str + " points") {
      auto cloud_copy = cloud;
      transform_point_cloud_inplace(cloud_copy, transform_f);
      return cloud_copy;
    };
    
    BENCHMARK("Parallel in-place " + size_str + " points") {
      auto cloud_copy = cloud;
      transform_point_cloud_inplace_parallel(cloud_copy, transform_f);
      return cloud_copy;
    };
  }
}

TEST_CASE("Memory efficiency benchmarks", "[benchmark][types][transform][memory]")
{
  // Test memory allocation patterns
  SECTION("Memory allocation overhead")
  {
    // Pre-generate clouds to exclude generation time
    auto small_cloud = generate_large_cloud<float>(10000);
    auto medium_cloud = generate_large_cloud<float>(100000);
    auto large_cloud = generate_large_cloud<float>(1000000);
    
    auto transform = create_test_transform<float>();
    
    BENCHMARK("New allocation - 10K points") {
      return transform_point_cloud(small_cloud, transform);
    };
    
    BENCHMARK("In-place - 10K points") {
      auto cloud_copy = small_cloud;
      transform_point_cloud_inplace(cloud_copy, transform);
      return cloud_copy;
    };
    
    BENCHMARK("New allocation - 100K points") {
      return transform_point_cloud(medium_cloud, transform);
    };
    
    BENCHMARK("In-place - 100K points") {
      auto cloud_copy = medium_cloud;
      transform_point_cloud_inplace(cloud_copy, transform);
      return cloud_copy;
    };
    
    BENCHMARK("New allocation - 1M points") {
      return transform_point_cloud(large_cloud, transform);
    };
    
    BENCHMARK("In-place - 1M points") {
      auto cloud_copy = large_cloud;
      transform_point_cloud_inplace(cloud_copy, transform);
      return cloud_copy;
    };
  }
}
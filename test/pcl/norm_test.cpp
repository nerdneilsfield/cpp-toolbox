#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace Catch::Matchers;

// Helper function to generate random point cloud
template<typename T>
point_cloud_t<T> generate_random_cloud(std::size_t num_points, T min_val = -10, T max_val = 10)
{
  point_cloud_t<T> cloud;
  cloud.reserve(num_points);

  toolbox::utils::random_t rng;
  
  for (std::size_t i = 0; i < num_points; ++i)
  {
    point_t<T> pt;
    pt.x = rng.random<T>(min_val, max_val);
    pt.y = rng.random<T>(min_val, max_val);
    pt.z = rng.random<T>(min_val, max_val);
    cloud.points.push_back(pt);
  }

  return cloud;
}

// Helper function to create a planar point cloud with known normal
template<typename T>
point_cloud_t<T> create_planar_cloud(const point_t<T>& normal, const point_t<T>& center, 
                                     std::size_t num_points, T extent = 5.0)
{
  point_cloud_t<T> cloud;
  cloud.reserve(num_points);

  toolbox::utils::random_t rng;
  
  // Create two orthogonal vectors in the plane
  point_t<T> u, v;
  
  // Find a vector that's not parallel to normal
  if (std::abs(normal.x) < 0.9) {
    u = point_t<T>(1, 0, 0);
  } else {
    u = point_t<T>(0, 1, 0);
  }
  
  // Make u orthogonal to normal using Gram-Schmidt
  T dot_product = u.x * normal.x + u.y * normal.y + u.z * normal.z;
  u.x -= dot_product * normal.x;
  u.y -= dot_product * normal.y;
  u.z -= dot_product * normal.z;
  
  // Normalize u
  T u_norm = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
  u.x /= u_norm;
  u.y /= u_norm;
  u.z /= u_norm;
  
  // Create v = normal x u
  v.x = normal.y * u.z - normal.z * u.y;
  v.y = normal.z * u.x - normal.x * u.z;
  v.z = normal.x * u.y - normal.y * u.x;

  for (std::size_t i = 0; i < num_points; ++i)
  {
    T s = rng.random<T>(-extent, extent);
    T t = rng.random<T>(-extent, extent);
    
    point_t<T> pt;
    pt.x = center.x + s * u.x + t * v.x;
    pt.y = center.y + s * u.y + t * v.y;
    pt.z = center.z + s * u.z + t * v.z;
    
    cloud.points.push_back(pt);
  }

  return cloud;
}

// Helper function to compute angle between two vectors
template<typename T>
T compute_angle(const point_t<T>& v1, const point_t<T>& v2)
{
  T dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  T norm1 = std::sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
  T norm2 = std::sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
  
  if (norm1 < 1e-10 || norm2 < 1e-10) return 0;
  
  T cos_angle = dot / (norm1 * norm2);
  cos_angle = std::clamp(cos_angle, T(-1), T(1));
  
  return std::acos(std::abs(cos_angle));  // Use abs to handle normal direction ambiguity
}

TEST_CASE("[pcl][norm] PCA Normal Estimation Basic Functionality", "[pcl][norm]")
{
  using data_type = float;
  
  SECTION("Test with KDTree KNN")
  {
    auto cloud = generate_random_cloud<data_type>(100);
    auto knn = kdtree_t<data_type>{};
    
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    
    auto result = norm_extractor.extract();
    
    REQUIRE(result.points.size() == cloud.size());
    REQUIRE(result.normals.size() == cloud.size());
    
    // Check that normals are approximately unit vectors
    for (const auto& normal : result.normals) {
      auto norm = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
      REQUIRE_THAT(norm, WithinRel(1.0f, 0.1f));
    }
  }
  
  SECTION("Test with Brute Force KNN")
  {
    auto cloud = generate_random_cloud<data_type>(50);
    auto knn = bfknn_t<data_type>{};
    
    pca_norm_extractor_t<data_type, bfknn_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(5);
    
    auto result = norm_extractor.extract();
    
    REQUIRE(result.points.size() == cloud.size());
    REQUIRE(result.normals.size() == cloud.size());
    
    // Check that normals are approximately unit vectors
    for (const auto& normal : result.normals) {
      auto norm = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
      REQUIRE_THAT(norm, WithinRel(1.0f, 0.1f));
    }
  }
}

TEST_CASE("[pcl][norm] PCA Normal Estimation Accuracy", "[pcl][norm]")
{
  using data_type = double;
  
  SECTION("Test with planar point cloud")
  {
    // Create a planar point cloud with known normal (0, 0, 1)
    point_t<data_type> known_normal(0, 0, 1);
    point_t<data_type> center(0, 0, 0);
    auto cloud = create_planar_cloud(known_normal, center, 200, 10.0);
    
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(15);
    
    auto result = norm_extractor.extract();
    
    REQUIRE(result.normals.size() == cloud.size());
    
    // Check that computed normals are close to the known normal
    data_type total_angle_error = 0;
    std::size_t valid_normals = 0;
    
    for (const auto& computed_normal : result.normals) {
      auto angle_error = compute_angle(known_normal, computed_normal);
      if (angle_error < M_PI / 4) {  // Exclude outliers
        total_angle_error += angle_error;
        valid_normals++;
      }
    }
    
    if (valid_normals > 0) {
      data_type mean_angle_error = total_angle_error / valid_normals;
      REQUIRE(mean_angle_error < 0.2);  // Less than ~11 degrees error on average
    }
  }
  
  SECTION("Test with multiple planar orientations")
  {
    std::vector<point_t<data_type>> test_normals = {
      {1, 0, 0},
      {0, 1, 0}, 
      {0, 0, 1},
      {0.577, 0.577, 0.577}  // Diagonal
    };
    
    for (const auto& known_normal : test_normals) {
      auto cloud = create_planar_cloud(known_normal, point_t<data_type>(0, 0, 0), 100, 5.0);
      
      auto knn = kdtree_t<data_type>{};
      pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
      norm_extractor.set_input(cloud);
      norm_extractor.set_knn(knn);
      norm_extractor.set_num_neighbors(10);
      
      auto result = norm_extractor.extract();
      
      // Check average angle error
      data_type total_angle_error = 0;
      std::size_t valid_normals = 0;
      
      for (const auto& computed_normal : result.normals) {
        auto angle_error = compute_angle(known_normal, computed_normal);
        if (angle_error < M_PI / 4) {  // Exclude outliers
          total_angle_error += angle_error;
          valid_normals++;
        }
      }
      
      if (valid_normals > 0) {
        data_type mean_angle_error = total_angle_error / valid_normals;
        REQUIRE(mean_angle_error < 0.3);  // Less than ~17 degrees error
      }
    }
  }
}

TEST_CASE("[pcl][norm] PCA Normal Estimation Parallel Processing", "[pcl][norm]")
{
  using data_type = float;
  
  auto cloud = generate_random_cloud<data_type>(500);
  auto knn1 = kdtree_t<data_type>{};
  auto knn2 = kdtree_t<data_type>{};
  
  // Sequential processing
  pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor_seq;
  norm_extractor_seq.set_input(cloud);
  norm_extractor_seq.set_knn(knn1);
  norm_extractor_seq.set_num_neighbors(10);
  norm_extractor_seq.enable_parallel(false);
  
  auto result_seq = norm_extractor_seq.extract();
  
  // Parallel processing
  pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor_par;
  norm_extractor_par.set_input(cloud);
  norm_extractor_par.set_knn(knn2);
  norm_extractor_par.set_num_neighbors(10);
  norm_extractor_par.enable_parallel(true);
  
  auto result_par = norm_extractor_par.extract();
  
  REQUIRE(result_seq.normals.size() == result_par.normals.size());
  
  // Results should be approximately the same (within numerical precision)
  for (std::size_t i = 0; i < result_seq.normals.size(); ++i) {
    auto angle_diff = compute_angle(result_seq.normals[i], result_par.normals[i]);
    REQUIRE(angle_diff < 0.1);  // Very small difference expected
  }
}

TEST_CASE("[pcl][norm] PCA Normal Estimation Edge Cases", "[pcl][norm]")
{
  using data_type = float;
  
  SECTION("Empty point cloud")
  {
    point_cloud_t<data_type> empty_cloud;
    auto knn = kdtree_t<data_type>{};
    
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(empty_cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(5);
    
    auto result = norm_extractor.extract();
    REQUIRE(result.points.empty());
    REQUIRE(result.normals.empty());
  }
  
  SECTION("Single point")
  {
    point_cloud_t<data_type> cloud;
    cloud.points.push_back(point_t<data_type>(1, 2, 3));
    
    auto knn = kdtree_t<data_type>{};
    
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(5);
    
    auto result = norm_extractor.extract();
    REQUIRE(result.points.size() == 1);
    REQUIRE(result.normals.size() == 1);
    // Should have default normal (0, 0, 1)
    REQUIRE_THAT(result.normals[0].z, WithinRel(1.0f, 0.01f));
  }
  
  SECTION("Insufficient neighbors")
  {
    point_cloud_t<data_type> cloud;
    cloud.points.push_back(point_t<data_type>(1, 2, 3));
    cloud.points.push_back(point_t<data_type>(1, 2, 4));
    
    auto knn = kdtree_t<data_type>{};
    
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);  // More neighbors than points
    
    auto result = norm_extractor.extract();
    REQUIRE(result.points.size() == 2);
    REQUIRE(result.normals.size() == 2);
  }
}

TEST_CASE("[pcl][norm] PCA Normal Estimation Benchmarks", "[pcl][norm][!benchmark]")
{
  using data_type = float;
  
  auto cloud_small = generate_random_cloud<data_type>(1000);
  auto cloud_medium = generate_random_cloud<data_type>(5000);
  auto cloud_large = generate_random_cloud<data_type>(10000);
  
  BENCHMARK("PCA Normal - Small Cloud (1K points) - KDTree")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_small);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(false);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Small Cloud (1K points) - KDTree Parallel")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_small);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(true);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Medium Cloud (5K points) - KDTree")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_medium);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(false);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Medium Cloud (5K points) - KDTree Parallel")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_medium);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(true);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Large Cloud (10K points) - KDTree")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_large);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(false);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Large Cloud (10K points) - KDTree Parallel")
  {
    auto knn = kdtree_t<data_type>{};
    pca_norm_extractor_t<data_type, kdtree_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_large);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(true);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Large Cloud (10K points) - BruteForce")
  {
    auto knn = bfknn_t<data_type>{};
    pca_norm_extractor_t<data_type, bfknn_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_large);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(false);
    
    return norm_extractor.extract();
  };
  
  BENCHMARK("PCA Normal - Large Cloud (10K points) - BruteForce Parallel")
  {
    auto knn = bfknn_parallel_t<data_type>{};
    pca_norm_extractor_t<data_type, bfknn_parallel_t<data_type>> norm_extractor;
    norm_extractor.set_input(cloud_large);
    norm_extractor.set_knn(knn);
    norm_extractor.set_num_neighbors(10);
    norm_extractor.enable_parallel(false);  // KNN itself is parallel
    
    return norm_extractor.extract();
  };
}
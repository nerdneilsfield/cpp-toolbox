#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

#include <iostream>
#include <memory>
#include <iomanip>
#include <cmath>

using namespace toolbox::pcl;
using namespace toolbox::types;

// Helper function to generate random point cloud
template<typename T>
point_cloud_t<T> generate_benchmark_cloud(std::size_t num_points, T min_val = -100, T max_val = 100)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

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

// Helper function to create a planar point cloud for more realistic benchmarking
template<typename T>
point_cloud_t<T> generate_planar_benchmark_cloud(std::size_t num_points, T extent = 50.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

  toolbox::utils::random_t rng;
  
  // Generate points on a plane with some noise
  for (std::size_t i = 0; i < num_points; ++i)
  {
    T x = rng.random<T>(-extent, extent);
    T y = rng.random<T>(-extent, extent);
    T z = rng.random<T>(-2.0, 2.0);  // Small noise in z direction
    
    cloud.points.emplace_back(x, y, z);
  }

  return cloud;
}

// Helper function to generate a spherical point cloud
template<typename T>
point_cloud_t<T> generate_spherical_benchmark_cloud(std::size_t num_points, T radius = 50.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

  toolbox::utils::random_t rng;
  
  for (std::size_t i = 0; i < num_points; ++i)
  {
    // Generate random point on sphere surface
    T theta = rng.random<T>(0, 2 * M_PI);
    T phi = rng.random<T>(0, M_PI);
    
    T x = radius * std::sin(phi) * std::cos(theta);
    T y = radius * std::sin(phi) * std::sin(theta);
    T z = radius * std::cos(phi);
    
    cloud.points.emplace_back(x, y, z);
  }

  return cloud;
}

template<typename DataType, typename KNN>
void benchmark_norm_computation(const std::string& test_name,
                               const point_cloud_t<DataType>& cloud,
                               KNN& knn,
                               std::size_t num_neighbors,
                               bool enable_parallel)
{
  pca_norm_extractor_t<DataType, KNN> norm_extractor;
  norm_extractor.set_input(cloud);
  norm_extractor.set_knn(knn);
  norm_extractor.set_num_neighbors(num_neighbors);
  norm_extractor.enable_parallel(enable_parallel);

  std::string parallel_suffix = enable_parallel ? " (Parallel)" : " (Sequential)";
  
  BENCHMARK(test_name + parallel_suffix)
  {
    return norm_extractor.extract();
  };
}

TEST_CASE("PCA Normal Estimation Parallel vs Sequential Benchmarks", "[benchmark][pcl][norm]")
{
  using data_type = float;
  constexpr std::size_t num_neighbors = 15;

  SECTION("Small Point Cloud (1K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(1000);
    INFO("Testing with " << cloud.size() << " points, " << num_neighbors << " neighbors");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("KDTree Small Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("KDTree Small Cloud", cloud, kdtree2, num_neighbors, true);
  }

  SECTION("Medium Point Cloud (5K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(5000);
    INFO("Testing with " << cloud.size() << " points, " << num_neighbors << " neighbors");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("KDTree Medium Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("KDTree Medium Cloud", cloud, kdtree2, num_neighbors, true);
  }

  SECTION("Large Point Cloud (10K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    INFO("Testing with " << cloud.size() << " points, " << num_neighbors << " neighbors");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("KDTree Large Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("KDTree Large Cloud", cloud, kdtree2, num_neighbors, true);
  }

  SECTION("Very Large Point Cloud (25K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(25000);
    INFO("Testing with " << cloud.size() << " points, " << num_neighbors << " neighbors");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("KDTree Very Large Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("KDTree Very Large Cloud", cloud, kdtree2, num_neighbors, true);
  }
}

TEST_CASE("PCA Normal Estimation - Different Point Cloud Types", "[benchmark][pcl][norm]")
{
  using data_type = float;
  constexpr std::size_t num_neighbors = 12;
  constexpr std::size_t num_points = 8000;

  SECTION("Random Point Cloud")
  {
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    INFO("Testing random cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("Random Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("Random Cloud", cloud, kdtree2, num_neighbors, true);
  }

  SECTION("Planar Point Cloud")
  {
    auto cloud = generate_planar_benchmark_cloud<data_type>(num_points);
    INFO("Testing planar cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("Planar Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("Planar Cloud", cloud, kdtree2, num_neighbors, true);
  }

  SECTION("Spherical Point Cloud")
  {
    auto cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
    INFO("Testing spherical cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    benchmark_norm_computation("Spherical Cloud", cloud, kdtree1, num_neighbors, false);
    benchmark_norm_computation("Spherical Cloud", cloud, kdtree2, num_neighbors, true);
  }
}

TEST_CASE("PCA Normal Estimation - KNN Algorithm Comparison", "[benchmark][pcl][norm]")
{
  using data_type = float;
  constexpr std::size_t num_neighbors = 10;
  constexpr std::size_t num_points = 5000;

  auto cloud = generate_benchmark_cloud<data_type>(num_points);
  INFO("Comparing KNN algorithms with " << cloud.size() << " points");

  SECTION("KDTree vs Brute Force - Sequential")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    benchmark_norm_computation("KDTree", cloud, kdtree, num_neighbors, false);
    benchmark_norm_computation("Brute Force", cloud, bfknn, num_neighbors, false);
  }

  SECTION("KDTree vs Brute Force - Parallel")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn_parallel = bfknn_parallel_t<data_type>{};

    benchmark_norm_computation("KDTree", cloud, kdtree, num_neighbors, true);
    benchmark_norm_computation("Brute Force Parallel", cloud, bfknn_parallel, num_neighbors, false);
  }
}

TEST_CASE("PCA Normal Estimation - Neighbor Count Impact", "[benchmark][pcl][norm]")
{
  using data_type = float;
  constexpr std::size_t num_points = 6000;

  auto cloud = generate_benchmark_cloud<data_type>(num_points);
  INFO("Testing neighbor count impact with " << cloud.size() << " points");

  SECTION("Sequential - Different Neighbor Counts")
  {
    auto kdtree_5 = kdtree_t<data_type>{};
    auto kdtree_10 = kdtree_t<data_type>{};
    auto kdtree_20 = kdtree_t<data_type>{};
    auto kdtree_30 = kdtree_t<data_type>{};

    benchmark_norm_computation("5 Neighbors", cloud, kdtree_5, 5, false);
    benchmark_norm_computation("10 Neighbors", cloud, kdtree_10, 10, false);
    benchmark_norm_computation("20 Neighbors", cloud, kdtree_20, 20, false);
    benchmark_norm_computation("30 Neighbors", cloud, kdtree_30, 30, false);
  }

  SECTION("Parallel - Different Neighbor Counts")
  {
    auto kdtree_5 = kdtree_t<data_type>{};
    auto kdtree_10 = kdtree_t<data_type>{};
    auto kdtree_20 = kdtree_t<data_type>{};
    auto kdtree_30 = kdtree_t<data_type>{};

    benchmark_norm_computation("5 Neighbors", cloud, kdtree_5, 5, true);
    benchmark_norm_computation("10 Neighbors", cloud, kdtree_10, 10, true);
    benchmark_norm_computation("20 Neighbors", cloud, kdtree_20, 20, true);
    benchmark_norm_computation("30 Neighbors", cloud, kdtree_30, 30, true);
  }
}

TEST_CASE("PCA Normal Estimation - Parallel Speedup Analysis", "[benchmark][pcl][norm]")
{
  using data_type = float;
  constexpr std::size_t num_neighbors = 15;
  
  // Test with different cloud sizes to analyze scaling
  std::vector<std::size_t> cloud_sizes = {2000, 4000, 8000, 15000, 30000};

  for (auto size : cloud_sizes) {
    DYNAMIC_SECTION("Cloud Size: " << size << " points") {
      auto cloud = generate_benchmark_cloud<data_type>(size);
      INFO("Analyzing parallel speedup with " << cloud.size() << " points");

      auto kdtree1 = kdtree_t<data_type>{};
      auto kdtree2 = kdtree_t<data_type>{};

      std::string size_label = "Size " + std::to_string(size);
      benchmark_norm_computation(size_label, cloud, kdtree1, num_neighbors, false);
      benchmark_norm_computation(size_label, cloud, kdtree2, num_neighbors, true);
    }
  }
}
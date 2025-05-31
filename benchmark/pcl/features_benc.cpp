#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/features/features.hpp>
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

// Helper function to generate a corner-like test cloud
template<typename T>
point_cloud_t<T> generate_corner_test_cloud(std::size_t num_points, T extent = 50.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

  toolbox::utils::random_t rng;
  
  // Generate corner-like structure with three planes meeting at origin
  for (std::size_t i = 0; i < num_points; ++i)
  {
    T choice = rng.random<T>(0, 3);
    T noise = static_cast<T>(0.5);
    
    point_t<T> pt;
    if (choice < 1) {
      // XY plane
      pt.x = rng.random<T>(0, extent);
      pt.y = rng.random<T>(0, extent);
      pt.z = rng.random<T>(-noise, noise);
    } else if (choice < 2) {
      // XZ plane
      pt.x = rng.random<T>(0, extent);
      pt.y = rng.random<T>(-noise, noise);
      pt.z = rng.random<T>(0, extent);
    } else {
      // YZ plane
      pt.x = rng.random<T>(-noise, noise);
      pt.y = rng.random<T>(0, extent);
      pt.z = rng.random<T>(0, extent);
    }
    
    cloud.points.push_back(pt);
  }

  return cloud;
}

template<typename DataType, typename KNN, typename Extractor>
void benchmark_feature_extraction(const std::string& test_name,
                                 const point_cloud_t<DataType>& cloud,
                                 KNN& knn,
                                 Extractor& extractor,
                                 bool enable_parallel)
{
  extractor.set_input(cloud);
  extractor.set_knn(knn);
  extractor.enable_parallel(enable_parallel);

  std::string parallel_suffix = enable_parallel ? " (Parallel)" : " (Sequential)";
  
  BENCHMARK(test_name + parallel_suffix)
  {
    return extractor.extract().size();
  };
}

TEST_CASE("Curvature Keypoint Extraction Parallel vs Sequential Benchmarks", "[benchmark][pcl][features][curvature]")
{
  using data_type = float;
  constexpr data_type search_radius = 2.0f;
  constexpr data_type curvature_threshold = 0.01f;

  SECTION("Small Point Cloud (1K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(1000);
    INFO("Testing with " << cloud.size() << " points, radius " << search_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_search_radius(search_radius);
    extractor1.set_curvature_threshold(curvature_threshold);

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_search_radius(search_radius);
    extractor2.set_curvature_threshold(curvature_threshold);

    benchmark_feature_extraction("Curvature Small Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("Curvature Small Cloud", cloud, kdtree2, extractor2, true);
  }

  SECTION("Medium Point Cloud (5K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(5000);
    INFO("Testing with " << cloud.size() << " points, radius " << search_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_search_radius(search_radius);
    extractor1.set_curvature_threshold(curvature_threshold);

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_search_radius(search_radius);
    extractor2.set_curvature_threshold(curvature_threshold);

    benchmark_feature_extraction("Curvature Medium Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("Curvature Medium Cloud", cloud, kdtree2, extractor2, true);
  }

  SECTION("Large Point Cloud (10K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    INFO("Testing with " << cloud.size() << " points, radius " << search_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_search_radius(search_radius);
    extractor1.set_curvature_threshold(curvature_threshold);

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_search_radius(search_radius);
    extractor2.set_curvature_threshold(curvature_threshold);

    benchmark_feature_extraction("Curvature Large Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("Curvature Large Cloud", cloud, kdtree2, extractor2, true);
  }

  SECTION("Very Large Point Cloud (25K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(25000);
    INFO("Testing with " << cloud.size() << " points, radius " << search_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_search_radius(search_radius);
    extractor1.set_curvature_threshold(curvature_threshold);

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_search_radius(search_radius);
    extractor2.set_curvature_threshold(curvature_threshold);

    benchmark_feature_extraction("Curvature Very Large Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("Curvature Very Large Cloud", cloud, kdtree2, extractor2, true);
  }
}

TEST_CASE("ISS Keypoint Extraction Parallel vs Sequential Benchmarks", "[benchmark][pcl][features][iss]")
{
  using data_type = float;
  constexpr data_type salient_radius = 3.0f;
  constexpr data_type threshold21 = 0.9f;
  constexpr data_type threshold32 = 0.9f;

  SECTION("Small Point Cloud (1K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(1000);
    INFO("Testing with " << cloud.size() << " points, salient radius " << salient_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_salient_radius(salient_radius);
    extractor1.set_threshold21(threshold21);
    extractor1.set_threshold32(threshold32);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_salient_radius(salient_radius);
    extractor2.set_threshold21(threshold21);
    extractor2.set_threshold32(threshold32);

    benchmark_feature_extraction("ISS Small Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("ISS Small Cloud", cloud, kdtree2, extractor2, true);
  }

  SECTION("Medium Point Cloud (5K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(5000);
    INFO("Testing with " << cloud.size() << " points, salient radius " << salient_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_salient_radius(salient_radius);
    extractor1.set_threshold21(threshold21);
    extractor1.set_threshold32(threshold32);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_salient_radius(salient_radius);
    extractor2.set_threshold21(threshold21);
    extractor2.set_threshold32(threshold32);

    benchmark_feature_extraction("ISS Medium Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("ISS Medium Cloud", cloud, kdtree2, extractor2, true);
  }

  SECTION("Large Point Cloud (10K points)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    INFO("Testing with " << cloud.size() << " points, salient radius " << salient_radius);

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_salient_radius(salient_radius);
    extractor1.set_threshold21(threshold21);
    extractor1.set_threshold32(threshold32);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_salient_radius(salient_radius);
    extractor2.set_threshold21(threshold21);
    extractor2.set_threshold32(threshold32);

    benchmark_feature_extraction("ISS Large Cloud", cloud, kdtree1, extractor1, false);
    benchmark_feature_extraction("ISS Large Cloud", cloud, kdtree2, extractor2, true);
  }
}

TEST_CASE("Feature Extraction - Different Point Cloud Types", "[benchmark][pcl][features][geometry]")
{
  using data_type = float;
  constexpr std::size_t num_points = 8000;

  SECTION("Random Point Cloud - Curvature vs ISS")
  {
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    INFO("Testing random cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.01f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(3.0f);
    iss_extractor.set_threshold21(0.9f);
    iss_extractor.set_threshold32(0.9f);

    benchmark_feature_extraction("Random Cloud Curvature", cloud, kdtree1, curvature_extractor, false);
    benchmark_feature_extraction("Random Cloud ISS", cloud, kdtree2, iss_extractor, false);
  }

  SECTION("Planar Point Cloud - Curvature vs ISS")
  {
    auto cloud = generate_planar_benchmark_cloud<data_type>(num_points);
    INFO("Testing planar cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.005f);  // Lower threshold for planar

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(3.0f);
    iss_extractor.set_threshold21(0.85f);  // More lenient for planar
    iss_extractor.set_threshold32(0.85f);

    benchmark_feature_extraction("Planar Cloud Curvature", cloud, kdtree1, curvature_extractor, false);
    benchmark_feature_extraction("Planar Cloud ISS", cloud, kdtree2, iss_extractor, false);
  }

  SECTION("Spherical Point Cloud - Curvature vs ISS")
  {
    auto cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
    INFO("Testing spherical cloud with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(3.0f);
    curvature_extractor.set_curvature_threshold(0.02f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(4.0f);
    iss_extractor.set_threshold21(0.85f);
    iss_extractor.set_threshold32(0.85f);

    benchmark_feature_extraction("Spherical Cloud Curvature", cloud, kdtree1, curvature_extractor, false);
    benchmark_feature_extraction("Spherical Cloud ISS", cloud, kdtree2, iss_extractor, false);
  }
}

TEST_CASE("Feature Extraction - KNN Algorithm Comparison", "[benchmark][pcl][features][knn]")
{
  using data_type = float;
  constexpr std::size_t num_points = 5000;

  auto cloud = generate_benchmark_cloud<data_type>(num_points);
  INFO("Comparing KNN algorithms with " << cloud.size() << " points");

  SECTION("Curvature Extraction - KDTree vs Brute Force")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_search_radius(2.0f);
    extractor_kd.set_curvature_threshold(0.01f);

    curvature_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_search_radius(2.0f);
    extractor_bf.set_curvature_threshold(0.01f);

    benchmark_feature_extraction("Curvature KDTree", cloud, kdtree, extractor_kd, false);
    benchmark_feature_extraction("Curvature Brute Force", cloud, bfknn, extractor_bf, false);
  }

  SECTION("ISS Extraction - KDTree vs Brute Force")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_salient_radius(3.0f);
    extractor_kd.set_threshold21(0.9f);
    extractor_kd.set_threshold32(0.9f);

    iss_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_salient_radius(3.0f);
    extractor_bf.set_threshold21(0.9f);
    extractor_bf.set_threshold32(0.9f);

    benchmark_feature_extraction("ISS KDTree", cloud, kdtree, extractor_kd, false);
    benchmark_feature_extraction("ISS Brute Force", cloud, bfknn, extractor_bf, false);
  }

  SECTION("Harris3D Extraction - KDTree vs Brute Force")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_threshold(0.001f);
    extractor_kd.set_num_neighbors(30);
    extractor_kd.set_suppression_radius(2.0f);

    harris3d_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_threshold(0.001f);
    extractor_bf.set_num_neighbors(30);
    extractor_bf.set_suppression_radius(2.0f);

    benchmark_feature_extraction("Harris3D KDTree", cloud, kdtree, extractor_kd, false);
    benchmark_feature_extraction("Harris3D Brute Force", cloud, bfknn, extractor_bf, false);
  }

  SECTION("SIFT3D Extraction - KDTree vs Brute Force")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_num_scales(4);
    extractor_kd.set_base_scale(1.0f);
    extractor_kd.set_contrast_threshold(0.01f);
    extractor_kd.set_edge_threshold(15.0f);

    sift3d_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_num_scales(4);
    extractor_bf.set_base_scale(1.0f);
    extractor_bf.set_contrast_threshold(0.01f);
    extractor_bf.set_edge_threshold(15.0f);

    benchmark_feature_extraction("SIFT3D KDTree", cloud, kdtree, extractor_kd, false);
    benchmark_feature_extraction("SIFT3D Brute Force", cloud, bfknn, extractor_bf, false);
  }
}

TEST_CASE("Feature Extraction - Parameter Impact", "[benchmark][pcl][features][params]")
{
  using data_type = float;
  constexpr std::size_t num_points = 6000;

  auto cloud = generate_benchmark_cloud<data_type>(num_points);
  INFO("Testing parameter impact with " << cloud.size() << " points");

  SECTION("Curvature - Different Search Radii")
  {
    std::vector<data_type> radii = {1.0f, 2.0f, 3.0f, 4.0f};
    
    for (auto radius : radii) {
      auto kdtree = kdtree_t<data_type>{};
      curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_search_radius(radius);
      extractor.set_curvature_threshold(0.01f);

      std::string test_name = "Curvature Radius " + std::to_string(radius);
      benchmark_feature_extraction(test_name, cloud, kdtree, extractor, false);
    }
  }

  SECTION("ISS - Different Salient Radii")
  {
    std::vector<data_type> radii = {2.0f, 3.0f, 4.0f, 5.0f};
    
    for (auto radius : radii) {
      auto kdtree = kdtree_t<data_type>{};
      iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_salient_radius(radius);
      extractor.set_threshold21(0.9f);
      extractor.set_threshold32(0.9f);

      std::string test_name = "ISS Radius " + std::to_string(radius);
      benchmark_feature_extraction(test_name, cloud, kdtree, extractor, false);
    }
  }
}

TEST_CASE("Feature Extraction - Parallel Speedup Analysis", "[benchmark][pcl][features][speedup]")
{
  using data_type = float;
  
  // Test with different cloud sizes to analyze scaling
  std::vector<std::size_t> cloud_sizes = {2000, 4000, 8000, 15000, 30000};

  for (auto size : cloud_sizes) {
    DYNAMIC_SECTION("Curvature Speedup - Cloud Size: " << size << " points") {
      auto cloud = generate_benchmark_cloud<data_type>(size);
      INFO("Analyzing curvature parallel speedup with " << cloud.size() << " points");

      auto kdtree1 = kdtree_t<data_type>{};
      auto kdtree2 = kdtree_t<data_type>{};

      curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
      extractor1.set_search_radius(2.0f);
      extractor1.set_curvature_threshold(0.01f);

      curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
      extractor2.set_search_radius(2.0f);
      extractor2.set_curvature_threshold(0.01f);

      std::string size_label = "Curvature Size " + std::to_string(size);
      benchmark_feature_extraction(size_label, cloud, kdtree1, extractor1, false);
      benchmark_feature_extraction(size_label, cloud, kdtree2, extractor2, true);
    }

    DYNAMIC_SECTION("ISS Speedup - Cloud Size: " << size << " points") {
      auto cloud = generate_benchmark_cloud<data_type>(size);
      INFO("Analyzing ISS parallel speedup with " << cloud.size() << " points");

      auto kdtree1 = kdtree_t<data_type>{};
      auto kdtree2 = kdtree_t<data_type>{};

      iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
      extractor1.set_salient_radius(3.0f);
      extractor1.set_threshold21(0.9f);
      extractor1.set_threshold32(0.9f);

      iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
      extractor2.set_salient_radius(3.0f);
      extractor2.set_threshold21(0.9f);
      extractor2.set_threshold32(0.9f);

      std::string size_label = "ISS Size " + std::to_string(size);
      benchmark_feature_extraction(size_label, cloud, kdtree1, extractor1, false);
      benchmark_feature_extraction(size_label, cloud, kdtree2, extractor2, true);
    }

    DYNAMIC_SECTION("Harris3D Speedup - Cloud Size: " << size << " points") {
      auto cloud = generate_benchmark_cloud<data_type>(size);
      INFO("Analyzing Harris3D parallel speedup with " << cloud.size() << " points");

      auto kdtree1 = kdtree_t<data_type>{};
      auto kdtree2 = kdtree_t<data_type>{};

      harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
      extractor1.set_threshold(0.001f);
      extractor1.set_num_neighbors(30);
      extractor1.set_suppression_radius(2.0f);

      harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
      extractor2.set_threshold(0.001f);
      extractor2.set_num_neighbors(30);
      extractor2.set_suppression_radius(2.0f);

      std::string size_label = "Harris3D Size " + std::to_string(size);
      benchmark_feature_extraction(size_label, cloud, kdtree1, extractor1, false);
      benchmark_feature_extraction(size_label, cloud, kdtree2, extractor2, true);
    }

    DYNAMIC_SECTION("SIFT3D Speedup - Cloud Size: " << size << " points") {
      auto cloud = generate_benchmark_cloud<data_type>(size);
      INFO("Analyzing SIFT3D parallel speedup with " << cloud.size() << " points");

      auto kdtree1 = kdtree_t<data_type>{};
      auto kdtree2 = kdtree_t<data_type>{};

      sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
      extractor1.set_num_scales(4);
      extractor1.set_base_scale(1.0f);
      extractor1.set_contrast_threshold(0.01f);

      sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
      extractor2.set_num_scales(4);
      extractor2.set_base_scale(1.0f);
      extractor2.set_contrast_threshold(0.01f);

      std::string size_label = "SIFT3D Size " + std::to_string(size);
      benchmark_feature_extraction(size_label, cloud, kdtree1, extractor1, false);
      benchmark_feature_extraction(size_label, cloud, kdtree2, extractor2, true);
    }
  }
}

TEST_CASE("Feature Extraction - All Algorithms Comparison", "[benchmark][pcl][features][comparison]")
{
  using data_type = float;
  constexpr std::size_t num_points = 10000;

  auto cloud = generate_benchmark_cloud<data_type>(num_points);
  INFO("Comparing all feature extraction algorithms with " << cloud.size() << " points");

  SECTION("Sequential Comparison")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    auto kdtree3 = kdtree_t<data_type>{};
    auto kdtree4 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.01f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(3.0f);
    iss_extractor.set_threshold21(0.9f);
    iss_extractor.set_threshold32(0.9f);

    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> harris_extractor;
    harris_extractor.set_threshold(0.001f);
    harris_extractor.set_num_neighbors(30);
    harris_extractor.set_suppression_radius(2.0f);

    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> sift_extractor;
    sift_extractor.set_num_scales(4);
    sift_extractor.set_base_scale(1.0f);
    sift_extractor.set_contrast_threshold(0.01f);

    benchmark_feature_extraction("Curvature Sequential", cloud, kdtree1, curvature_extractor, false);
    benchmark_feature_extraction("ISS Sequential", cloud, kdtree2, iss_extractor, false);
    benchmark_feature_extraction("Harris3D Sequential", cloud, kdtree3, harris_extractor, false);
    benchmark_feature_extraction("SIFT3D Sequential", cloud, kdtree4, sift_extractor, false);
  }

  SECTION("Parallel Comparison")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    auto kdtree3 = kdtree_t<data_type>{};
    auto kdtree4 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.01f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(3.0f);
    iss_extractor.set_threshold21(0.9f);
    iss_extractor.set_threshold32(0.9f);

    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> harris_extractor;
    harris_extractor.set_threshold(0.001f);
    harris_extractor.set_num_neighbors(30);
    harris_extractor.set_suppression_radius(2.0f);

    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> sift_extractor;
    sift_extractor.set_num_scales(4);
    sift_extractor.set_base_scale(1.0f);
    sift_extractor.set_contrast_threshold(0.01f);

    benchmark_feature_extraction("Curvature Parallel", cloud, kdtree1, curvature_extractor, true);
    benchmark_feature_extraction("ISS Parallel", cloud, kdtree2, iss_extractor, true);
    benchmark_feature_extraction("Harris3D Parallel", cloud, kdtree3, harris_extractor, true);
    benchmark_feature_extraction("SIFT3D Parallel", cloud, kdtree4, sift_extractor, true);
  }
}

TEST_CASE("LOAM Feature Extraction Benchmarks", "[benchmark][pcl][features][loam]")
{
  using data_type = float;

  SECTION("LOAM Sequential vs Parallel - 10K points")
  {
    constexpr std::size_t num_points = 10000;
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    INFO("Testing LOAM with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_edge_threshold(0.2f);
    extractor1.set_planar_threshold(0.1f);
    extractor1.set_num_scan_neighbors(20);

    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_edge_threshold(0.2f);
    extractor2.set_planar_threshold(0.1f);
    extractor2.set_num_scan_neighbors(20);

    // Benchmark sequential
    extractor1.set_input(cloud);
    extractor1.set_knn(kdtree1);
    extractor1.enable_parallel(false);

    BENCHMARK("LOAM Sequential") {
      auto result = extractor1.extract_labeled_cloud();
      return result.cloud.size();
    };

    // Benchmark parallel
    extractor2.set_input(cloud);
    extractor2.set_knn(kdtree2);
    extractor2.enable_parallel(true);

    BENCHMARK("LOAM Parallel") {
      auto result = extractor2.extract_labeled_cloud();
      return result.cloud.size();
    };
  }

  SECTION("LOAM Feature Distribution Analysis")
  {
    constexpr std::size_t num_points = 20000;
    auto cloud = generate_planar_benchmark_cloud<data_type>(num_points);
    INFO("Analyzing LOAM feature distribution with " << cloud.size() << " points");

    auto kdtree = kdtree_t<data_type>{};
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_edge_threshold(0.15f);
    extractor.set_planar_threshold(0.05f);
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);

    BENCHMARK("LOAM Feature Extraction and Classification") {
      auto result = extractor.extract_labeled_cloud();
      
      // Count features
      auto edge_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_edge_indices(result.labels);
      auto planar_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_planar_indices(result.labels);
      
      return edge_indices.size() + planar_indices.size();
    };
  }

  SECTION("LOAM vs Other Extractors - Edge Detection")
  {
    constexpr std::size_t num_points = 8000;
    auto cloud = generate_corner_test_cloud<data_type>(num_points);
    INFO("Comparing edge detection performance with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    auto kdtree3 = kdtree_t<data_type>{};

    // LOAM extractor
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> loam_extractor;
    loam_extractor.set_edge_threshold(0.2f);
    loam_extractor.set_planar_threshold(0.1f);
    loam_extractor.set_input(cloud);
    loam_extractor.set_knn(kdtree1);

    // Harris3D for comparison
    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> harris_extractor;
    harris_extractor.set_threshold(0.001f);
    harris_extractor.set_num_neighbors(30);
    harris_extractor.set_input(cloud);
    harris_extractor.set_knn(kdtree2);

    // Curvature for comparison
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.01f);
    curvature_extractor.set_input(cloud);
    curvature_extractor.set_knn(kdtree3);

    BENCHMARK("LOAM Edge Detection") {
      auto result = loam_extractor.extract_labeled_cloud();
      auto edge_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_edge_indices(result.labels);
      return edge_indices.size();
    };

    BENCHMARK("Harris3D Edge Detection") {
      return harris_extractor.extract().size();
    };

    BENCHMARK("Curvature Edge Detection") {
      return curvature_extractor.extract().size();
    };
  }
}

TEST_CASE("SUSAN Feature Extraction Benchmarks", "[benchmark][pcl][features][susan]")
{
  using data_type = float;

  SECTION("SUSAN Sequential vs Parallel - 10K points")
  {
    constexpr std::size_t num_points = 10000;
    auto cloud = generate_corner_test_cloud<data_type>(num_points);
    INFO("Testing SUSAN with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_geometric_threshold(0.1f);
    extractor1.set_angular_threshold(0.984f);
    extractor1.set_susan_threshold(0.5f);
    extractor1.set_non_maxima_radius(0.5f);

    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_geometric_threshold(0.1f);
    extractor2.set_angular_threshold(0.984f);
    extractor2.set_susan_threshold(0.5f);
    extractor2.set_non_maxima_radius(0.5f);

    // Benchmark sequential
    extractor1.set_input(cloud);
    extractor1.set_knn(kdtree1);
    extractor1.enable_parallel(false);

    BENCHMARK("SUSAN Sequential") {
      return extractor1.extract().size();
    };

    // Benchmark parallel
    extractor2.set_input(cloud);
    extractor2.set_knn(kdtree2);
    extractor2.enable_parallel(true);

    BENCHMARK("SUSAN Parallel") {
      return extractor2.extract().size();
    };
  }

  SECTION("SUSAN with and without Normal Similarity")
  {
    constexpr std::size_t num_points = 8000;
    auto cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
    INFO("Testing SUSAN normal similarity with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    // With normal similarity
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_normal;
    extractor_normal.set_search_radius(2.0f);
    extractor_normal.set_susan_threshold(0.5f);
    extractor_normal.set_use_normal_similarity(true);
    extractor_normal.set_input(cloud);
    extractor_normal.set_knn(kdtree1);

    BENCHMARK("SUSAN with Normals") {
      return extractor_normal.extract().size();
    };

    // Without normal similarity
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_geo;
    extractor_geo.set_search_radius(2.0f);
    extractor_geo.set_susan_threshold(0.5f);
    extractor_geo.set_use_normal_similarity(false);
    extractor_geo.set_input(cloud);
    extractor_geo.set_knn(kdtree2);

    BENCHMARK("SUSAN Geometric Only") {
      return extractor_geo.extract().size();
    };
  }

  SECTION("SUSAN Parameter Sensitivity")
  {
    constexpr std::size_t num_points = 5000;
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    
    // Test different thresholds
    {
      auto kdtree = kdtree_t<data_type>{};
      susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_search_radius(1.5f);
      extractor.set_susan_threshold(0.3f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("SUSAN Threshold 0.3") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_search_radius(1.5f);
      extractor.set_susan_threshold(0.5f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("SUSAN Threshold 0.5") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_search_radius(1.5f);
      extractor.set_susan_threshold(0.7f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("SUSAN Threshold 0.7") {
        return extractor.extract().size();
      };
    }
  }
}

TEST_CASE("AGAST Feature Extraction Benchmarks", "[benchmark][pcl][features][agast]")
{
  using data_type = float;

  SECTION("AGAST Sequential vs Parallel - 10K points")
  {
    constexpr std::size_t num_points = 10000;
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    INFO("Testing AGAST with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_threshold(0.05f);
    extractor1.set_pattern_radius(0.5f);
    extractor1.set_num_test_points(16);
    extractor1.set_min_arc_length(9);

    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_threshold(0.05f);
    extractor2.set_pattern_radius(0.5f);
    extractor2.set_num_test_points(16);
    extractor2.set_min_arc_length(9);

    // Benchmark sequential
    extractor1.set_input(cloud);
    extractor1.set_knn(kdtree1);
    extractor1.enable_parallel(false);

    BENCHMARK("AGAST Sequential") {
      return extractor1.extract().size();
    };

    // Benchmark parallel
    extractor2.set_input(cloud);
    extractor2.set_knn(kdtree2);
    extractor2.enable_parallel(true);

    BENCHMARK("AGAST Parallel") {
      return extractor2.extract().size();
    };
  }

  SECTION("AGAST Pattern Configuration Impact")
  {
    constexpr std::size_t num_points = 8000;
    auto cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
    INFO("Testing AGAST pattern configurations with " << cloud.size() << " points");

    // Test different pattern sizes
    {
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.05f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_num_test_points(12);
      extractor.set_min_arc_length(7);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST 12 Points") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.05f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_num_test_points(16);
      extractor.set_min_arc_length(9);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST 16 Points") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.05f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_num_test_points(20);
      extractor.set_min_arc_length(11);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST 20 Points") {
        return extractor.extract().size();
      };
    }
  }

  SECTION("AGAST on Different Geometries")
  {
    constexpr std::size_t num_points = 6000;
    
    // Planar cloud
    {
      auto planar_cloud = generate_planar_benchmark_cloud<data_type>(num_points);
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.03f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_input(planar_cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST Planar Cloud") {
        return extractor.extract().size();
      };
    }

    // Spherical cloud
    {
      auto sphere_cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.03f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_input(sphere_cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST Spherical Cloud") {
        return extractor.extract().size();
      };
    }

    // Corner cloud
    {
      auto corner_cloud = generate_corner_test_cloud<data_type>(num_points);
      auto kdtree = kdtree_t<data_type>{};
      agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_threshold(0.03f);
      extractor.set_pattern_radius(0.5f);
      extractor.set_input(corner_cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("AGAST Corner Cloud") {
        return extractor.extract().size();
      };
    }
  }
}

TEST_CASE("MLS Feature Extraction Benchmarks", "[benchmark][pcl][features][mls]")
{
  using data_type = float;

  SECTION("MLS Sequential vs Parallel - 10K points")
  {
    constexpr std::size_t num_points = 10000;
    auto cloud = generate_corner_test_cloud<data_type>(num_points);
    
    // Add normals
    cloud.normals.reserve(cloud.points.size());
    for (const auto& pt : cloud.points) {
      point_t<data_type> normal;
      normal.x = 0.0f;
      normal.y = 0.0f;
      normal.z = 1.0f;
      cloud.normals.push_back(normal);
    }
    
    INFO("Testing MLS with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor1;
    extractor1.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor1.set_variation_threshold(0.0005f);
    extractor1.set_curvature_threshold(0.05f);
    extractor1.set_non_maxima_radius(0.5f);

    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor2;
    extractor2.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor2.set_variation_threshold(0.0005f);
    extractor2.set_curvature_threshold(0.05f);
    extractor2.set_non_maxima_radius(0.5f);

    // Benchmark sequential
    extractor1.set_input(cloud);
    extractor1.set_knn(kdtree1);
    extractor1.enable_parallel(false);

    BENCHMARK("MLS Sequential") {
      return extractor1.extract().size();
    };

    // Benchmark parallel
    extractor2.set_input(cloud);
    extractor2.set_knn(kdtree2);
    extractor2.enable_parallel(true);

    BENCHMARK("MLS Parallel") {
      return extractor2.extract().size();
    };
  }

  SECTION("MLS Polynomial Order Impact")
  {
    constexpr std::size_t num_points = 8000;
    auto cloud = generate_benchmark_cloud<data_type>(num_points);
    
    // Add normals
    cloud.normals.reserve(cloud.points.size());
    for (const auto& pt : cloud.points) {
      point_t<data_type> normal;
      normal.x = 0.0f;
      normal.y = 0.0f;
      normal.z = 1.0f;
      cloud.normals.push_back(normal);
    }
    
    INFO("Testing MLS polynomial orders with " << cloud.size() << " points");

    // Test different polynomial orders
    {
      auto kdtree = kdtree_t<data_type>{};
      mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::NONE);
      extractor.set_variation_threshold(0.0005f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("MLS Plane Only") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::LINEAR);
      extractor.set_variation_threshold(0.0005f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("MLS Linear") {
        return extractor.extract().size();
      };
    }
    
    {
      auto kdtree = kdtree_t<data_type>{};
      mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
      extractor.set_variation_threshold(0.0005f);
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);

      BENCHMARK("MLS Quadratic") {
        return extractor.extract().size();
      };
    }
  }

  SECTION("MLS with and without Curvature")
  {
    constexpr std::size_t num_points = 6000;
    auto cloud = generate_spherical_benchmark_cloud<data_type>(num_points);
    
    // Add proper normals for sphere
    cloud.normals.clear();
    cloud.normals.reserve(cloud.points.size());
    for (const auto& pt : cloud.points) {
      float norm = std::sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);
      point_t<data_type> normal;
      normal.x = pt.x / norm;
      normal.y = pt.y / norm;
      normal.z = pt.z / norm;
      cloud.normals.push_back(normal);
    }
    
    INFO("Testing MLS curvature computation with " << cloud.size() << " points");

    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    // With curvature
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_curv;
    extractor_curv.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor_curv.set_compute_curvatures(true);
    extractor_curv.set_variation_threshold(0.0001f);
    extractor_curv.set_curvature_threshold(0.05f);
    extractor_curv.set_input(cloud);
    extractor_curv.set_knn(kdtree1);

    BENCHMARK("MLS with Curvature") {
      return extractor_curv.extract().size();
    };

    // Without curvature
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_no_curv;
    extractor_no_curv.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor_no_curv.set_compute_curvatures(false);
    extractor_no_curv.set_variation_threshold(0.0001f);
    extractor_no_curv.set_input(cloud);
    extractor_no_curv.set_knn(kdtree2);

    BENCHMARK("MLS without Curvature") {
      return extractor_no_curv.extract().size();
    };
  }
}

TEST_CASE("All Feature Extractors Comparison - Including SUSAN and AGAST", "[benchmark][pcl][features][all]")
{
  using data_type = float;
  constexpr std::size_t num_points = 10000;

  auto cloud = generate_corner_test_cloud<data_type>(num_points);
  INFO("Comparing all feature extraction algorithms with " << cloud.size() << " points");

  SECTION("Sequential Comparison - All Extractors")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    auto kdtree3 = kdtree_t<data_type>{};
    auto kdtree4 = kdtree_t<data_type>{};
    auto kdtree5 = kdtree_t<data_type>{};
    auto kdtree6 = kdtree_t<data_type>{};
    auto kdtree7 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> curvature_extractor;
    curvature_extractor.set_search_radius(2.0f);
    curvature_extractor.set_curvature_threshold(0.01f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> iss_extractor;
    iss_extractor.set_salient_radius(3.0f);
    iss_extractor.set_threshold21(0.9f);
    iss_extractor.set_threshold32(0.9f);

    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> harris_extractor;
    harris_extractor.set_threshold(0.001f);
    harris_extractor.set_num_neighbors(30);
    harris_extractor.set_suppression_radius(2.0f);

    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> sift_extractor;
    sift_extractor.set_num_scales(4);
    sift_extractor.set_base_scale(1.0f);
    sift_extractor.set_contrast_threshold(0.01f);

    loam_feature_extractor_t<data_type, kdtree_t<data_type>> loam_extractor;
    loam_extractor.set_edge_threshold(0.2f);
    loam_extractor.set_planar_threshold(0.1f);

    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> susan_extractor;
    susan_extractor.set_search_radius(1.5f);
    susan_extractor.set_susan_threshold(0.5f);

    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> agast_extractor;
    agast_extractor.set_threshold(0.05f);
    agast_extractor.set_pattern_radius(0.5f);
    
    // Add normals for MLS
    cloud.normals.reserve(cloud.points.size());
    for (const auto& pt : cloud.points) {
      point_t<data_type> normal;
      normal.x = 0.0f;
      normal.y = 0.0f;
      normal.z = 1.0f;
      cloud.normals.push_back(normal);
    }
    
    auto kdtree8 = kdtree_t<data_type>{};
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> mls_extractor;
    mls_extractor.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    mls_extractor.set_variation_threshold(0.0005f);
    mls_extractor.set_curvature_threshold(0.05f);

    benchmark_feature_extraction("Curvature", cloud, kdtree1, curvature_extractor, false);
    benchmark_feature_extraction("ISS", cloud, kdtree2, iss_extractor, false);
    benchmark_feature_extraction("Harris3D", cloud, kdtree3, harris_extractor, false);
    benchmark_feature_extraction("SIFT3D", cloud, kdtree4, sift_extractor, false);
    
    // LOAM special case
    loam_extractor.set_input(cloud);
    loam_extractor.set_knn(kdtree5);
    loam_extractor.enable_parallel(false);
    BENCHMARK("LOAM (Sequential)") {
      return loam_extractor.extract_labeled_cloud().cloud.size();
    };
    
    benchmark_feature_extraction("SUSAN", cloud, kdtree6, susan_extractor, false);
    benchmark_feature_extraction("AGAST", cloud, kdtree7, agast_extractor, false);
    benchmark_feature_extraction("MLS", cloud, kdtree8, mls_extractor, false);
  }
}
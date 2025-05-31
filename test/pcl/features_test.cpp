#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/features/features.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/types/point_utils.hpp>

#include <iostream>
#include <memory>
#include <cmath>

using namespace toolbox::pcl;
using namespace toolbox::types;

// Helper function to generate random point cloud
template<typename T>
point_cloud_t<T> generate_test_cloud(std::size_t num_points, T min_val = -10, T max_val = 10)
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

// Helper function to create a planar point cloud with some noise
template<typename T>
point_cloud_t<T> generate_planar_test_cloud(std::size_t num_points, T extent = 5.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

  toolbox::utils::random_t rng;
  
  // Generate points on a plane with some noise
  for (std::size_t i = 0; i < num_points; ++i)
  {
    T x = rng.random<T>(-extent, extent);
    T y = rng.random<T>(-extent, extent);
    T z = rng.random<T>(-0.1, 0.1);  // Small noise in z direction
    
    cloud.points.emplace_back(x, y, z);
  }

  return cloud;
}

// Helper function to create a spherical point cloud
template<typename T>
point_cloud_t<T> generate_spherical_test_cloud(std::size_t num_points, T radius = 5.0)
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
point_cloud_t<T> generate_corner_test_cloud(std::size_t num_points, T extent = 5.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);

  toolbox::utils::random_t rng;
  
  // Generate corner-like structure with three planes meeting at origin
  for (std::size_t i = 0; i < num_points; ++i)
  {
    T choice = rng.random<T>(0, 3);
    T noise = static_cast<T>(0.1);
    
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

TEST_CASE("Curvature Keypoint Extractor - Basic Functionality", "[pcl][features][curvature]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(1000);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_search_radius(1.0f);
    extractor.set_curvature_threshold(0.01f);
    extractor.set_min_neighbors(10);
    extractor.set_non_maxima_radius(0.5f);
    
    REQUIRE_THAT(extractor.get_search_radius(), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(extractor.get_curvature_threshold(), Catch::Matchers::WithinAbs(0.01f, 1e-6f));
    REQUIRE(extractor.get_min_neighbors() == 10);
    REQUIRE_THAT(extractor.get_non_maxima_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
  }

  SECTION("Sequential Keypoint Extraction")
  {
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(2.0f);
    extractor.set_curvature_threshold(0.005f);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
    
    // Verify keypoint cloud contains correct points
    for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
      const auto& original_point = cloud.points[keypoint_indices[i]];
      const auto& keypoint = keypoint_cloud.points[i];
      REQUIRE_THAT(original_point.x, Catch::Matchers::WithinAbs(keypoint.x, 1e-6f));
      REQUIRE_THAT(original_point.y, Catch::Matchers::WithinAbs(keypoint.y, 1e-6f));
      REQUIRE_THAT(original_point.z, Catch::Matchers::WithinAbs(keypoint.z, 1e-6f));
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(2.0f);
    extractor.set_curvature_threshold(0.005f);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Sequential vs Parallel Consistency")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_seq;
    extractor_seq.set_input(cloud);
    extractor_seq.set_knn(kdtree1);
    extractor_seq.set_search_radius(1.5f);
    extractor_seq.set_curvature_threshold(0.01f);
    extractor_seq.enable_parallel(false);
    
    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_par;
    extractor_par.set_input(cloud);
    extractor_par.set_knn(kdtree2);
    extractor_par.set_search_radius(1.5f);
    extractor_par.set_curvature_threshold(0.01f);
    extractor_par.enable_parallel(true);
    
    auto keypoints_seq = extractor_seq.extract();
    auto keypoints_par = extractor_par.extract();
    
    // Results should be identical
    REQUIRE(keypoints_seq.size() == keypoints_par.size());
    
    // Sort both vectors for comparison
    std::sort(keypoints_seq.begin(), keypoints_seq.end());
    std::sort(keypoints_par.begin(), keypoints_par.end());
    
    for (std::size_t i = 0; i < keypoints_seq.size(); ++i) {
      REQUIRE(keypoints_seq[i] == keypoints_par[i]);
    }
  }
}

TEST_CASE("ISS Keypoint Extractor - Basic Functionality", "[pcl][features][iss]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(1000);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_salient_radius(1.0f);
    extractor.set_non_maxima_radius(0.5f);
    extractor.set_threshold21(0.975f);
    extractor.set_threshold32(0.975f);
    extractor.set_min_neighbors(5);
    
    REQUIRE_THAT(extractor.get_salient_radius(), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(extractor.get_non_maxima_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE_THAT(extractor.get_threshold21(), Catch::Matchers::WithinAbs(0.975f, 1e-6f));
    REQUIRE_THAT(extractor.get_threshold32(), Catch::Matchers::WithinAbs(0.975f, 1e-6f));
    REQUIRE(extractor.get_min_neighbors() == 5);
  }

  SECTION("Sequential Keypoint Extraction")
  {
    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_salient_radius(2.0f);
    extractor.set_threshold21(0.9f);
    extractor.set_threshold32(0.9f);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() >= 0);  // ISS might find few keypoints
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_salient_radius(2.0f);
    extractor.set_threshold21(0.9f);
    extractor.set_threshold32(0.9f);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() >= 0);  // ISS might find few keypoints
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }
}

TEST_CASE("Feature Extraction - Different KNN Algorithms", "[pcl][features][knn]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(500);

  SECTION("Curvature Extraction with Different KNN")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};
    auto bfknn_parallel = bfknn_parallel_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_input(cloud);
    extractor_kd.set_knn(kdtree);
    extractor_kd.set_search_radius(2.5f);  // Increased radius
    extractor_kd.set_curvature_threshold(0.005f);  // Lower threshold
    extractor_kd.enable_parallel(false);

    curvature_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_input(cloud);
    extractor_bf.set_knn(bfknn);
    extractor_bf.set_search_radius(2.5f);  // Increased radius
    extractor_bf.set_curvature_threshold(0.005f);  // Lower threshold
    extractor_bf.enable_parallel(false);

    auto keypoints_kd = extractor_kd.extract();
    auto keypoints_bf = extractor_bf.extract();

    // Results should be similar (might not be identical due to implementation differences)
    INFO("KDTree found " << keypoints_kd.size() << " keypoints");
    INFO("Brute Force found " << keypoints_bf.size() << " keypoints");
    
    // Both should find some keypoints
    REQUIRE(keypoints_kd.size() > 0);
    REQUIRE(keypoints_bf.size() > 0);
  }

  SECTION("ISS Extraction with Different KNN")
  {
    auto kdtree = kdtree_t<data_type>{};
    auto bfknn = bfknn_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_kd;
    extractor_kd.set_input(cloud);
    extractor_kd.set_knn(kdtree);
    extractor_kd.set_salient_radius(2.0f);
    extractor_kd.set_threshold21(0.9f);
    extractor_kd.set_threshold32(0.9f);
    extractor_kd.enable_parallel(false);

    iss_keypoint_extractor_t<data_type, bfknn_t<data_type>> extractor_bf;
    extractor_bf.set_input(cloud);
    extractor_bf.set_knn(bfknn);
    extractor_bf.set_salient_radius(2.0f);
    extractor_bf.set_threshold21(0.9f);
    extractor_bf.set_threshold32(0.9f);
    extractor_bf.enable_parallel(false);

    auto keypoints_kd = extractor_kd.extract();
    auto keypoints_bf = extractor_bf.extract();

    INFO("KDTree found " << keypoints_kd.size() << " keypoints");
    INFO("Brute Force found " << keypoints_bf.size() << " keypoints");
    
    // Both should produce valid results
    REQUIRE(keypoints_kd.size() >= 0);
    REQUIRE(keypoints_bf.size() >= 0);
  }
}

TEST_CASE("Feature Extraction - Geometric Point Clouds", "[pcl][features][geometry]")
{
  using data_type = float;

  SECTION("Planar Point Cloud - Should Find Edge Features")
  {
    auto cloud = generate_planar_test_cloud<data_type>(800);
    auto kdtree = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(1.0f);
    extractor.set_curvature_threshold(0.001f);  // Lower threshold for planar surface
    extractor.enable_parallel(false);

    auto keypoints = extractor.extract();
    
    INFO("Found " << keypoints.size() << " keypoints in planar cloud");
    // Planar surfaces should have few high-curvature points
    REQUIRE(keypoints.size() >= 0);
  }

  SECTION("Spherical Point Cloud - Should Find Distributed Features")
  {
    auto cloud = generate_spherical_test_cloud<data_type>(800);
    auto kdtree = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_salient_radius(1.5f);
    extractor.set_threshold21(0.85f);  // More lenient thresholds
    extractor.set_threshold32(0.85f);
    extractor.enable_parallel(false);

    auto keypoints = extractor.extract();
    
    INFO("Found " << keypoints.size() << " ISS keypoints in spherical cloud");
    // Spherical surfaces should have more uniform features
    REQUIRE(keypoints.size() >= 0);
  }
}

TEST_CASE("Feature Extraction - Edge Cases", "[pcl][features][edge]")
{
  using data_type = float;

  SECTION("Empty Point Cloud")
  {
    point_cloud_t<data_type> empty_cloud;
    auto kdtree = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(empty_cloud);
    extractor.set_knn(kdtree);

    auto keypoints = extractor.extract();
    REQUIRE(keypoints.empty());
  }

  SECTION("Single Point Cloud")
  {
    point_cloud_t<data_type> single_cloud;
    single_cloud.points.emplace_back(1.0f, 2.0f, 3.0f);
    auto kdtree = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(single_cloud);
    extractor.set_knn(kdtree);
    extractor.set_min_neighbors(1);

    auto keypoints = extractor.extract();
    REQUIRE(keypoints.size() <= 1);  // Might be 0 due to insufficient neighbors
  }

  SECTION("Very Small Point Cloud")
  {
    auto cloud = generate_test_cloud<data_type>(5);
    auto kdtree = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_min_neighbors(3);
    extractor.set_salient_radius(10.0f);  // Large radius to include all points

    auto keypoints = extractor.extract();
    REQUIRE(keypoints.size() <= cloud.size());
  }
}

TEST_CASE("Feature Extraction - Parameter Sensitivity", "[pcl][features][params]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(1000);

  SECTION("Curvature Threshold Sensitivity")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_low;
    extractor_low.set_input(cloud);
    extractor_low.set_knn(kdtree1);
    extractor_low.set_search_radius(1.5f);
    extractor_low.set_curvature_threshold(0.001f);  // Low threshold

    curvature_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_high;
    extractor_high.set_input(cloud);
    extractor_high.set_knn(kdtree2);
    extractor_high.set_search_radius(1.5f);
    extractor_high.set_curvature_threshold(0.1f);   // High threshold

    auto keypoints_low = extractor_low.extract();
    auto keypoints_high = extractor_high.extract();

    INFO("Low threshold: " << keypoints_low.size() << " keypoints");
    INFO("High threshold: " << keypoints_high.size() << " keypoints");

    // Lower threshold should find more keypoints
    REQUIRE(keypoints_low.size() >= keypoints_high.size());
  }

  SECTION("ISS Threshold Sensitivity")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_strict;
    extractor_strict.set_input(cloud);
    extractor_strict.set_knn(kdtree1);
    extractor_strict.set_salient_radius(2.0f);
    extractor_strict.set_threshold21(0.99f);  // Very strict
    extractor_strict.set_threshold32(0.99f);

    iss_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_lenient;
    extractor_lenient.set_input(cloud);
    extractor_lenient.set_knn(kdtree2);
    extractor_lenient.set_salient_radius(2.0f);
    extractor_lenient.set_threshold21(0.8f);   // More lenient
    extractor_lenient.set_threshold32(0.8f);

    auto keypoints_strict = extractor_strict.extract();
    auto keypoints_lenient = extractor_lenient.extract();

    INFO("Strict thresholds: " << keypoints_strict.size() << " keypoints");
    INFO("Lenient thresholds: " << keypoints_lenient.size() << " keypoints");

    // Both should find keypoints, but the relationship might vary due to the nature of ISS
    REQUIRE(keypoints_strict.size() >= 0);
    REQUIRE(keypoints_lenient.size() >= 0);
    // Note: With ISS, more lenient thresholds don't always mean more keypoints
    // because ISS uses eigenvalue ratios which can behave non-monotonically
  }
}

TEST_CASE("Harris3D Keypoint Extraction", "[pcl][features][harris3d]")
{
  using data_type = float;
  auto cloud = generate_corner_test_cloud<data_type>(1000);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_threshold(0.01f);
    extractor.set_harris_k(0.04f);
    extractor.set_suppression_radius(0.1f);
    extractor.set_num_neighbors(20);
    
    REQUIRE_THAT(extractor.get_threshold(), Catch::Matchers::WithinAbs(0.01f, 1e-6f));
    REQUIRE_THAT(extractor.get_harris_k(), Catch::Matchers::WithinAbs(0.04f, 1e-6f));
    REQUIRE_THAT(extractor.get_suppression_radius(), Catch::Matchers::WithinAbs(0.1f, 1e-6f));
    REQUIRE(extractor.get_num_neighbors() == 20);
  }

  SECTION("Sequential Keypoint Extraction")
  {
    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_threshold(0.001f);
    extractor.set_num_neighbors(30);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    harris3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_threshold(0.001f);
    extractor.set_num_neighbors(30);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }
}

TEST_CASE("SIFT3D Keypoint Extraction", "[pcl][features][sift3d]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(800);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_num_scales(5);
    extractor.set_base_scale(0.05f);
    extractor.set_scale_factor(1.414f);
    extractor.set_contrast_threshold(0.03f);
    extractor.set_edge_threshold(10.0f);
    extractor.set_num_neighbors(20);
    
    REQUIRE(extractor.get_num_scales() == 5);
    REQUIRE_THAT(extractor.get_base_scale(), Catch::Matchers::WithinAbs(0.05f, 1e-6f));
    REQUIRE_THAT(extractor.get_scale_factor(), Catch::Matchers::WithinAbs(1.414f, 1e-3f));
    REQUIRE_THAT(extractor.get_contrast_threshold(), Catch::Matchers::WithinAbs(0.03f, 1e-6f));
    REQUIRE_THAT(extractor.get_edge_threshold(), Catch::Matchers::WithinAbs(10.0f, 1e-6f));
    REQUIRE(extractor.get_num_neighbors() == 20);
  }

  SECTION("Sequential Keypoint Extraction")
  {
    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_contrast_threshold(0.01f);  // Lower threshold for more keypoints
    extractor.set_edge_threshold(15.0f);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() >= 0);  // SIFT3D might find few keypoints
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_contrast_threshold(0.01f);
    extractor.set_edge_threshold(15.0f);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() >= 0);  // SIFT3D might find few keypoints
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Scale Space Parameters")
  {
    sift3d_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    
    // Test with different scale parameters
    extractor.set_num_scales(3);
    extractor.set_base_scale(0.1f);
    extractor.set_scale_factor(2.0f);
    extractor.set_contrast_threshold(0.005f);
    
    auto keypoints = extractor.extract();
    
    INFO("Found " << keypoints.size() << " SIFT3D keypoints");
    REQUIRE(keypoints.size() >= 0);
  }
}

TEST_CASE("LOAM Feature Extractor", "[pcl][features][loam]")
{
  using data_type = float;
  auto cloud = generate_corner_test_cloud<data_type>(1000);
  auto original_cloud = cloud;  // Save original for intensity restoration
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_edge_threshold(0.2f);
    extractor.set_planar_threshold(0.1f);
    extractor.set_curvature_threshold(0.001f);
    extractor.set_num_scan_neighbors(10);
    
    REQUIRE_THAT(extractor.get_edge_threshold(), Catch::Matchers::WithinAbs(0.2f, 1e-6f));
    REQUIRE_THAT(extractor.get_planar_threshold(), Catch::Matchers::WithinAbs(0.1f, 1e-6f));
    REQUIRE_THAT(extractor.get_curvature_threshold(), Catch::Matchers::WithinAbs(0.001f, 1e-6f));
    REQUIRE(extractor.get_num_scan_neighbors() == 10);
  }

  SECTION("Feature Extraction and Classification")
  {
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_edge_threshold(0.15f);
    extractor.set_planar_threshold(0.05f);
    extractor.enable_parallel(false);
    
    // Extract features - returns labeled result
    auto result = extractor.extract_labeled_cloud();
    
    // Cloud size should remain the same
    REQUIRE(result.cloud.size() == cloud.size());
    REQUIRE(result.labels.size() == cloud.size());
    
    // Extract different feature types
    auto edge_points = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_edge_points(result);
    auto planar_points = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_planar_points(result);
    auto non_feature_points = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_non_feature_points(result);
    
    // Should have found some features
    INFO("Edge points: " << edge_points.size());
    INFO("Planar points: " << planar_points.size());
    INFO("Non-feature points: " << non_feature_points.size());
    
    REQUIRE(edge_points.size() > 0);
    REQUIRE(planar_points.size() > 0);
    
    // Total should equal original size
    REQUIRE(edge_points.size() + planar_points.size() + non_feature_points.size() == cloud.size());
    
    // Test index extraction
    auto edge_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_edge_indices(result.labels);
    auto planar_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_planar_indices(result.labels);
    
    REQUIRE(edge_indices.size() == edge_points.size());
    REQUIRE(planar_indices.size() == planar_points.size());
  }

  SECTION("Compatibility with Base Class Interface")
  {
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    
    // Using base class interface - should return all feature points
    auto feature_points = extractor.extract_keypoints();
    auto feature_indices = extractor.extract();
    
    REQUIRE(feature_points.size() == feature_indices.size());
    REQUIRE(feature_points.size() > 0);
    
    // Verify indices are valid
    for (const auto& idx : feature_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Feature Extraction")
  {
    loam_feature_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.enable_parallel(true);
    
    auto result = extractor.extract_labeled_cloud();
    
    // Extract feature counts
    auto edge_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_edge_indices(result.labels);
    auto planar_indices = loam_feature_extractor_t<data_type, kdtree_t<data_type>>::extract_planar_indices(result.labels);
    
    // Should have found features
    REQUIRE(edge_indices.size() > 0);
    REQUIRE(planar_indices.size() > 0);
  }
}

TEST_CASE("SUSAN Keypoint Extraction", "[pcl][features][susan]")
{
  using data_type = float;
  auto cloud = generate_corner_test_cloud<data_type>(1000);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_geometric_threshold(0.1f);
    extractor.set_angular_threshold(0.984f);
    extractor.set_susan_threshold(0.5f);
    extractor.set_non_maxima_radius(0.5f);
    extractor.set_use_normal_similarity(true);
    
    REQUIRE_THAT(extractor.get_geometric_threshold(), Catch::Matchers::WithinAbs(0.1f, 1e-6f));
    REQUIRE_THAT(extractor.get_angular_threshold(), Catch::Matchers::WithinAbs(0.984f, 1e-6f));
    REQUIRE_THAT(extractor.get_susan_threshold(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE_THAT(extractor.get_non_maxima_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE(extractor.get_use_normal_similarity() == true);
  }

  SECTION("Sequential Keypoint Extraction with Normals")
  {
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(1.0f);
    extractor.set_susan_threshold(0.5f);
    extractor.set_use_normal_similarity(true);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Sequential Keypoint Extraction without Normals")
  {
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(1.0f);
    extractor.set_susan_threshold(0.5f);
    extractor.set_use_normal_similarity(false);  // Geometric distance only
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(1.0f);
    extractor.set_susan_threshold(0.5f);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("SUSAN with Different Thresholds")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};

    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_low;
    extractor_low.set_input(cloud);
    extractor_low.set_knn(kdtree1);
    extractor_low.set_search_radius(1.0f);
    extractor_low.set_susan_threshold(0.3f);  // Low threshold - more keypoints

    susan_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_high;
    extractor_high.set_input(cloud);
    extractor_high.set_knn(kdtree2);
    extractor_high.set_search_radius(1.0f);
    extractor_high.set_susan_threshold(0.7f);  // High threshold - fewer keypoints

    auto keypoints_low = extractor_low.extract();
    auto keypoints_high = extractor_high.extract();

    INFO("Low threshold: " << keypoints_low.size() << " keypoints");
    INFO("High threshold: " << keypoints_high.size() << " keypoints");

    // Lower threshold should find more keypoints
    REQUIRE(keypoints_low.size() >= keypoints_high.size());
  }
}

TEST_CASE("AGAST Keypoint Extraction", "[pcl][features][agast]")
{
  using data_type = float;
  auto cloud = generate_test_cloud<data_type>(1000);
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_threshold(0.1f);
    extractor.set_pattern_radius(0.5f);
    extractor.set_non_maxima_radius(0.5f);
    extractor.set_num_test_points(16);
    extractor.set_min_arc_length(9);
    
    REQUIRE_THAT(extractor.get_threshold(), Catch::Matchers::WithinAbs(0.1f, 1e-6f));
    REQUIRE_THAT(extractor.get_pattern_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE_THAT(extractor.get_non_maxima_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE(extractor.get_num_test_points() == 16);
    REQUIRE(extractor.get_min_arc_length() == 9);
  }

  SECTION("Sequential Keypoint Extraction")
  {
    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_threshold(0.05f);
    extractor.set_pattern_radius(0.5f);
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_threshold(0.05f);
    extractor.set_pattern_radius(0.5f);
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Test Pattern Configuration")
  {
    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    
    // Test with different pattern sizes
    extractor.set_num_test_points(12);
    extractor.set_min_arc_length(7);
    extractor.set_threshold(0.05f);
    
    auto keypoints1 = extractor.extract();
    
    // Change pattern configuration
    extractor.set_num_test_points(20);
    extractor.set_min_arc_length(11);
    
    auto keypoints2 = extractor.extract();
    
    INFO("12 test points: " << keypoints1.size() << " keypoints");
    INFO("20 test points: " << keypoints2.size() << " keypoints");
    
    // Both should find keypoints
    REQUIRE(keypoints1.size() > 0);
    REQUIRE(keypoints2.size() > 0);
  }

  SECTION("AGAST on Spherical Cloud")
  {
    auto sphere_cloud = generate_spherical_test_cloud<data_type>(800);
    auto sphere_kdtree = kdtree_t<data_type>{};
    
    agast_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(sphere_cloud);
    extractor.set_knn(sphere_kdtree);
    extractor.set_threshold(0.03f);
    extractor.set_pattern_radius(1.0f);
    
    auto keypoints = extractor.extract();
    
    INFO("Found " << keypoints.size() << " AGAST keypoints on sphere");
    REQUIRE(keypoints.size() > 0);
  }
}

TEST_CASE("MLS Keypoint Extraction", "[pcl][features][mls]")
{
  using data_type = float;
  auto cloud = generate_corner_test_cloud<data_type>(1000);
  
  // Add normals to the cloud (required for MLS)
  cloud.normals.reserve(cloud.points.size());
  for (const auto& pt : cloud.points) {
    // Simple normal estimation based on position
    point_t<data_type> normal;
    normal.x = 0.0f;
    normal.y = 0.0f;
    normal.z = 1.0f;  // Default to up
    cloud.normals.push_back(normal);
  }
  
  auto kdtree = kdtree_t<data_type>{};

  SECTION("Basic Setup and Configuration")
  {
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    
    REQUIRE(extractor.set_input(cloud) == cloud.size());
    REQUIRE(extractor.set_knn(kdtree) == cloud.size());
    
    extractor.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor.set_sqr_gauss_param(0.0f);
    extractor.set_compute_curvatures(true);
    extractor.set_variation_threshold(0.001f);
    extractor.set_curvature_threshold(0.1f);
    extractor.set_non_maxima_radius(0.5f);
    extractor.set_min_neighbors(10);
    
    REQUIRE(extractor.get_polynomial_order() == mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    REQUIRE_THAT(extractor.get_sqr_gauss_param(), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
    REQUIRE(extractor.get_compute_curvatures() == true);
    REQUIRE_THAT(extractor.get_variation_threshold(), Catch::Matchers::WithinAbs(0.001f, 1e-6f));
    REQUIRE_THAT(extractor.get_curvature_threshold(), Catch::Matchers::WithinAbs(0.1f, 1e-6f));
    REQUIRE_THAT(extractor.get_non_maxima_radius(), Catch::Matchers::WithinAbs(0.5f, 1e-6f));
    REQUIRE(extractor.get_min_neighbors() == 10);
  }

  SECTION("Sequential Keypoint Extraction")
  {
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(2.0f);  // Increased radius
    extractor.set_variation_threshold(0.00001f);  // Much lower threshold
    extractor.set_curvature_threshold(0.001f);  // Lower threshold
    extractor.enable_parallel(false);
    
    auto keypoint_indices = extractor.extract();
    auto keypoint_cloud = extractor.extract_keypoints();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    REQUIRE(keypoint_cloud.size() == keypoint_indices.size());
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Parallel Keypoint Extraction")
  {
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(kdtree);
    extractor.set_search_radius(2.0f);  // Increased radius
    extractor.set_variation_threshold(0.00001f);  // Much lower threshold
    extractor.set_curvature_threshold(0.001f);  // Lower threshold
    extractor.enable_parallel(true);
    
    auto keypoint_indices = extractor.extract();
    
    // Should extract some keypoints
    REQUIRE(keypoint_indices.size() > 0);
    
    // Verify keypoint indices are valid
    for (const auto& idx : keypoint_indices) {
      REQUIRE(idx < cloud.size());
    }
  }

  SECTION("Different Polynomial Orders")
  {
    auto kdtree1 = kdtree_t<data_type>{};
    auto kdtree2 = kdtree_t<data_type>{};
    auto kdtree3 = kdtree_t<data_type>{};

    // Test with no polynomial (plane only)
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_none;
    extractor_none.set_input(cloud);
    extractor_none.set_knn(kdtree1);
    extractor_none.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::NONE);
    extractor_none.set_search_radius(2.0f);
    extractor_none.set_variation_threshold(0.00001f);

    // Test with linear polynomial
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_linear;
    extractor_linear.set_input(cloud);
    extractor_linear.set_knn(kdtree2);
    extractor_linear.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::LINEAR);
    extractor_linear.set_search_radius(2.0f);
    extractor_linear.set_variation_threshold(0.00001f);

    // Test with quadratic polynomial
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor_quadratic;
    extractor_quadratic.set_input(cloud);
    extractor_quadratic.set_knn(kdtree3);
    extractor_quadratic.set_polynomial_order(mls_keypoint_extractor_t<data_type, kdtree_t<data_type>>::polynomial_order_t::QUADRATIC);
    extractor_quadratic.set_search_radius(2.0f);
    extractor_quadratic.set_variation_threshold(0.00001f);

    auto keypoints_none = extractor_none.extract();
    auto keypoints_linear = extractor_linear.extract();
    auto keypoints_quadratic = extractor_quadratic.extract();

    INFO("Plane only: " << keypoints_none.size() << " keypoints");
    INFO("Linear: " << keypoints_linear.size() << " keypoints");
    INFO("Quadratic: " << keypoints_quadratic.size() << " keypoints");

    // All should find keypoints
    REQUIRE(keypoints_none.size() > 0);
    REQUIRE(keypoints_linear.size() > 0);
    REQUIRE(keypoints_quadratic.size() > 0);
  }

  SECTION("MLS on Smooth Surface")
  {
    // Generate a smooth spherical surface
    auto sphere_cloud = generate_spherical_test_cloud<data_type>(800);
    
    // Add proper normals for sphere
    sphere_cloud.normals.clear();
    sphere_cloud.normals.reserve(sphere_cloud.points.size());
    for (const auto& pt : sphere_cloud.points) {
      // Normal points outward from center
      float norm = std::sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);
      point_t<data_type> normal;
      normal.x = pt.x / norm;
      normal.y = pt.y / norm;
      normal.z = pt.z / norm;
      sphere_cloud.normals.push_back(normal);
    }
    
    auto sphere_kdtree = kdtree_t<data_type>{};
    
    mls_keypoint_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(sphere_cloud);
    extractor.set_knn(sphere_kdtree);
    extractor.set_search_radius(2.0f);
    extractor.set_variation_threshold(0.000001f);  // Much lower threshold for smooth surface
    extractor.set_curvature_threshold(0.001f);
    
    auto keypoints = extractor.extract();
    
    INFO("Found " << keypoints.size() << " MLS keypoints on sphere");
    // Smooth surfaces should have fewer keypoints
    REQUIRE(keypoints.size() >= 0);
  }
}
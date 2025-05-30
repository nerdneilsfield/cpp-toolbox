#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>

#include <algorithm>
#include <chrono>
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

// Helper function to create a simple test cloud
template<typename T>
point_cloud_t<T> create_test_cloud()
{
  point_cloud_t<T> cloud;
  
  // Create a simple 3x3x3 grid
  for (int x = 0; x < 3; ++x)
  {
    for (int y = 0; y < 3; ++y)
    {
      for (int z = 0; z < 3; ++z)
      {
        point_t<T> pt;
        pt.x = static_cast<T>(x);
        pt.y = static_cast<T>(y);
        pt.z = static_cast<T>(z);
        cloud.points.push_back(pt);
      }
    }
  }
  
  return cloud;
}

TEST_CASE("KNN Algorithms - Basic Functionality", "[pcl][knn]")
{
  using scalar_t = float;
  
  auto cloud = create_test_cloud<scalar_t>();
  point_t<scalar_t> query_point;
  query_point.x = 1.5f;
  query_point.y = 1.5f;
  query_point.z = 1.5f;

  SECTION("Brute Force KNN - k-neighbors")
  {
    bfknn_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
    REQUIRE(distances.size() == 5);
    
    // Check that distances are sorted
    for (std::size_t i = 1; i < distances.size(); ++i)
    {
      REQUIRE(distances[i] >= distances[i-1]);
    }
    
    // The closest point should be (1,1,1) or (2,2,2) with distance sqrt(0.75)
    REQUIRE_THAT(distances[0], WithinRel(std::sqrt(0.75f), 0.001f));
  }

  SECTION("Brute Force KNN - radius neighbors")
  {
    bfknn_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    scalar_t radius = 1.0f;
    REQUIRE(knn.radius_neighbors(query_point, radius, indices, distances));
    
    // All returned points should be within radius
    for (auto dist : distances)
    {
      REQUIRE(dist <= radius);
    }
    
    // Check that distances are sorted
    for (std::size_t i = 1; i < distances.size(); ++i)
    {
      REQUIRE(distances[i] >= distances[i-1]);
    }
  }

  SECTION("Parallel Brute Force KNN - k-neighbors")
  {
    bfknn_parallel_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
    REQUIRE(distances.size() == 5);
    
    // Check that distances are sorted
    for (std::size_t i = 1; i < distances.size(); ++i)
    {
      REQUIRE(distances[i] >= distances[i-1]);
    }
  }

  SECTION("KD-tree KNN - k-neighbors")
  {
    kdtree_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
    REQUIRE(distances.size() == 5);
    
    // Check that distances are sorted
    for (std::size_t i = 1; i < distances.size(); ++i)
    {
      REQUIRE(distances[i] >= distances[i-1]);
    }
    
    // The closest point should be (1,1,1) or (2,2,2) with distance sqrt(0.75)
    REQUIRE_THAT(distances[0], WithinRel(std::sqrt(0.75f), 0.001f));
  }

  SECTION("KD-tree KNN - radius neighbors")
  {
    kdtree_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    scalar_t radius = 1.0f;
    REQUIRE(knn.radius_neighbors(query_point, radius, indices, distances));
    
    // All returned points should be within radius
    for (auto dist : distances)
    {
      REQUIRE(dist <= radius);
    }
    
    // Check that distances are sorted
    for (std::size_t i = 1; i < distances.size(); ++i)
    {
      REQUIRE(distances[i] >= distances[i-1]);
    }
  }
}

TEST_CASE("KNN Algorithms - Consistency Check", "[pcl][knn]")
{
  using scalar_t = float;
  
  auto cloud = generate_random_cloud<scalar_t>(1000);
  point_t<scalar_t> query_point;
  query_point.x = 0.0f;
  query_point.y = 0.0f;
  query_point.z = 0.0f;

  bfknn_t<scalar_t> bf_knn;
  bfknn_parallel_t<scalar_t> bf_parallel_knn;
  kdtree_t<scalar_t> kd_knn;

  bf_knn.set_input(cloud);
  bf_parallel_knn.set_input(cloud);
  kd_knn.set_input(cloud);

  SECTION("k-neighbors consistency")
  {
    const std::size_t k = 10;
    
    std::vector<std::size_t> bf_indices, bf_parallel_indices, kd_indices;
    std::vector<scalar_t> bf_distances, bf_parallel_distances, kd_distances;

    REQUIRE(bf_knn.kneighbors(query_point, k, bf_indices, bf_distances));
    REQUIRE(bf_parallel_knn.kneighbors(query_point, k, bf_parallel_indices, bf_parallel_distances));
    REQUIRE(kd_knn.kneighbors(query_point, k, kd_indices, kd_distances));

    // All should return k neighbors
    REQUIRE(bf_indices.size() == k);
    REQUIRE(bf_parallel_indices.size() == k);
    REQUIRE(kd_indices.size() == k);

    // The results should be the same (allowing for different ordering of equidistant points)
    std::sort(bf_indices.begin(), bf_indices.end());
    std::sort(bf_parallel_indices.begin(), bf_parallel_indices.end());
    std::sort(kd_indices.begin(), kd_indices.end());

    REQUIRE(bf_indices == bf_parallel_indices);
    REQUIRE(bf_indices == kd_indices);
  }

  SECTION("radius neighbors consistency")
  {
    const scalar_t radius = 2.0f;
    
    std::vector<std::size_t> bf_indices, bf_parallel_indices, kd_indices;
    std::vector<scalar_t> bf_distances, bf_parallel_distances, kd_distances;

    REQUIRE(bf_knn.radius_neighbors(query_point, radius, bf_indices, bf_distances));
    REQUIRE(bf_parallel_knn.radius_neighbors(query_point, radius, bf_parallel_indices, bf_parallel_distances));
    REQUIRE(kd_knn.radius_neighbors(query_point, radius, kd_indices, kd_distances));

    // All should return the same number of neighbors
    REQUIRE(bf_indices.size() == bf_parallel_indices.size());
    REQUIRE(bf_indices.size() == kd_indices.size());

    // The results should be the same (allowing for different ordering)
    std::sort(bf_indices.begin(), bf_indices.end());
    std::sort(bf_parallel_indices.begin(), bf_parallel_indices.end());
    std::sort(kd_indices.begin(), kd_indices.end());

    REQUIRE(bf_indices == bf_parallel_indices);
    REQUIRE(bf_indices == kd_indices);
  }
}

TEST_CASE("KNN Algorithms - Edge Cases", "[pcl][knn]")
{
  using scalar_t = float;

  SECTION("Empty cloud")
  {
    point_cloud_t<scalar_t> empty_cloud;
    point_t<scalar_t> query_point{0, 0, 0};

    bfknn_t<scalar_t> bf_knn;
    kdtree_t<scalar_t> kd_knn;

    REQUIRE(bf_knn.set_input(empty_cloud) == 0);
    REQUIRE(kd_knn.set_input(empty_cloud) == 0);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;

    REQUIRE_FALSE(bf_knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE_FALSE(kd_knn.kneighbors(query_point, 5, indices, distances));
  }

  SECTION("Single point cloud")
  {
    point_cloud_t<scalar_t> cloud;
    point_t<scalar_t> pt{1, 2, 3};
    cloud.points.push_back(pt);

    point_t<scalar_t> query_point{0, 0, 0};

    bfknn_t<scalar_t> bf_knn;
    kdtree_t<scalar_t> kd_knn;

    REQUIRE(bf_knn.set_input(cloud) == 1);
    REQUIRE(kd_knn.set_input(cloud) == 1);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;

    REQUIRE(bf_knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 1);  // Only one point available
    REQUIRE(indices[0] == 0);

    indices.clear();
    distances.clear();

    REQUIRE(kd_knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 1);
    REQUIRE(indices[0] == 0);
  }

  SECTION("k larger than cloud size")
  {
    auto cloud = create_test_cloud<scalar_t>();
    point_t<scalar_t> query_point{0, 0, 0};

    bfknn_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;

    REQUIRE(knn.kneighbors(query_point, 100, indices, distances));
    REQUIRE(indices.size() == 27);  // Should return all points
  }
}

TEST_CASE("KNN Algorithms - Different Metrics", "[pcl][knn]")
{
  using scalar_t = float;
  
  auto cloud = create_test_cloud<scalar_t>();
  point_t<scalar_t> query_point{1.5f, 1.5f, 1.5f};

  SECTION("Manhattan distance")
  {
    bfknn_t<scalar_t> knn;
    knn.set_input(cloud);
    knn.set_metric(metric_type_t::manhattan);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    
    // Verify Manhattan distance calculation
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
      auto& pt = cloud.points[indices[i]];
      scalar_t expected_dist = std::abs(pt.x - query_point.x) + 
                              std::abs(pt.y - query_point.y) + 
                              std::abs(pt.z - query_point.z);
      REQUIRE_THAT(distances[i], WithinRel(expected_dist, 0.001f));
    }
  }

  SECTION("Chebyshev distance")
  {
    bfknn_t<scalar_t> knn;
    knn.set_input(cloud);
    knn.set_metric(metric_type_t::chebyshev);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    
    // Verify Chebyshev distance calculation
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
      auto& pt = cloud.points[indices[i]];
      scalar_t expected_dist = std::max({std::abs(pt.x - query_point.x),
                                        std::abs(pt.y - query_point.y),
                                        std::abs(pt.z - query_point.z)});
      REQUIRE_THAT(distances[i], WithinRel(expected_dist, 0.001f));
    }
  }
}

TEST_CASE("KNN Algorithms - Performance Comparison", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> sizes = {100, 1000, 10000};
  
  for (auto size : sizes)
  {
    INFO("Testing with " << size << " points");
    
    auto cloud = generate_random_cloud<scalar_t>(size);
    point_t<scalar_t> query_point{0, 0, 0};
    
    bfknn_t<scalar_t> bf_knn;
    bfknn_parallel_t<scalar_t> bf_parallel_knn;
    kdtree_t<scalar_t> kd_knn;
    
    // Time setup
    auto start = std::chrono::high_resolution_clock::now();
    bf_knn.set_input(cloud);
    auto bf_setup_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    bf_parallel_knn.set_input(cloud);
    auto bf_parallel_setup_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    kd_knn.set_input(cloud);
    auto kd_setup_time = std::chrono::high_resolution_clock::now() - start;
    
    // Time queries
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    const std::size_t k = 10;
    const int num_queries = 100;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_queries; ++i)
    {
      bf_knn.kneighbors(query_point, k, indices, distances);
    }
    auto bf_query_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_queries; ++i)
    {
      bf_parallel_knn.kneighbors(query_point, k, indices, distances);
    }
    auto bf_parallel_query_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_queries; ++i)
    {
      kd_knn.kneighbors(query_point, k, indices, distances);
    }
    auto kd_query_time = std::chrono::high_resolution_clock::now() - start;
    
    // KD-tree should be faster for queries on larger datasets
    if (size >= 1000)
    {
      REQUIRE(kd_query_time < bf_query_time);
    }
  }
}
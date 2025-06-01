#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/angular_metrics.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::metrics;
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
    REQUIRE(knn.set_input(cloud.points) == 27);

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
    REQUIRE(knn.set_input(cloud.points) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    scalar_t radius = 2.0f;
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

  SECTION("KD-Tree KNN - k-neighbors")
  {
    kdtree_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud.points) == 27);

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

  SECTION("Parallel Brute Force KNN")
  {
    bfknn_parallel_t<scalar_t> knn;
    REQUIRE(knn.set_input(cloud.points) == 27);

    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
    REQUIRE(distances.size() == 5);
  }
}

TEST_CASE("KNN Algorithms - Different Metrics", "[pcl][knn]")
{
  using scalar_t = float;
  
  auto cloud = create_test_cloud<scalar_t>();
  point_t<scalar_t> query_point;
  query_point.x = 1.5f;
  query_point.y = 1.5f;
  query_point.z = 1.5f;


  SECTION("Generic interface - Compile-time metrics")
  {
    // Convert point cloud to vector for generic interface
    std::vector<point_t<scalar_t>> points(cloud.points.begin(), cloud.points.end());

    // Test with L2 metric
    {
      bfknn_generic_t<point_t<scalar_t>, L2Metric<scalar_t>> knn_l2;
      REQUIRE(knn_l2.set_input(points) == 27);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      
      REQUIRE(knn_l2.kneighbors(query_point, 5, indices, distances));
      REQUIRE(indices.size() == 5);
      REQUIRE_THAT(distances[0], WithinRel(std::sqrt(0.75f), 0.001f));
    }

    // Test with L1 metric
    {
      bfknn_generic_t<point_t<scalar_t>, L1Metric<scalar_t>> knn_l1;
      REQUIRE(knn_l1.set_input(points) == 27);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      
      REQUIRE(knn_l1.kneighbors(query_point, 5, indices, distances));
      REQUIRE(indices.size() == 5);
      // For L1, distance to (1,1,1) is |1.5-1| + |1.5-1| + |1.5-1| = 1.5
      REQUIRE_THAT(distances[0], WithinRel(1.5f, 0.001f));
    }

    // Test with Linf metric
    {
      bfknn_generic_t<point_t<scalar_t>, LinfMetric<scalar_t>> knn_linf;
      REQUIRE(knn_linf.set_input(points) == 27);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      
      REQUIRE(knn_linf.kneighbors(query_point, 5, indices, distances));
      REQUIRE(indices.size() == 5);
      // For Linf, distance to (1,1,1) is max(|1.5-1|, |1.5-1|, |1.5-1|) = 0.5
      REQUIRE_THAT(distances[0], WithinRel(0.5f, 0.001f));
    }

    // Test with Cosine metric
    {
      bfknn_generic_t<point_t<scalar_t>, CosineMetric<scalar_t>> knn_cosine;
      REQUIRE(knn_cosine.set_input(points) == 27);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      
      REQUIRE(knn_cosine.kneighbors(query_point, 5, indices, distances));
      REQUIRE(indices.size() == 5);
    }
  }

  SECTION("Generic interface - Runtime metrics")
  {
    std::vector<point_t<scalar_t>> points(cloud.points.begin(), cloud.points.end());
    
    bfknn_generic_t<point_t<scalar_t>, L2Metric<scalar_t>> knn;
    REQUIRE(knn.set_input(points) == 27);

    // Create runtime metric using factory
    auto metric_l1 = MetricFactory<scalar_t>::instance().create("l1");
    knn.set_metric(std::move(metric_l1));
    
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
    
    // Test with different runtime metric
    auto metric_linf = MetricFactory<scalar_t>::instance().create("linf");
    knn.set_metric(std::move(metric_linf));
    
    indices.clear();
    distances.clear();
    
    REQUIRE(knn.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
  }
}

TEST_CASE("KNN Algorithms - KDTree Metric Fallback", "[pcl][knn]")
{
  using scalar_t = float;
  
  auto cloud = create_test_cloud<scalar_t>();
  point_t<scalar_t> query_point;
  query_point.x = 1.5f;
  query_point.y = 1.5f;
  query_point.z = 1.5f;

  kdtree_t<scalar_t> kdtree;
  bfknn_t<scalar_t> bfknn;
  
  REQUIRE(kdtree.set_input(cloud.points) == 27);
  REQUIRE(bfknn.set_input(cloud.points) == 27);

  SECTION("KDTree supports Euclidean metric")
  {
    // KDTree uses L2 metric by default
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(kdtree.kneighbors(query_point, 5, indices, distances));
    REQUIRE(indices.size() == 5);
  }

  SECTION("KDTree falls back to brute-force for unsupported metrics")
  {
    // Set unsupported metric using runtime metric
    auto metric_l1 = MetricFactory<scalar_t>::instance().create("l1");
    kdtree.set_metric(std::move(metric_l1));
    
    // Create a generic bfknn with L1 metric for comparison
    bfknn_generic_t<point_t<scalar_t>, L1Metric<scalar_t>> bfknn_l1;
    REQUIRE(bfknn_l1.set_input(cloud.points) == 27);
    
    std::vector<std::size_t> kdtree_indices, bfknn_indices;
    std::vector<scalar_t> kdtree_distances, bfknn_distances;
    
    REQUIRE(kdtree.kneighbors(query_point, 5, kdtree_indices, kdtree_distances));
    REQUIRE(bfknn_l1.kneighbors(query_point, 5, bfknn_indices, bfknn_distances));
    
    // Results should be the same (indices might be in different order for equal distances)
    REQUIRE(kdtree_distances.size() == bfknn_distances.size());
    
    // Check distances match
    for (std::size_t i = 0; i < kdtree_distances.size(); ++i)
    {
      REQUIRE_THAT(kdtree_distances[i], WithinRel(bfknn_distances[i], 0.001f));
    }
  }
}

TEST_CASE("KNN Algorithms - Performance Comparison", "[pcl][knn][!benchmark]")
{
  using scalar_t = float;
  
  const std::size_t num_points = 10000;
  auto cloud = generate_random_cloud<scalar_t>(num_points);
  auto query_cloud = generate_random_cloud<scalar_t>(100);

  SECTION("Compare algorithms")
  {
    bfknn_t<scalar_t> bfknn;
    bfknn_parallel_t<scalar_t> bfknn_parallel;
    kdtree_t<scalar_t> kdtree;

    REQUIRE(bfknn.set_input(cloud.points) == num_points);
    REQUIRE(bfknn_parallel.set_input(cloud.points) == num_points);
    REQUIRE(kdtree.set_input(cloud.points) == num_points);

    const std::size_t k = 10;
    
    // Test all algorithms produce same results for first query
    std::vector<std::size_t> indices_bf, indices_parallel, indices_kd;
    std::vector<scalar_t> distances_bf, distances_parallel, distances_kd;
    
    auto query_point = query_cloud.points[0];
    
    REQUIRE(bfknn.kneighbors(query_point, k, indices_bf, distances_bf));
    REQUIRE(bfknn_parallel.kneighbors(query_point, k, indices_parallel, distances_parallel));
    REQUIRE(kdtree.kneighbors(query_point, k, indices_kd, distances_kd));
    
    // All should find same number of neighbors
    REQUIRE(indices_bf.size() == k);
    REQUIRE(indices_parallel.size() == k);
    REQUIRE(indices_kd.size() == k);
    
    // Distances should be approximately the same (allowing for floating point differences)
    for (std::size_t i = 0; i < k; ++i)
    {
      REQUIRE_THAT(distances_bf[i], WithinRel(distances_parallel[i], 0.001f));
      REQUIRE_THAT(distances_bf[i], WithinRel(distances_kd[i], 0.001f));
    }
  }
}

TEST_CASE("KNN Algorithms - Edge Cases", "[pcl][knn]")
{
  using scalar_t = float;

  SECTION("Empty cloud")
  {
    point_cloud_t<scalar_t> empty_cloud;
    bfknn_t<scalar_t> knn;
    
    REQUIRE(knn.set_input(empty_cloud.points) == 0);
    
    point_t<scalar_t> query;
    query.x = query.y = query.z = 0;
    
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE_FALSE(knn.kneighbors(query, 5, indices, distances));
  }

  SECTION("K larger than cloud size")
  {
    auto cloud = create_test_cloud<scalar_t>();
    bfknn_t<scalar_t> knn;
    
    REQUIRE(knn.set_input(cloud.points) == 27);
    
    point_t<scalar_t> query;
    query.x = query.y = query.z = 1.5f;
    
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE(knn.kneighbors(query, 100, indices, distances));
    REQUIRE(indices.size() == 27);  // Should return all points
    REQUIRE(distances.size() == 27);
  }

  SECTION("Zero radius")
  {
    auto cloud = create_test_cloud<scalar_t>();
    bfknn_t<scalar_t> knn;
    
    REQUIRE(knn.set_input(cloud.points) == 27);
    
    point_t<scalar_t> query;
    query.x = query.y = query.z = 1.5f;
    
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    REQUIRE_FALSE(knn.radius_neighbors(query, 0, indices, distances));
  }
}
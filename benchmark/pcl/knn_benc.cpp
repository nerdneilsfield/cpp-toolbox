#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/angular_metrics.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>

#include <iostream>
#include <memory>
#include <iomanip>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::metrics;

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

// Generate query points
template<typename T>
std::vector<point_t<T>> generate_query_points(std::size_t num_queries, T min_val = -100, T max_val = 100)
{
  std::vector<point_t<T>> queries;
  queries.reserve(num_queries);

  toolbox::utils::random_t rng;
  
  for (std::size_t i = 0; i < num_queries; ++i)
  {
    point_t<T> pt;
    pt.x = rng.random<T>(min_val, max_val);
    pt.y = rng.random<T>(min_val, max_val);
    pt.z = rng.random<T>(min_val, max_val);
    queries.push_back(pt);
  }

  return queries;
}

TEST_CASE("KNN Benchmark - Setup Time", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> cloud_sizes = {1000, 10000, 50000, 100000};
  
  for (auto cloud_size : cloud_sizes)
  {
    auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
    
    BENCHMARK("BruteForce Setup - " + std::to_string(cloud_size) + " points")
    {
      bfknn_t<scalar_t> knn;
      return knn.set_input(cloud);
    };
    
    BENCHMARK("BruteForce Parallel Setup - " + std::to_string(cloud_size) + " points")
    {
      bfknn_parallel_t<scalar_t> knn;
      return knn.set_input(cloud);
    };
    
    BENCHMARK("KDTree Setup - " + std::to_string(cloud_size) + " points")
    {
      kdtree_t<scalar_t> knn;
      return knn.set_input(cloud);
    };
  }
}

TEST_CASE("KNN Benchmark - K-Neighbors Query", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> cloud_sizes = {1000, 10000, 50000, 100000};
  std::vector<std::size_t> k_values = {1, 5, 10, 50};
  
  for (auto cloud_size : cloud_sizes)
  {
    auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
    auto queries = generate_query_points<scalar_t>(100);
    
    // Setup KNN algorithms
    auto bfknn = std::make_unique<bfknn_t<scalar_t>>();
    auto bfknn_parallel = std::make_unique<bfknn_parallel_t<scalar_t>>();
    auto kdtree = std::make_unique<kdtree_t<scalar_t>>();
    
    bfknn->set_input(cloud);
    bfknn_parallel->set_input(cloud);
    kdtree->set_input(cloud);
    
    for (auto k : k_values)
    {
      BENCHMARK("BruteForce Query - " + std::to_string(cloud_size) + " points, k=" + std::to_string(k))
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        for (const auto& query : queries)
        {
          bfknn->kneighbors(query, k, indices, distances);
        }
      };
      
      BENCHMARK("BruteForce Parallel Query - " + std::to_string(cloud_size) + " points, k=" + std::to_string(k))
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        for (const auto& query : queries)
        {
          bfknn_parallel->kneighbors(query, k, indices, distances);
        }
      };
      
      BENCHMARK("KDTree Query - " + std::to_string(cloud_size) + " points, k=" + std::to_string(k))
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        for (const auto& query : queries)
        {
          kdtree->kneighbors(query, k, indices, distances);
        }
      };
    }
  }
}

TEST_CASE("KNN Benchmark - Different Metrics", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  const std::size_t cloud_size = 10000;
  const std::size_t num_queries = 100;
  const std::size_t k = 10;
  
  auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
  auto queries = generate_query_points<scalar_t>(num_queries);
  
  
  SECTION("Generic interface metrics")
  {
    // Convert to vector for generic interface
    std::vector<point_t<scalar_t>> points(cloud.points.begin(), cloud.points.end());
    
    BENCHMARK("Generic L2 Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, L2Metric<scalar_t>> knn;
      knn.set_input(points);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
    
    BENCHMARK("Generic L1 Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, L1Metric<scalar_t>> knn;
      knn.set_input(points);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
    
    BENCHMARK("Generic Linf Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, LinfMetric<scalar_t>> knn;
      knn.set_input(points);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
    
    BENCHMARK("Generic Cosine Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, CosineMetric<scalar_t>> knn;
      knn.set_input(points);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
    
    BENCHMARK("Generic Angular Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, AngularMetric<scalar_t>> knn;
      knn.set_input(points);
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
  }
  
  SECTION("Runtime metrics")
  {
    std::vector<point_t<scalar_t>> points(cloud.points.begin(), cloud.points.end());
    
    BENCHMARK("Runtime L2 Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, L2Metric<scalar_t>> knn;
      knn.set_input(points);
      
      auto metric = MetricFactory<scalar_t>::instance().create("L2");
      knn.set_metric(std::move(metric));
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
    
    BENCHMARK("Runtime L1 Metric")
    {
      bfknn_generic_t<point_t<scalar_t>, L2Metric<scalar_t>> knn;
      knn.set_input(points);
      
      auto metric = MetricFactory<scalar_t>::instance().create("L1");
      knn.set_metric(std::move(metric));
      
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      for (const auto& query : queries)
      {
        knn.kneighbors(query, k, indices, distances);
      }
    };
  }
}

TEST_CASE("KNN Benchmark - Radius Neighbors", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  const std::size_t cloud_size = 10000;
  const std::size_t num_queries = 100;
  const scalar_t radius = 10.0f;
  
  auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
  auto queries = generate_query_points<scalar_t>(num_queries);
  
  // Setup KNN algorithms
  auto bfknn = std::make_unique<bfknn_t<scalar_t>>();
  auto bfknn_parallel = std::make_unique<bfknn_parallel_t<scalar_t>>();
  auto kdtree = std::make_unique<kdtree_t<scalar_t>>();
  
  bfknn->set_input(cloud.points);
  bfknn_parallel->set_input(cloud.points);
  kdtree->set_input(cloud.points);
  
  BENCHMARK("BruteForce Radius Search - " + std::to_string(cloud_size) + " points")
  {
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      bfknn->radius_neighbors(query, radius, indices, distances);
    }
  };
  
  BENCHMARK("BruteForce Parallel Radius Search - " + std::to_string(cloud_size) + " points")
  {
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      bfknn_parallel->radius_neighbors(query, radius, indices, distances);
    }
  };
  
  BENCHMARK("KDTree Radius Search - " + std::to_string(cloud_size) + " points")
  {
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      kdtree->radius_neighbors(query, radius, indices, distances);
    }
  };
}

TEST_CASE("KNN Benchmark - Parallel Scaling", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  const std::size_t cloud_size = 100000;
  const std::size_t num_queries = 100;
  const std::size_t k = 10;
  
  auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
  auto queries = generate_query_points<scalar_t>(num_queries);
  
  auto bfknn_parallel = std::make_unique<bfknn_parallel_t<scalar_t>>();
  bfknn_parallel->set_input(cloud.points);
  
  BENCHMARK("Parallel Enabled - " + std::to_string(cloud_size) + " points")
  {
    bfknn_parallel->enable_parallel(true);
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      bfknn_parallel->kneighbors(query, k, indices, distances);
    }
  };
  
  BENCHMARK("Parallel Disabled - " + std::to_string(cloud_size) + " points")
  {
    bfknn_parallel->enable_parallel(false);
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      bfknn_parallel->kneighbors(query, k, indices, distances);
    }
  };
}

TEST_CASE("KNN Benchmark - KDTree Fallback for Metrics", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  const std::size_t cloud_size = 10000;
  const std::size_t num_queries = 100;
  const std::size_t k = 10;
  
  auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
  auto queries = generate_query_points<scalar_t>(num_queries);
  
  auto kdtree = std::make_unique<kdtree_t<scalar_t>>();
  kdtree->set_input(cloud.points);
  
  BENCHMARK("KDTree with Euclidean (Native)")
  {
    // KDTree uses L2 metric by default
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      kdtree->kneighbors(query, k, indices, distances);
    }
  };
  
  BENCHMARK("KDTree with L1 (Fallback)")
  {
    auto metric_l1 = MetricFactory<scalar_t>::instance().create("l1");
    kdtree->set_metric(std::move(metric_l1));
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    for (const auto& query : queries)
    {
      kdtree->kneighbors(query, k, indices, distances);
    }
  };
}
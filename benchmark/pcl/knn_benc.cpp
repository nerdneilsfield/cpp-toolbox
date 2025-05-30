#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

#include <iostream>
#include <memory>
#include <iomanip>

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
    
    // Setup algorithms once
    auto bf_knn = std::make_unique<bfknn_t<scalar_t>>();
    auto bf_parallel_knn = std::make_unique<bfknn_parallel_t<scalar_t>>();
    auto kd_knn = std::make_unique<kdtree_t<scalar_t>>();
    
    bf_knn->set_input(cloud);
    bf_parallel_knn->set_input(cloud);
    kd_knn->set_input(cloud);
    
    for (auto k : k_values)
    {
      // Skip if k > cloud_size
      if (k > cloud_size) continue;
      
      std::string bench_name = std::to_string(cloud_size) + " points, k=" + std::to_string(k);
      
      BENCHMARK("BruteForce kNN - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          bf_knn->kneighbors(query, k, indices, distances);
        }
      };
      
      BENCHMARK("BruteForce Parallel kNN - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          bf_parallel_knn->kneighbors(query, k, indices, distances);
        }
      };
      
      BENCHMARK("KDTree kNN - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          kd_knn->kneighbors(query, k, indices, distances);
        }
      };
    }
  }
}

TEST_CASE("KNN Benchmark - Radius Search", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> cloud_sizes = {1000, 10000, 50000, 100000};
  std::vector<scalar_t> radii = {1.0f, 5.0f, 10.0f, 20.0f};
  
  for (auto cloud_size : cloud_sizes)
  {
    auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
    auto queries = generate_query_points<scalar_t>(100);
    
    // Setup algorithms once
    auto bf_knn = std::make_unique<bfknn_t<scalar_t>>();
    auto bf_parallel_knn = std::make_unique<bfknn_parallel_t<scalar_t>>();
    auto kd_knn = std::make_unique<kdtree_t<scalar_t>>();
    
    bf_knn->set_input(cloud);
    bf_parallel_knn->set_input(cloud);
    kd_knn->set_input(cloud);
    
    for (auto radius : radii)
    {
      std::string bench_name = std::to_string(cloud_size) + " points, r=" + std::to_string(radius);
      
      BENCHMARK("BruteForce Radius - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          bf_knn->radius_neighbors(query, radius, indices, distances);
        }
      };
      
      BENCHMARK("BruteForce Parallel Radius - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          bf_parallel_knn->radius_neighbors(query, radius, indices, distances);
        }
      };
      
      BENCHMARK("KDTree Radius - " + bench_name)
      {
        std::vector<std::size_t> indices;
        std::vector<scalar_t> distances;
        
        for (const auto& query : queries)
        {
          kd_knn->radius_neighbors(query, radius, indices, distances);
        }
      };
    }
  }
}

TEST_CASE("KNN Benchmark - Different Metrics", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  const std::size_t cloud_size = 50000;
  const std::size_t k = 10;
  
  auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
  auto queries = generate_query_points<scalar_t>(100);
  
  std::vector<metric_type_t> metrics = {
    metric_type_t::euclidean,
    metric_type_t::manhattan,
    metric_type_t::chebyshev,
    metric_type_t::minkowski
  };
  
  for (auto metric : metrics)
  {
    auto bf_knn = std::make_unique<bfknn_t<scalar_t>>();
    bf_knn->set_input(cloud);
    bf_knn->set_metric(metric);
    
    std::string metric_name;
    switch (metric)
    {
      case metric_type_t::euclidean: metric_name = "Euclidean"; break;
      case metric_type_t::manhattan: metric_name = "Manhattan"; break;
      case metric_type_t::chebyshev: metric_name = "Chebyshev"; break;
      case metric_type_t::minkowski: metric_name = "Minkowski"; break;
    }
    
    BENCHMARK("BruteForce kNN - " + metric_name + " metric")
    {
      std::vector<std::size_t> indices;
      std::vector<scalar_t> distances;
      
      for (const auto& query : queries)
      {
        bf_knn->kneighbors(query, k, indices, distances);
      }
    };
  }
}

TEST_CASE("KNN Benchmark - Parallel Speedup Analysis", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> cloud_sizes = {10000, 50000, 100000, 200000};
  const std::size_t k = 10;
  
  std::cout << "\n=== Parallel Speedup Analysis ===\n";
  std::cout << "Cloud Size | Serial Time | Parallel Time | Speedup\n";
  std::cout << "-----------|-------------|---------------|--------\n";
  
  for (auto cloud_size : cloud_sizes)
  {
    auto cloud = generate_benchmark_cloud<scalar_t>(cloud_size);
    auto queries = generate_query_points<scalar_t>(100);
    
    auto bf_knn = std::make_unique<bfknn_t<scalar_t>>();
    auto bf_parallel_knn = std::make_unique<bfknn_parallel_t<scalar_t>>();
    
    bf_knn->set_input(cloud);
    bf_parallel_knn->set_input(cloud);
    
    // Time serial version
    toolbox::utils::stop_watch_timer_t timer;
    std::vector<std::size_t> indices;
    std::vector<scalar_t> distances;
    
    timer.start();
    for (const auto& query : queries)
    {
      bf_knn->kneighbors(query, k, indices, distances);
    }
    timer.stop();
    double serial_time = timer.elapsed_time();
    
    // Time parallel version
    timer.start();
    for (const auto& query : queries)
    {
      bf_parallel_knn->kneighbors(query, k, indices, distances);
    }
    timer.stop();
    double parallel_time = timer.elapsed_time();
    
    double speedup = serial_time / parallel_time;
    
    std::cout << std::setw(10) << cloud_size << " | "
              << std::setw(11) << std::fixed << std::setprecision(3) << serial_time << " | "
              << std::setw(13) << std::fixed << std::setprecision(3) << parallel_time << " | "
              << std::setw(7) << std::fixed << std::setprecision(2) << speedup << "x\n";
  }
  std::cout << "\n";
}

TEST_CASE("KNN Benchmark - Memory Usage Patterns", "[pcl][knn][benchmark]")
{
  using scalar_t = float;
  
  std::vector<std::size_t> cloud_sizes = {1000, 10000, 100000};
  
  std::cout << "\n=== Memory Usage Analysis ===\n";
  std::cout << "Algorithm | Cloud Size | Approx Memory (MB)\n";
  std::cout << "----------|------------|------------------\n";
  
  for (auto cloud_size : cloud_sizes)
  {
    // Calculate approximate memory usage
    std::size_t point_size = sizeof(point_t<scalar_t>);
    std::size_t cloud_memory = cloud_size * point_size / (1024 * 1024);
    
    // BruteForce: just stores the cloud
    std::cout << "BruteForce | " << std::setw(10) << cloud_size 
              << " | " << std::setw(17) << cloud_memory << "\n";
    
    // KDTree: stores cloud + tree structure (roughly 2x)
    std::cout << "KDTree     | " << std::setw(10) << cloud_size 
              << " | " << std::setw(17) << cloud_memory * 2 << "\n";
  }
  std::cout << "\n";
}
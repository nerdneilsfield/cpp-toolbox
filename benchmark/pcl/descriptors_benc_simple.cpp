#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/shot_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/pfh_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>

using namespace toolbox::pcl;
using namespace toolbox::types;

namespace
{

template<typename DataType>
point_cloud_t<DataType> generate_benchmark_cloud(std::size_t num_points)
{
  point_cloud_t<DataType> cloud;
  cloud.points.resize(num_points);
  toolbox::utils::random_t rng;
  
  // Generate clustered point cloud for more realistic benchmarking
  std::size_t num_clusters = std::max<std::size_t>(10, num_points / 10000);
  std::size_t points_per_cluster = num_points / num_clusters;
  
  for (std::size_t c = 0; c < num_clusters; ++c)
  {
    // Random cluster center
    point_t<DataType> center(rng.random<DataType>(-100.0, 100.0), 
                             rng.random<DataType>(-100.0, 100.0), 
                             rng.random<DataType>(-100.0, 100.0));
    
    for (std::size_t i = 0; i < points_per_cluster; ++i)
    {
      std::size_t idx = c * points_per_cluster + i;
      if (idx < num_points)
      {
        cloud.points[idx] = center;
        cloud.points[idx].x += rng.random<DataType>(-2.0, 2.0);
        cloud.points[idx].y += rng.random<DataType>(-2.0, 2.0);
        cloud.points[idx].z += rng.random<DataType>(-2.0, 2.0);
      }
    }
  }
  
  return cloud;
}

std::vector<std::size_t> generate_keypoint_indices(std::size_t cloud_size, std::size_t num_keypoints)
{
  std::vector<std::size_t> indices;
  indices.reserve(num_keypoints);
  
  if (num_keypoints >= cloud_size)
  {
    // Use all points as keypoints
    for (std::size_t i = 0; i < cloud_size; ++i)
    {
      indices.push_back(i);
    }
  }
  else
  {
    // Sample evenly
    std::size_t step = cloud_size / num_keypoints;
    for (std::size_t i = 0; i < num_keypoints; ++i)
    {
      indices.push_back(i * step);
    }
  }
  
  return indices;
}

} // anonymous namespace

TEST_CASE("Descriptor extraction performance", "[pcl][descriptors][benchmark]")
{
  using data_type = float;
  
  SECTION("FPFH small cloud (1K points, 10 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(1000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 10);
    
    BENCHMARK("FPFH extraction")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("FPFH medium cloud (10K points, 100 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100);
    
    BENCHMARK("FPFH extraction")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("FPFH large cloud (50K points, 500 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(50000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 500);
    
    BENCHMARK("FPFH extraction sequential")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      extractor.enable_parallel(false);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("FPFH extraction parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      extractor.enable_parallel(true);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("FPFH very large cloud (50K points, 2000 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(50000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 2000);
    
    BENCHMARK("FPFH extraction parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      extractor.enable_parallel(true);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("SHOT medium cloud (10K points, 100 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100);
    
    BENCHMARK("SHOT extraction")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(100);
      
      std::vector<shot_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("SHOT large cloud (50K points, 500 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(50000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 500);
    
    BENCHMARK("SHOT extraction sequential")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(100);
      extractor.enable_parallel(false);
      
      std::vector<shot_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("SHOT extraction parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(100);
      extractor.enable_parallel(true);
      
      std::vector<shot_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("PFH small cloud (1K points, 10 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(1000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 10);
    
    BENCHMARK("PFH extraction")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(30);
      
      std::vector<pfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("PFH medium cloud (10K points, 100 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100);
    
    BENCHMARK("PFH extraction sequential")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(30);
      extractor.enable_parallel(false);
      
      std::vector<pfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("PFH extraction parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(30);
      extractor.enable_parallel(true);
      
      std::vector<pfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
  
  SECTION("Parallel speedup comparison (20K points, 200 keypoints)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(20000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 200);
    
    BENCHMARK("FPFH sequential")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      extractor.enable_parallel(false);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("FPFH parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(50);
      extractor.enable_parallel(true);
      
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("SHOT sequential")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(100);
      extractor.enable_parallel(false);
      
      std::vector<shot_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
    
    BENCHMARK("SHOT parallel")
    {
      kdtree_t<data_type> kdtree;
      kdtree.set_input(cloud);
      
      shot_extractor_t<data_type, kdtree_t<data_type>> extractor;
      extractor.set_input(cloud);
      extractor.set_knn(kdtree);
      extractor.set_search_radius(1.0);
      extractor.set_num_neighbors(100);
      extractor.enable_parallel(true);
      
      std::vector<shot_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      
      return descriptors.size();
    };
  }
}
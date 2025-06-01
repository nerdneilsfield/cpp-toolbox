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
    for (std::size_t i = 0; i < cloud_size; ++i)
    {
      indices.push_back(i);
    }
  }
  else
  {
    std::size_t step = cloud_size / num_keypoints;
    for (std::size_t i = 0; i < num_keypoints; ++i)
    {
      indices.push_back(i * step);
    }
  }
  
  return indices;
}

} // anonymous namespace

TEST_CASE("Descriptor extraction comprehensive benchmarks", "[pcl][descriptors][benchmark]")
{
  using data_type = float;
  
  // Small cloud benchmarks (1K-10K points)
  SECTION("Small cloud - sparse keypoints (1% density)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100); // 1% of points
    
    BENCHMARK("FPFH - 10K points, 100 keypoints")
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
    
    BENCHMARK("PFH - 10K points, 100 keypoints")
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
    
    BENCHMARK("SHOT - 10K points, 100 keypoints")
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
  
  SECTION("Small cloud - dense keypoints (20% density)")
  {
    auto cloud = generate_benchmark_cloud<data_type>(5000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 1000); // 20% of points
    
    BENCHMARK("FPFH - 5K points, 1K keypoints")
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
    
    BENCHMARK("PFH - 5K points, 1K keypoints")
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
  
  // Medium cloud benchmarks (20K-50K points)
  SECTION("Medium cloud - various keypoint densities")
  {
    auto cloud = generate_benchmark_cloud<data_type>(30000);
    
    // 0.1% density (30 keypoints)
    {
      auto keypoint_indices = generate_keypoint_indices(cloud.size(), 30);
      
      BENCHMARK("FPFH - 30K points, 30 keypoints (0.1%)")
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
      
      BENCHMARK("PFH - 30K points, 30 keypoints (0.1%)")
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
    
    // 1% density (300 keypoints)
    {
      auto keypoint_indices = generate_keypoint_indices(cloud.size(), 300);
      
      BENCHMARK("FPFH - 30K points, 300 keypoints (1%)")
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
      
      BENCHMARK("PFH - 30K points, 300 keypoints (1%)")
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
    
    // 10% density (3000 keypoints)
    {
      auto keypoint_indices = generate_keypoint_indices(cloud.size(), 3000);
      
      BENCHMARK("FPFH - 30K points, 3K keypoints (10%)")
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
      
      BENCHMARK("PFH - 30K points, 3K keypoints (10%)")
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
  }
  
  // Parallel vs Sequential comparison
  SECTION("Parallel speedup analysis")
  {
    auto cloud = generate_benchmark_cloud<data_type>(20000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 200);
    
    BENCHMARK("FPFH Sequential - 20K points, 200 keypoints")
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
    
    BENCHMARK("FPFH Parallel - 20K points, 200 keypoints")
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
    
    BENCHMARK("SHOT Sequential - 20K points, 200 keypoints")
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
    
    BENCHMARK("SHOT Parallel - 20K points, 200 keypoints")
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
    
    BENCHMARK("PFH Sequential - 20K points, 200 keypoints")
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
    
    BENCHMARK("PFH Parallel - 20K points, 200 keypoints")
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
  
  // Search radius impact
  SECTION("Search radius impact on performance")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100);
    
    std::vector<data_type> radii = {0.5f, 1.0f, 2.0f, 4.0f};
    
    for (auto radius : radii)
    {
      std::string radius_str = std::to_string(radius);
      
      BENCHMARK("FPFH - radius " + radius_str)
      {
        kdtree_t<data_type> kdtree;
        kdtree.set_input(cloud);
        
        fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
        extractor.set_input(cloud);
        extractor.set_knn(kdtree);
        extractor.set_search_radius(radius);
        extractor.set_num_neighbors(50);
        
        std::vector<fpfh_signature_t<data_type>> descriptors;
        extractor.compute(cloud, keypoint_indices, descriptors);
        return descriptors.size();
      };
    }
  }
  
  // Neighbor count impact
  SECTION("Neighbor count impact on performance")
  {
    auto cloud = generate_benchmark_cloud<data_type>(10000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 100);
    
    std::vector<std::size_t> neighbor_counts = {10, 30, 50, 100};
    
    for (auto num_neighbors : neighbor_counts)
    {
      std::string neighbor_str = std::to_string(num_neighbors);
      
      BENCHMARK("FPFH - " + neighbor_str + " neighbors")
      {
        kdtree_t<data_type> kdtree;
        kdtree.set_input(cloud);
        
        fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
        extractor.set_input(cloud);
        extractor.set_knn(kdtree);
        extractor.set_search_radius(1.0);
        extractor.set_num_neighbors(num_neighbors);
        
        std::vector<fpfh_signature_t<data_type>> descriptors;
        extractor.compute(cloud, keypoint_indices, descriptors);
        return descriptors.size();
      };
      
      // PFH with fewer neighbors due to O(k^2) complexity
      if (num_neighbors <= 30)
      {
        BENCHMARK("PFH - " + neighbor_str + " neighbors")
        {
          kdtree_t<data_type> kdtree;
          kdtree.set_input(cloud);
          
          pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
          extractor.set_input(cloud);
          extractor.set_knn(kdtree);
          extractor.set_search_radius(1.0);
          extractor.set_num_neighbors(num_neighbors);
          
          std::vector<pfh_signature_t<data_type>> descriptors;
          extractor.compute(cloud, keypoint_indices, descriptors);
          return descriptors.size();
        };
      }
    }
  }
  
  // Descriptor distance computation
  SECTION("Descriptor distance computation performance")
  {
    auto cloud = generate_benchmark_cloud<data_type>(5000);
    auto keypoint_indices = generate_keypoint_indices(cloud.size(), 50);
    
    // Pre-compute descriptors
    kdtree_t<data_type> kdtree;
    kdtree.set_input(cloud);
    
    // FPFH descriptors
    fpfh_extractor_t<data_type, kdtree_t<data_type>> fpfh_extractor;
    fpfh_extractor.set_input(cloud);
    fpfh_extractor.set_knn(kdtree);
    fpfh_extractor.set_search_radius(1.0);
    fpfh_extractor.set_num_neighbors(50);
    
    std::vector<fpfh_signature_t<data_type>> fpfh_desc;
    fpfh_extractor.compute(cloud, keypoint_indices, fpfh_desc);
    
    // SHOT descriptors
    shot_extractor_t<data_type, kdtree_t<data_type>> shot_extractor;
    shot_extractor.set_input(cloud);
    shot_extractor.set_knn(kdtree);
    shot_extractor.set_search_radius(1.0);
    shot_extractor.set_num_neighbors(100);
    
    std::vector<shot_signature_t<data_type>> shot_desc;
    shot_extractor.compute(cloud, keypoint_indices, shot_desc);
    
    // PFH descriptors
    pfh_extractor_t<data_type, kdtree_t<data_type>> pfh_extractor;
    pfh_extractor.set_input(cloud);
    pfh_extractor.set_knn(kdtree);
    pfh_extractor.set_search_radius(1.0);
    pfh_extractor.set_num_neighbors(30);
    
    std::vector<pfh_signature_t<data_type>> pfh_desc;
    pfh_extractor.compute(cloud, keypoint_indices, pfh_desc);
    
    BENCHMARK("FPFH distance computation (33 dims)")
    {
      data_type total_distance = 0;
      for (std::size_t i = 0; i < fpfh_desc.size(); ++i)
      {
        for (std::size_t j = i + 1; j < fpfh_desc.size(); ++j)
        {
          total_distance += fpfh_desc[i].distance(fpfh_desc[j]);
        }
      }
      return total_distance;
    };
    
    BENCHMARK("SHOT distance computation (352 dims)")
    {
      data_type total_distance = 0;
      for (std::size_t i = 0; i < shot_desc.size(); ++i)
      {
        for (std::size_t j = i + 1; j < shot_desc.size(); ++j)
        {
          total_distance += shot_desc[i].distance(shot_desc[j]);
        }
      }
      return total_distance;
    };
    
    BENCHMARK("PFH distance computation (125 dims)")
    {
      data_type total_distance = 0;
      for (std::size_t i = 0; i < pfh_desc.size(); ++i)
      {
        for (std::size_t j = i + 1; j < pfh_desc.size(); ++j)
        {
          total_distance += pfh_desc[i].distance(pfh_desc[j]);
        }
      }
      return total_distance;
    };
  }
}
#include <iostream>
#include <chrono>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/pfh_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>

using namespace toolbox::pcl;
using namespace toolbox::types;

template<typename DataType>
point_cloud_t<DataType> generate_test_cloud(std::size_t num_points)
{
  point_cloud_t<DataType> cloud;
  cloud.points.resize(num_points);
  toolbox::utils::random_t rng;
  
  for (std::size_t i = 0; i < num_points; ++i)
  {
    cloud.points[i] = point_t<DataType>(
      rng.random<DataType>(-10.0, 10.0),
      rng.random<DataType>(-10.0, 10.0),
      rng.random<DataType>(-10.0, 10.0)
    );
  }
  
  return cloud;
}

int main()
{
  using data_type = float;
  
  std::cout << "FPFH vs PFH Performance Comparison\n";
  std::cout << "==================================\n\n";
  
  // Test configurations
  std::vector<std::size_t> cloud_sizes = {10000, 50000, 100000};
  std::vector<std::size_t> keypoint_counts = {10, 50, 100, 500};
  
  for (auto cloud_size : cloud_sizes)
  {
    std::cout << "Cloud size: " << cloud_size << " points\n";
    std::cout << "----------------------------------------\n";
    
    // Generate cloud
    auto cloud = generate_test_cloud<data_type>(cloud_size);
    
    // Create KDTree
    kdtree_t<data_type> kdtree;
    kdtree.set_input(cloud);
    
    for (auto num_keypoints : keypoint_counts)
    {
      if (num_keypoints > cloud_size / 100) continue;  // Skip if too many keypoints
      
      std::cout << "  Keypoints: " << num_keypoints << "\n";
      
      // Generate keypoint indices
      std::vector<std::size_t> keypoint_indices;
      std::size_t step = cloud_size / num_keypoints;
      for (std::size_t i = 0; i < num_keypoints; ++i)
      {
        keypoint_indices.push_back(i * step);
      }
      
      // Test FPFH
      {
        fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
        extractor.set_input(cloud);
        extractor.set_knn(kdtree);
        extractor.set_search_radius(1.0);
        extractor.set_num_neighbors(50);
        
        std::vector<fpfh_signature_t<data_type>> descriptors;
        
        auto start = std::chrono::high_resolution_clock::now();
        extractor.compute(cloud, keypoint_indices, descriptors);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "    FPFH time: " << duration.count() << " ms";
        std::cout << " (computing SPFH for all " << cloud_size << " points)\n";
      }
      
      // Test PFH
      {
        pfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
        extractor.set_input(cloud);
        extractor.set_knn(kdtree);
        extractor.set_search_radius(1.0);
        extractor.set_num_neighbors(30);  // PFH is O(k^2), use fewer neighbors
        
        std::vector<pfh_signature_t<data_type>> descriptors;
        
        auto start = std::chrono::high_resolution_clock::now();
        extractor.compute(cloud, keypoint_indices, descriptors);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "    PFH time:  " << duration.count() << " ms";
        std::cout << " (computing only for " << num_keypoints << " keypoints)\n";
      }
      
      std::cout << "\n";
    }
    
    std::cout << "\n";
  }
  
  std::cout << "\nAnalysis:\n";
  std::cout << "---------\n";
  std::cout << "FPFH computes SPFH for ALL points in the cloud, then combines them for keypoints.\n";
  std::cout << "PFH computes descriptors ONLY for keypoints.\n";
  std::cout << "\nWhen the ratio (cloud_size / num_keypoints) is large, PFH can be faster!\n";
  std::cout << "FPFH advantage appears when you have many keypoints relative to cloud size.\n";
  
  return 0;
}
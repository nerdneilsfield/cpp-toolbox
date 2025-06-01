#include <iostream>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>

using namespace toolbox;
using namespace toolbox::pcl;

int main()
{
  using data_type = float;
  
  // Generate a simple test cloud
  types::point_cloud_t<data_type> cloud;
  cloud.points.resize(100);
  
  // Create a simple plane
  for (std::size_t i = 0; i < 10; ++i)
  {
    for (std::size_t j = 0; j < 10; ++j)
    {
      std::size_t idx = i * 10 + j;
      cloud.points[idx] = types::point_t<data_type>(
        static_cast<data_type>(i) * 0.1f,
        static_cast<data_type>(j) * 0.1f,
        0.0f
      );
    }
  }
  
  std::cout << "Generated cloud with " << cloud.size() << " points\n";
  
  // Create FPFH extractor
  fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
  kdtree_t<data_type> kdtree;
  
  std::cout << "Setting input cloud...\n";
  auto input_size = extractor.set_input(cloud);
  std::cout << "Input size: " << input_size << "\n";
  
  std::cout << "Setting KNN...\n";
  kdtree.set_input(cloud);
  auto knn_size = extractor.set_knn(kdtree);
  std::cout << "KNN size: " << knn_size << "\n";
  
  std::cout << "Setting search radius...\n";
  extractor.set_search_radius(0.15f);  // Larger radius for plane
  extractor.set_num_neighbors(20);
  
  // Test with a single keypoint in the middle
  std::vector<std::size_t> keypoint_indices = {55};  // Middle of the plane
  std::vector<fpfh_signature_t<data_type>> descriptors;
  
  std::cout << "Computing descriptors...\n";
  extractor.compute(cloud, keypoint_indices, descriptors);
  
  std::cout << "Number of descriptors: " << descriptors.size() << "\n";
  if (!descriptors.empty())
  {
    std::cout << "First descriptor histogram values:\n";
    bool all_zero = true;
    for (std::size_t i = 0; i < 33; ++i)
    {
      if (descriptors[0].histogram[i] != 0)
      {
        all_zero = false;
        std::cout << "  bin[" << i << "] = " << descriptors[0].histogram[i] << "\n";
      }
    }
    
    if (all_zero)
    {
      std::cout << "  All bins are zero!\n";
      
      // Check neighbors
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      kdtree.radius_neighbors(cloud.points[55], 0.15f, neighbor_indices, neighbor_distances);
      std::cout << "  Number of neighbors found: " << neighbor_indices.size() << "\n";
    }
    else
    {
      // Check descriptor norm
      data_type sum = 0;
      for (const auto& val : descriptors[0].histogram)
      {
        sum += val * val;
      }
      std::cout << "Descriptor norm: " << std::sqrt(sum) << "\n";
    }
  }
  
  return 0;
}
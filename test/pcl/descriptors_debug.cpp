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
  utils::random_t rng;
  
  for (std::size_t i = 0; i < 100; ++i)
  {
    cloud.points[i] = types::point_t<data_type>(
      rng.random<data_type>(-1.0, 1.0),
      rng.random<data_type>(-1.0, 1.0),
      rng.random<data_type>(-1.0, 1.0)
    );
  }
  
  std::cout << "Generated cloud with " << cloud.size() << " points\n";
  
  // Create FPFH extractor
  fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
  kdtree_t<data_type> kdtree;
  
  std::cout << "Setting input cloud...\n";
  auto input_size = extractor.set_input(cloud);
  std::cout << "Input size: " << input_size << "\n";
  
  std::cout << "Setting KNN...\n";
  auto knn_size = extractor.set_knn(kdtree);
  std::cout << "KNN size: " << knn_size << "\n";
  
  std::cout << "Setting search radius...\n";
  extractor.set_search_radius(0.5f);
  extractor.set_num_neighbors(20);
  
  // Test with a single keypoint
  std::vector<std::size_t> keypoint_indices = {50};
  std::vector<fpfh_signature_t<data_type>> descriptors;
  
  std::cout << "Computing descriptors...\n";
  extractor.compute(cloud, keypoint_indices, descriptors);
  
  std::cout << "Number of descriptors: " << descriptors.size() << "\n";
  if (!descriptors.empty())
  {
    std::cout << "First descriptor histogram values:\n";
    for (std::size_t i = 0; i < 10; ++i)
    {
      std::cout << "  bin[" << i << "] = " << descriptors[0].histogram[i] << "\n";
    }
    
    // Check if descriptor is all zeros
    data_type sum = 0;
    for (const auto& val : descriptors[0].histogram)
    {
      sum += val;
    }
    std::cout << "Sum of histogram values: " << sum << "\n";
  }
  
  return 0;
}
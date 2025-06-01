#include <iostream>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/utils/random.hpp>

using namespace toolbox::pcl;
using namespace toolbox::types;

int main()
{
  using data_type = float;
  
  std::cout << "Testing large scale descriptor extraction...\n";
  
  // Test with increasing sizes
  std::vector<std::size_t> cloud_sizes = {1000, 5000, 10000, 20000, 30000, 40000, 50000};
  
  for (auto size : cloud_sizes)
  {
    std::cout << "\nTesting with cloud size: " << size << std::endl;
    
    // Generate cloud
    point_cloud_t<data_type> cloud;
    cloud.points.resize(size);
    toolbox::utils::random_t rng;
    
    for (std::size_t i = 0; i < size; ++i)
    {
      cloud.points[i] = point_t<data_type>(
        rng.random<data_type>(-10.0, 10.0),
        rng.random<data_type>(-10.0, 10.0),
        rng.random<data_type>(-10.0, 10.0)
      );
    }
    
    // Create kdtree
    std::cout << "  Creating KDTree..." << std::flush;
    auto kdtree = std::make_unique<kdtree_t<data_type>>();
    auto kd_size = kdtree->set_input(cloud);
    std::cout << " done. Size: " << kd_size << std::endl;
    
    // Create extractor
    std::cout << "  Creating FPFH extractor..." << std::flush;
    fpfh_extractor_t<data_type, kdtree_t<data_type>> extractor;
    extractor.set_input(cloud);
    extractor.set_knn(*kdtree);
    extractor.set_search_radius(1.0);
    extractor.set_num_neighbors(50);
    extractor.enable_parallel(false);  // Start with sequential
    std::cout << " done." << std::endl;
    
    // Select a few keypoints
    std::vector<std::size_t> keypoint_indices;
    std::size_t num_keypoints = std::min<std::size_t>(10, size / 100);
    for (std::size_t i = 0; i < num_keypoints; ++i)
    {
      keypoint_indices.push_back(i * (size / num_keypoints));
    }
    
    std::cout << "  Computing descriptors for " << num_keypoints << " keypoints..." << std::flush;
    
    try
    {
      std::vector<fpfh_signature_t<data_type>> descriptors;
      extractor.compute(cloud, keypoint_indices, descriptors);
      std::cout << " done. Got " << descriptors.size() << " descriptors." << std::endl;
    }
    catch (const std::exception& e)
    {
      std::cout << " FAILED: " << e.what() << std::endl;
      break;
    }
    catch (...)
    {
      std::cout << " FAILED: Unknown exception" << std::endl;
      break;
    }
  }
  
  return 0;
}
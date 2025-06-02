#include <catch2/catch_all.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence_generator.hpp>  // For backward compatibility
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <random>
#include <chrono>

// FPFH度量类 / FPFH metric class
namespace toolbox::metrics {
template<typename T>
struct FPFHMetric
{
  using value_type = T;
  using result_type = T;
  
  T operator()(const toolbox::pcl::fpfh_signature_t<T>& a, const toolbox::pcl::fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
  
  T distance(const toolbox::pcl::fpfh_signature_t<T>& a, const toolbox::pcl::fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
};
} // namespace toolbox::metrics

using namespace toolbox::pcl;
using namespace toolbox::types;

// 创建测试点云和描述子 / Create test point cloud and descriptors
template<typename T>
std::pair<std::shared_ptr<std::vector<fpfh_signature_t<T>>>, 
          std::shared_ptr<std::vector<std::size_t>>> 
create_test_descriptors(size_t num_descriptors, std::mt19937& rng)
{
  auto descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
  auto indices = std::make_shared<std::vector<std::size_t>>();
  
  std::uniform_real_distribution<T> dist(0.0, 1.0);
  
  for (size_t i = 0; i < num_descriptors; ++i) {
    fpfh_signature_t<T> desc;
    
    // 创建随机描述子 / Create random descriptor
    for (int j = 0; j < 33; ++j) {
      desc.histogram[j] = dist(rng);
    }
    
    // 归一化 / Normalize
    T sum = 0;
    for (int j = 0; j < 33; ++j) {
      sum += desc.histogram[j];
    }
    if (sum > 0) {
      for (int j = 0; j < 33; ++j) {
        desc.histogram[j] /= sum;
      }
    }
    
    descriptors->push_back(desc);
    indices->push_back(i * 10);  // 假设关键点索引 / Assume keypoint indices
  }
  
  return {descriptors, indices};
}


TEST_CASE("对应点生成性能 / Correspondence Generation Performance", "[pcl][features][benchmark]")
{
  using T = float;
  
  SECTION("不同描述子数量的性能 / Performance with different descriptor counts")
  {
    const std::vector<size_t> descriptor_counts = {100, 500, 1000};
    
    for (size_t num_desc : descriptor_counts) {
      DYNAMIC_SECTION("描述子数量 / Descriptor count: " << num_desc)
      {
        // 准备测试数据 / Prepare test data
        std::mt19937 rng(42);
        auto src_data = create_test_descriptors<T>(num_desc, rng);
        auto dst_data = create_test_descriptors<T>(num_desc, rng);
        auto src_descriptors = src_data.first;
        auto src_indices = src_data.second;
        auto dst_descriptors = dst_data.first;
        auto dst_indices = dst_data.second;
        
        // 创建假的点云 / Create fake point clouds
        auto src_cloud = std::make_shared<point_cloud_t<T>>();
        auto dst_cloud = std::make_shared<point_cloud_t<T>>();
        src_cloud->points.resize(num_desc * 10);
        dst_cloud->points.resize(num_desc * 10);
        
        // KNN方法 / KNN method
        BENCHMARK("KNN方法 / KNN method")
        {
          using CorrespondenceGen = knn_correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                                  bfknn_generic_t<fpfh_signature_t<T>, 
                                                                                 toolbox::metrics::FPFHMetric<T>>>;
          CorrespondenceGen corr_gen;
          
          auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<T>, toolbox::metrics::FPFHMetric<T>>>();
          corr_gen.set_knn(knn);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);
          
          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);
          
          return correspondences.size();
        };
        
        // 暴力搜索方法（串行） / Brute-force method (serial)
        BENCHMARK("暴力搜索（串行） / Brute-force (serial)")
        {
          brute_force_correspondence_generator_t<T, fpfh_signature_t<T>> corr_gen;
          
          corr_gen.enable_parallel(false);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);
          
          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);
          
          return correspondences.size();
        };
        
        // 暴力搜索方法（并行） / Brute-force method (parallel)
        BENCHMARK("暴力搜索（并行） / Brute-force (parallel)")
        {
          brute_force_correspondence_generator_t<T, fpfh_signature_t<T>> corr_gen;
          
          corr_gen.enable_parallel(true);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(0.8f);
          corr_gen.set_mutual_verification(true);
          
          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);
          
          return correspondences.size();
        };
      }
    }
  }
  
  SECTION("参数影响 / Parameter effects")
  {
    // 准备固定大小的测试数据 / Prepare fixed-size test data
    const size_t num_desc = 500;
    std::mt19937 rng(42);
    auto src_data = create_test_descriptors<T>(num_desc, rng);
    auto dst_data = create_test_descriptors<T>(num_desc, rng);
    auto src_descriptors = src_data.first;
    auto src_indices = src_data.second;
    auto dst_descriptors = dst_data.first;
    auto dst_indices = dst_data.second;
    
    auto src_cloud = std::make_shared<point_cloud_t<T>>();
    auto dst_cloud = std::make_shared<point_cloud_t<T>>();
    src_cloud->points.resize(num_desc * 10);
    dst_cloud->points.resize(num_desc * 10);
    
    // 测试不同比率阈值 / Test different ratio thresholds
    for (float ratio : {0.6f, 0.7f, 0.8f, 0.9f}) {
      DYNAMIC_SECTION("比率阈值 / Ratio threshold: " << ratio)
      {
        BENCHMARK("对应点生成 / Correspondence generation")
        {
          using CorrespondenceGen = correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                              bfknn_generic_t<fpfh_signature_t<T>, 
                                                                             toolbox::metrics::FPFHMetric<T>>>;
          CorrespondenceGen corr_gen;
          
          auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<T>, toolbox::metrics::FPFHMetric<T>>>();
          corr_gen.set_knn(knn);
          corr_gen.set_source(src_cloud, src_descriptors, src_indices);
          corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
          corr_gen.set_ratio(ratio);
          corr_gen.set_mutual_verification(false);
          
          std::vector<correspondence_t> correspondences;
          corr_gen.compute(correspondences);
          
          return correspondences.size();
        };
      }
    }
    
    // 测试双向验证的影响 / Test mutual verification effect
    BENCHMARK("无双向验证 / No mutual verification")
    {
      using CorrespondenceGen = correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                          bfknn_generic_t<fpfh_signature_t<T>, 
                                                                         toolbox::metrics::FPFHMetric<T>>>;
      CorrespondenceGen corr_gen;
      
      auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<T>, toolbox::metrics::FPFHMetric<T>>>();
      corr_gen.set_knn(knn);
      corr_gen.set_source(src_cloud, src_descriptors, src_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(false);
      
      std::vector<correspondence_t> correspondences;
      corr_gen.compute(correspondences);
      
      return correspondences.size();
    };
    
    BENCHMARK("有双向验证 / With mutual verification")
    {
      using CorrespondenceGen = correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                          bfknn_generic_t<fpfh_signature_t<T>, 
                                                                         toolbox::metrics::FPFHMetric<T>>>;
      CorrespondenceGen corr_gen;
      
      auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<T>, toolbox::metrics::FPFHMetric<T>>>();
      corr_gen.set_knn(knn);
      corr_gen.set_source(src_cloud, src_descriptors, src_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_indices);
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(true);
      
      std::vector<correspondence_t> correspondences;
      corr_gen.compute(correspondences);
      
      return correspondences.size();
    };
  }
}
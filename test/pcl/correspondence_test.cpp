#include <catch2/catch_all.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>  // 只为了 fpfh_signature_t / Only for fpfh_signature_t
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace toolbox::pcl;
using namespace toolbox::types;

// 创建测试点云 / Create test point cloud
template<typename T>
point_cloud_t<T> create_test_cloud(size_t num_points, T scale = 1.0)
{
  point_cloud_t<T> cloud;
  cloud.points.reserve(num_points);
  
  std::srand(std::time(nullptr));
  for (size_t i = 0; i < num_points; ++i) {
    point_t<T> p;
    p.x = (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 2 - 1) * scale;
    p.y = (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 2 - 1) * scale;
    p.z = (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 2 - 1) * scale;
    cloud.points.push_back(p);
  }
  
  return cloud;
}

// FPFH度量类 / FPFH metric class
template<typename T>
struct FPFHMetric
{
  using value_type = T;
  using result_type = T;
  
  T operator()(const fpfh_signature_t<T>& a, const fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
  
  T distance(const fpfh_signature_t<T>& a, const fpfh_signature_t<T>& b) const
  {
    return a.distance(b);
  }
};

TEST_CASE("对应点生成器 / Correspondence Generator", "[pcl][correspondence]")
{
  using T = float;
  
  SECTION("基本功能测试 / Basic functionality test")
  {
    // 创建源和目标点云 / Create source and target clouds
    auto src_cloud = std::make_shared<point_cloud_t<T>>(create_test_cloud<T>(1000, 10.0f));
    auto dst_cloud = std::make_shared<point_cloud_t<T>>(*src_cloud);
    
    // 添加一些噪声到目标点云 / Add some noise to target cloud
    for (auto& p : dst_cloud->points) {
      p.x += (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 0.2f - 0.1f);
      p.y += (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 0.2f - 0.1f);
      p.z += (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 0.2f - 0.1f);
    }
    
    // 使用简单的均匀采样作为关键点 / Use simple uniform sampling as keypoints
    auto src_keypoint_indices = std::make_shared<std::vector<size_t>>();
    auto dst_keypoint_indices = std::make_shared<std::vector<size_t>>();
    
    // 每隔10个点选一个作为关键点 / Select every 10th point as keypoint
    for (size_t i = 0; i < src_cloud->points.size(); i += 10) {
      src_keypoint_indices->push_back(i);
    }
    for (size_t i = 0; i < dst_cloud->points.size(); i += 10) {
      dst_keypoint_indices->push_back(i);
    }
    
    // 为了测试，创建假的描述子 / Create fake descriptors for testing
    auto src_descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
    auto dst_descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
    
    // 为每个关键点创建一个简单的描述子 / Create a simple descriptor for each keypoint
    for (size_t i = 0; i < src_keypoint_indices->size(); ++i) {
      fpfh_signature_t<T> desc;
      size_t idx = (*src_keypoint_indices)[i];
      const auto& p = src_cloud->points[idx];
      
      // 基于点的位置创建简单的特征 / Create simple features based on point position
      for (int j = 0; j < 33; ++j) {
        desc.histogram[j] = static_cast<T>(i * 33 + j) * 0.01f + 
                           (p.x + p.y + p.z) * 0.001f;
      }
      src_descriptors->push_back(desc);
    }
    
    // 为目标点创建类似但略有不同的描述子 / Create similar but slightly different descriptors for target
    for (size_t i = 0; i < dst_keypoint_indices->size(); ++i) {
      fpfh_signature_t<T> desc;
      size_t idx = (*dst_keypoint_indices)[i];
      const auto& p = dst_cloud->points[idx];
      
      // 基于点的位置创建简单的特征，添加一些噪声 / Create simple features with some noise
      for (int j = 0; j < 33; ++j) {
        desc.histogram[j] = static_cast<T>(i * 33 + j) * 0.01f + 
                           (p.x + p.y + p.z) * 0.001f + 
                           static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX) * 0.001f;
      }
      dst_descriptors->push_back(desc);
    }
    
    std::cout << "关键点数量 / Keypoint count: " << src_keypoint_indices->size() 
              << ", " << dst_keypoint_indices->size() << std::endl;
    std::cout << "描述子数量 / Descriptor count: " << src_descriptors->size() 
              << ", " << dst_descriptors->size() << std::endl;
    
    // 测试KNN方法 / Test KNN method
    {
      using CorrespondenceGen = knn_correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                              bfknn_generic_t<fpfh_signature_t<T>, 
                                                                             FPFHMetric<T>>>;
      CorrespondenceGen corr_gen;
      
      // 设置KNN / Set KNN
      auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<T>, FPFHMetric<T>>>();
      corr_gen.set_knn(knn);
      
      // 设置数据 / Set data
      corr_gen.set_source(src_cloud, src_descriptors, src_keypoint_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_keypoint_indices);
      
      // 设置参数 / Set parameters
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(true);
      corr_gen.set_distance_threshold(0.5f);
      
      // 计算对应关系 / Compute correspondences
      std::vector<correspondence_t> correspondences_knn;
      corr_gen.compute(correspondences_knn);
      
      // 验证结果 / Verify results
      REQUIRE(correspondences_knn.size() > 0);
      REQUIRE(correspondences_knn.size() <= src_keypoint_indices->size());
      
      // 打印统计信息 / Print statistics
      std::cout << "KNN方法统计 / KNN method statistics:\n";
      std::cout << corr_gen.get_statistics() << std::endl;
      
      // 检查每个对应关系 / Check each correspondence
      for (const auto& corr : correspondences_knn) {
        REQUIRE(corr.distance >= 0.0f);
        REQUIRE(corr.distance <= 0.5f);  // 应该满足距离阈值 / Should satisfy distance threshold
      }
    }
    
    // 测试暴力搜索方法 / Test brute-force method
    {
      brute_force_correspondence_generator_t<T, fpfh_signature_t<T>> corr_gen;
      
      // 启用并行 / Enable parallel
      corr_gen.enable_parallel(true);
      
      // 设置数据 / Set data
      corr_gen.set_source(src_cloud, src_descriptors, src_keypoint_indices);
      corr_gen.set_destination(dst_cloud, dst_descriptors, dst_keypoint_indices);
      
      // 设置参数 / Set parameters
      corr_gen.set_ratio(0.8f);
      corr_gen.set_mutual_verification(true);
      corr_gen.set_distance_threshold(0.5f);
      
      // 计算对应关系 / Compute correspondences
      std::vector<correspondence_t> correspondences_bf;
      corr_gen.compute(correspondences_bf);
      
      // 验证结果 / Verify results
      REQUIRE(correspondences_bf.size() > 0);
      REQUIRE(correspondences_bf.size() <= src_keypoint_indices->size());
      
      // 打印统计信息 / Print statistics
      std::cout << "暴力搜索方法统计 / Brute-force method statistics:\n";
      std::cout << corr_gen.get_statistics() << std::endl;
      
      // 检查每个对应关系 / Check each correspondence
      for (const auto& corr : correspondences_bf) {
        REQUIRE(corr.distance >= 0.0f);
        REQUIRE(corr.distance <= 0.5f);  // 应该满足距离阈值 / Should satisfy distance threshold
      }
    }
  }
  
  SECTION("参数影响测试 / Parameter effect test")
  {
    // 创建两个不同的点云 / Create two different point clouds
    auto src_cloud = std::make_shared<point_cloud_t<T>>(create_test_cloud<T>(500, 5.0f));
    auto dst_cloud = std::make_shared<point_cloud_t<T>>(create_test_cloud<T>(500, 5.0f));
    
    // 使用简单的关键点（所有点） / Use simple keypoints (all points)
    auto src_keypoint_indices = std::make_shared<std::vector<size_t>>();
    auto dst_keypoint_indices = std::make_shared<std::vector<size_t>>();
    for (size_t i = 0; i < 50; ++i) {  // 只用前50个点 / Use only first 50 points
      src_keypoint_indices->push_back(i);
      dst_keypoint_indices->push_back(i);
    }
    
    // 创建简单的描述子 / Create simple descriptors
    auto src_descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
    auto dst_descriptors = std::make_shared<std::vector<fpfh_signature_t<T>>>();
    
    // 为测试创建一些相似的描述子 / Create some similar descriptors for testing
    for (size_t i = 0; i < 50; ++i) {
      fpfh_signature_t<T> src_desc, dst_desc;
      
      // 初始化描述子 / Initialize descriptors
      for (int j = 0; j < 33; ++j) {
        src_desc.histogram[j] = static_cast<T>(i + j) / 100.0f;
        dst_desc.histogram[j] = src_desc.histogram[j] + 0.01f * i;  // 添加小的差异 / Add small difference
      }
      
      src_descriptors->push_back(src_desc);
      dst_descriptors->push_back(dst_desc);
    }
    
    using CorrespondenceGen = correspondence_generator_t<T, fpfh_signature_t<T>, 
                                                        bfknn_generic_t<fpfh_signature_t<T>, 
                                                                       FPFHMetric<T>>>;
    
    // 使用便捷函数测试 / Test using convenience functions
    {
      // 测试KNN便捷函数 / Test KNN convenience function
      auto corr_knn = generate_correspondences_knn<T, fpfh_signature_t<T>, 
                                                   bfknn_generic_t<fpfh_signature_t<T>, FPFHMetric<T>>>(
          src_cloud, src_descriptors, src_keypoint_indices,
          dst_cloud, dst_descriptors, dst_keypoint_indices,
          0.8f, true);
      
      REQUIRE(corr_knn.size() > 0);
      
      // 测试暴力搜索便捷函数 / Test brute-force convenience function
      auto corr_bf = generate_correspondences_brute_force<T, fpfh_signature_t<T>>(
          src_cloud, src_descriptors, src_keypoint_indices,
          dst_cloud, dst_descriptors, dst_keypoint_indices,
          0.8f, true, false);  // 不使用并行 / Don't use parallel
      
      REQUIRE(corr_bf.size() > 0);
      
      // 两种方法应该产生相似的结果 / Both methods should produce similar results
      REQUIRE(std::abs(static_cast<int>(corr_knn.size()) - static_cast<int>(corr_bf.size())) <= 5);
    }
  }
}
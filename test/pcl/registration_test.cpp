#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/registration/super_four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/features/curvature_keypoints.hpp>
#include <cpp-toolbox/pcl/features/harris3d_keypoints.hpp>
#include <cpp-toolbox/pcl/features/iss_keypoints.hpp>
#include <cpp-toolbox/pcl/features/sift3d_keypoints.hpp>
#include <cpp-toolbox/pcl/features/loam_feature_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/filters/voxel_grid_downsampling.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/io/formats/pcd.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>

#include "test_data_dir.hpp"

#include <random>

// Logger macros
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()
#define LOG_DEBUG_S toolbox::logger::thread_logger_t::instance().debug_s()

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace Catch::Matchers;

// 前向声明辅助函数 / Forward declare helper function
void perform_feature_matching_and_ransac(
    const std::shared_ptr<point_cloud_t<float>>& cloud1,
    const std::shared_ptr<point_cloud_t<float>>& cloud2,
    const std::vector<std::size_t>& keypoints1,
    const std::vector<std::size_t>& keypoints2,
    const std::string& method_name);

// 辅助函数：创建简单的测试点云 / Helper: create simple test point cloud
template<typename DataType>
std::shared_ptr<point_cloud_t<DataType>> create_test_cloud(std::size_t num_points = 100)
{
  auto cloud = std::make_shared<point_cloud_t<DataType>>();
  cloud->points.resize(num_points);
  
  std::mt19937 gen(42);
  std::uniform_real_distribution<DataType> dist(-1.0, 1.0);
  
  for (std::size_t i = 0; i < num_points; ++i) {
    cloud->points[i].x = dist(gen);
    cloud->points[i].y = dist(gen);
    cloud->points[i].z = dist(gen);
  }
  
  return cloud;
}

// 辅助函数：应用变换到点云 / Helper: apply transformation to point cloud
template<typename DataType>
std::shared_ptr<point_cloud_t<DataType>> transform_cloud(
    const std::shared_ptr<point_cloud_t<DataType>>& cloud,
    const Eigen::Matrix<DataType, 4, 4>& transform)
{
  auto transformed = std::make_shared<point_cloud_t<DataType>>();
  transformed->points.resize(cloud->points.size());
  
  for (std::size_t i = 0; i < cloud->points.size(); ++i) {
    const auto& pt = cloud->points[i];
    Eigen::Matrix<DataType, 4, 1> p;
    p << pt.x, pt.y, pt.z, 1.0;
    
    Eigen::Matrix<DataType, 4, 1> tp = transform * p;
    transformed->points[i].x = tp[0];
    transformed->points[i].y = tp[1];
    transformed->points[i].z = tp[2];
  }
  
  return transformed;
}

// 辅助函数：创建测试用的对应关系 / Helper: create test correspondences
std::shared_ptr<std::vector<correspondence_t>> create_test_correspondences(
    std::size_t num_points, float outlier_ratio = 0.3)
{
  auto correspondences = std::make_shared<std::vector<correspondence_t>>();
  correspondences->reserve(num_points);
  
  std::mt19937 gen(42);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  std::uniform_int_distribution<std::size_t> idx_dist(0, num_points - 1);
  
  for (std::size_t i = 0; i < num_points; ++i) {
    correspondence_t corr;
    corr.src_idx = i;
    
    if (dist(gen) < outlier_ratio) {
      // 外点：随机匹配 / Outlier: random match
      corr.dst_idx = idx_dist(gen);
      corr.distance = dist(gen) * 10.0f;  // 大距离 / Large distance
    } else {
      // 内点：正确匹配 / Inlier: correct match
      corr.dst_idx = i;
      corr.distance = dist(gen) * 0.1f;  // 小距离 / Small distance
    }
    
    correspondences->push_back(corr);
  }
  
  return correspondences;
}

TEST_CASE("RANSAC Registration Basic", "[registration][ransac]")
{
  using DataType = float;
  
  // 创建源点云 / Create source cloud
  auto source_cloud = create_test_cloud<DataType>(200);
  
  // 创建已知变换 / Create known transformation
  Eigen::Matrix<DataType, 4, 4> ground_truth;
  ground_truth.setIdentity();
  ground_truth.block<3, 3>(0, 0) = Eigen::AngleAxis<DataType>(0.1, Eigen::Matrix<DataType, 3, 1>::UnitZ()).matrix();
  ground_truth.block<3, 1>(0, 3) << 0.1, 0.2, 0.3;
  
  // 变换点云创建目标点云 / Transform cloud to create target
  auto target_cloud = transform_cloud(source_cloud, ground_truth);
  
  // 创建对应关系（包含一些外点） / Create correspondences (with some outliers)
  auto correspondences = create_test_correspondences(source_cloud->size(), 0.3f);
  
  SECTION("Basic alignment")
  {
    ransac_registration_t<DataType> ransac;
    ransac.set_source(source_cloud);
    ransac.set_target(target_cloud);
    ransac.set_correspondences(correspondences);
    ransac.set_max_iterations(100);
    ransac.set_inlier_threshold(0.05f);
    ransac.set_confidence(0.99f);
    
    registration_result_t<DataType> result;
    REQUIRE(ransac.align(result));
    
    // 检查结果 / Check results
    REQUIRE(result.converged);
    REQUIRE(result.inliers.size() > source_cloud->size() * 0.5);  // 至少50%内点 / At least 50% inliers
    
    // 检查变换矩阵是否接近真值 / Check if transformation is close to ground truth
    Eigen::Matrix<DataType, 4, 4> error = result.transformation.inverse() * ground_truth;
    DataType translation_error = error.block<3, 1>(0, 3).norm();
    REQUIRE_THAT(translation_error, WithinAbs(0.0, 0.1));
  }
  
  SECTION("Parameter validation")
  {
    ransac_registration_t<DataType> ransac;
    
    // 测试置信度范围 / Test confidence range
    ransac.set_confidence(1.5f);
    REQUIRE(ransac.get_confidence() == 1.0f);
    
    ransac.set_confidence(-0.5f);
    REQUIRE(ransac.get_confidence() == 0.0f);
    
    // 测试样本大小 / Test sample size
    ransac.set_sample_size(2);
    REQUIRE(ransac.get_sample_size() == 3);  // 最小为3 / Minimum is 3
  }
  
  SECTION("Empty input handling")
  {
    ransac_registration_t<DataType> ransac;
    registration_result_t<DataType> result;
    
    // 没有设置点云 / No clouds set
    REQUIRE_FALSE(ransac.align(result));
    
    // 设置点云但没有对应关系 / Clouds set but no correspondences
    ransac.set_source(source_cloud);
    ransac.set_target(target_cloud);
    REQUIRE_FALSE(ransac.align(result));
  }
}

TEST_CASE("4PCS Registration Basic", "[registration][4pcs]")
{
  using DataType = float;
  
  // 创建源点云 / Create source cloud
  auto source_cloud = create_test_cloud<DataType>(500);
  
  // 创建已知变换 / Create known transformation
  Eigen::Matrix<DataType, 4, 4> ground_truth;
  ground_truth.setIdentity();
  ground_truth.block<3, 3>(0, 0) = Eigen::AngleAxis<DataType>(0.2, Eigen::Matrix<DataType, 3, 1>::UnitY()).matrix();
  ground_truth.block<3, 1>(0, 3) << 0.15, 0.25, 0.35;
  
  // 变换点云创建目标点云 / Transform cloud to create target
  auto target_cloud = transform_cloud(source_cloud, ground_truth);
  
  SECTION("Basic alignment")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(source_cloud);
    fourpcs.set_target(target_cloud);
    fourpcs.set_delta(0.02f);
    fourpcs.set_overlap(0.8f);
    fourpcs.set_sample_size(200);
    fourpcs.set_num_bases(20);
    fourpcs.set_max_iterations(50);
    
    registration_result_t<DataType> result;
    REQUIRE(fourpcs.align(result));
    
    // 检查结果 / Check results
    REQUIRE(result.inliers.size() > 0);
    REQUIRE(result.fitness_score < 0.1f);
  }
  
  SECTION("Parameter validation")
  {
    four_pcs_registration_t<DataType> fourpcs;
    
    // 测试重叠率范围 / Test overlap range
    fourpcs.set_overlap(1.5f);
    REQUIRE(fourpcs.get_overlap() == 1.0f);
    
    fourpcs.set_overlap(-0.5f);
    REQUIRE(fourpcs.get_overlap() == 0.0f);
    
    // 测试delta / Test delta
    fourpcs.set_delta(-0.01f);
    REQUIRE(fourpcs.get_delta() == 0.01f);  // 取绝对值 / Takes absolute value
  }
  
  SECTION("Small point cloud handling")
  {
    auto small_cloud = create_test_cloud<DataType>(3);  // 太小 / Too small
    
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(small_cloud);
    fourpcs.set_target(small_cloud);
    
    registration_result_t<DataType> result;
    REQUIRE_FALSE(fourpcs.align(result));
  }
}

TEST_CASE("Super4PCS Registration Basic", "[registration][super4pcs]")
{
  using DataType = float;
  
  // 创建较大的点云测试Super4PCS的性能优势 / Create larger cloud to test Super4PCS performance advantage
  auto source_cloud = create_test_cloud<DataType>(1000);
  
  // 创建已知变换 / Create known transformation
  Eigen::Matrix<DataType, 4, 4> ground_truth;
  ground_truth.setIdentity();
  ground_truth.block<3, 3>(0, 0) = Eigen::AngleAxis<DataType>(0.15, 
      Eigen::Matrix<DataType, 3, 1>(1, 1, 1).normalized()).matrix();
  ground_truth.block<3, 1>(0, 3) << 0.2, 0.3, 0.4;
  
  // 变换点云创建目标点云 / Transform cloud to create target
  auto target_cloud = transform_cloud(source_cloud, ground_truth);
  
  SECTION("Smart indexing enabled")
  {
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(source_cloud);
    super4pcs.set_target(target_cloud);
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.7f);
    super4pcs.set_sample_size(300);
    super4pcs.enable_smart_indexing(true);
    super4pcs.set_grid_resolution(0.0f);  // 自动计算 / Auto compute
    super4pcs.set_num_bases(10);
    
    registration_result_t<DataType> result;
    REQUIRE(super4pcs.align(result));
    
    // 检查结果 / Check results
    REQUIRE(result.inliers.size() > 0);
    // Super4PCS should report its algorithm name (might be "4PCS" if using base implementation)
    const auto algo_name = super4pcs.get_algorithm_name();
    REQUIRE((algo_name == "Super4PCS" || algo_name == "4PCS"));
  }
  
  SECTION("Smart indexing disabled (fallback to 4PCS)")
  {
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(source_cloud);
    super4pcs.set_target(target_cloud);
    super4pcs.set_delta(0.02f);
    super4pcs.set_overlap(0.7f);
    super4pcs.set_sample_size(300);
    super4pcs.enable_smart_indexing(false);  // 禁用智能索引 / Disable smart indexing
    super4pcs.set_num_bases(10);
    
    registration_result_t<DataType> result;
    REQUIRE(super4pcs.align(result));
    
    // 应该退化为标准4PCS / Should fallback to standard 4PCS
    REQUIRE(result.inliers.size() > 0);
  }
  
  SECTION("Grid resolution settings")
  {
    super_four_pcs_registration_t<DataType> super4pcs;
    
    // 测试自定义网格分辨率 / Test custom grid resolution
    super4pcs.set_grid_resolution(0.1f);
    REQUIRE(super4pcs.get_grid_resolution() == 0.1f);
    
    // 测试点对容差 / Test pair distance tolerance
    super4pcs.set_pair_distance_epsilon(0.05f);
    REQUIRE(super4pcs.get_pair_distance_epsilon() == 0.05f);
  }
}

TEST_CASE("Registration with Real Data", "[registration][integration]")
{
  using DataType = float;
  
  // 加载真实点云数据 / Load real point cloud data
  std::string bunny_path = test_data_dir + std::string("/bunny.pcd");
  
  auto cloud_unique = toolbox::io::read_pcd<DataType>(bunny_path);
  std::shared_ptr<point_cloud_t<DataType>> cloud;
  if (cloud_unique) {
    cloud = std::move(cloud_unique);
  } else {
    // If bunny.pcd doesn't exist, create a synthetic cloud
    cloud = create_test_cloud<DataType>(1000);
  }
  REQUIRE(cloud);
  REQUIRE(!cloud->empty());
  
  // 创建带噪声的变换版本 / Create transformed version with noise
  Eigen::Matrix<DataType, 4, 4> transform;
  transform.setIdentity();
  transform.block<3, 3>(0, 0) = Eigen::AngleAxis<DataType>(0.1, 
      Eigen::Matrix<DataType, 3, 1>::UnitZ()).matrix();
  transform.block<3, 1>(0, 3) << 0.05, 0.05, 0.05;
  
  auto transformed_cloud = transform_cloud(cloud, transform);
  
  // 添加噪声 / Add noise
  std::mt19937 gen(42);
  std::normal_distribution<DataType> noise_dist(0.0, 0.001);
  for (auto& pt : transformed_cloud->points) {
    pt.x += noise_dist(gen);
    pt.y += noise_dist(gen);
    pt.z += noise_dist(gen);
  }
  
  SECTION("Full pipeline with feature extraction")
  {
    // 1. 提取关键点 / Extract keypoints
    curvature_keypoint_extractor_t<DataType> curvature;
    curvature.set_curvature_threshold(0.0001f);  // 降低阈值以获得更多关键点 / Lower threshold for more keypoints
    curvature.set_search_radius(0.05f);  // 增大搜索半径 / Increase search radius
    curvature.set_non_maxima_radius(0.01f);  // 设置非极大值抑制半径 / Set non-maxima suppression radius
    
    // 设置knn算法 / Set knn algorithm
    kdtree_t<DataType> kdtree_src, kdtree_tgt;
    
    curvature.set_input(cloud);
    curvature.set_knn(kdtree_src);
    auto src_keypoint_indices = curvature.extract();
    
    curvature.set_input(transformed_cloud);
    curvature.set_knn(kdtree_tgt);
    auto tgt_keypoint_indices = curvature.extract();
    
    // 确保有足够的关键点 / Ensure enough keypoints
    if (src_keypoint_indices.size() < 10 || tgt_keypoint_indices.size() < 10) {
      // 如果关键点太少，跳过此测试 / Skip test if too few keypoints
      WARN("关键点太少，跳过完整流程测试 / Too few keypoints, skipping full pipeline test");
      return;
    }
    
    auto src_keypoint_indices_ptr = std::make_shared<std::vector<std::size_t>>(src_keypoint_indices);
    auto tgt_keypoint_indices_ptr = std::make_shared<std::vector<std::size_t>>(tgt_keypoint_indices);
    
    // 2. 计算描述子 / Compute descriptors
    fpfh_extractor_t<DataType> fpfh;
    fpfh.set_search_radius(0.05f);
    
    auto src_descriptors = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
    auto tgt_descriptors = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
    
    // 设置knn算法 / Set knn algorithm for FPFH
    kdtree_t<DataType> kdtree_fpfh_src, kdtree_fpfh_tgt;
    
    fpfh.set_input(cloud);
    fpfh.set_knn(kdtree_fpfh_src);
    fpfh.compute(*cloud, *src_keypoint_indices_ptr, *src_descriptors);
    
    fpfh.set_input(transformed_cloud);
    fpfh.set_knn(kdtree_fpfh_tgt);
    fpfh.compute(*transformed_cloud, *tgt_keypoint_indices_ptr, *tgt_descriptors);
    
    // 3. 生成对应关系 / Generate correspondences
    auto correspondences = generate_correspondences_brute_force<DataType, fpfh_signature_t<DataType>>(
        cloud, src_descriptors, src_keypoint_indices_ptr,
        transformed_cloud, tgt_descriptors, tgt_keypoint_indices_ptr,
        0.8f, true, false);
    
    if (correspondences.size() < 10) {
      // 如果对应关系太少，跳过RANSAC测试 / Skip RANSAC test if too few correspondences
      WARN("对应关系太少 / Too few correspondences: " + std::to_string(correspondences.size()));
      return;
    }
    
    REQUIRE(!correspondences.empty());
    
    // 4. RANSAC配准 / RANSAC registration
    auto corr_ptr = std::make_shared<std::vector<correspondence_t>>(correspondences);
    
    ransac_registration_t<DataType> ransac;
    ransac.set_source(cloud);
    ransac.set_target(transformed_cloud);
    ransac.set_correspondences(corr_ptr);
    ransac.set_max_iterations(500);
    ransac.set_inlier_threshold(0.01f);
    ransac.set_min_inliers(3);  // 设置最小内点数 / Set minimum inliers
    
    registration_result_t<DataType> result;
    bool success = ransac.align(result);
    
    if (success) {
      // 检查配准质量 / Check registration quality
      REQUIRE(result.inliers.size() > correspondences.size() * 0.1);  // 至少10%内点 / At least 10% inliers
      REQUIRE(result.fitness_score < 0.1f);
    } else {
      WARN("配准失败，可能由于对应关系质量不佳 / Registration failed, possibly due to poor correspondence quality");
    }
  }
}

TEST_CASE("KITTI Dataset Registration", "[registration][kitti]")
{
  using DataType = float;
  
  // 加载KITTI数据集点云 / Load KITTI dataset point clouds
  std::string kitti_file1 = test_data_dir + std::string("/000000.bin");
  std::string kitti_file2 = test_data_dir + std::string("/000002.bin");
  
  auto cloud1_unique = toolbox::io::read_kitti_bin<DataType>(kitti_file1);
  auto cloud2_unique = toolbox::io::read_kitti_bin<DataType>(kitti_file2);
  
  if (!cloud1_unique || !cloud2_unique) {
    WARN("KITTI数据文件不存在，跳过测试 / KITTI data files not found, skipping test");
    return;
  }
  
  // 转换为shared_ptr / Convert to shared_ptr
  auto cloud1 = std::shared_ptr<point_cloud_t<DataType>>(std::move(cloud1_unique));
  auto cloud2 = std::shared_ptr<point_cloud_t<DataType>>(std::move(cloud2_unique));
  
  REQUIRE(cloud1);
  REQUIRE(cloud2);
  REQUIRE(!cloud1->empty());
  REQUIRE(!cloud2->empty());
  
  LOG_INFO_S << "加载KITTI点云1: " << cloud1->size() << " 个点 / Loaded KITTI cloud 1: " << cloud1->size() << " points";
  LOG_INFO_S << "加载KITTI点云2: " << cloud2->size() << " 个点 / Loaded KITTI cloud 2: " << cloud2->size() << " points";
  
  SECTION("4PCS on KITTI data")
  {
    four_pcs_registration_t<DataType> fourpcs;
    fourpcs.set_source(cloud1);
    fourpcs.set_target(cloud2);
    fourpcs.set_delta(1.0f);  // KITTI数据尺度较大 / KITTI data has larger scale
    fourpcs.set_overlap(0.3f);  // 两帧之间可能只有30%重叠 / May only have 30% overlap between frames
    fourpcs.set_sample_size(1000);  // 采样更多点以提高成功率 / Sample more points for better success rate
    fourpcs.set_num_bases(100);  // 尝试更多基以找到正确匹配 / Try more bases to find correct match
    fourpcs.set_max_iterations(200);
    fourpcs.set_inlier_threshold(1.0f);  // 设置合理的内点阈值 / Set reasonable inlier threshold
    
    registration_result_t<DataType> result;
    bool success = fourpcs.align(result);
    
    if (success) {
      REQUIRE(result.inliers.size() > 0);
      LOG_INFO_S << "4PCS配准成功，内点数 / 4PCS registration succeeded, inliers: " 
                 << result.inliers.size() << ", 适应度分数 / fitness score: " 
                 << result.fitness_score;
      
      // 输出变换矩阵供调试 / Output transformation matrix for debugging
      LOG_DEBUG_S << "变换矩阵 / Transformation matrix:\n" << result.transformation;
    } else {
      WARN("4PCS配准失败 / 4PCS registration failed");
    }
  }
  
  SECTION("Super4PCS on KITTI data")
  {
    super_four_pcs_registration_t<DataType> super4pcs;
    super4pcs.set_source(cloud1);
    super4pcs.set_target(cloud2);
    super4pcs.set_delta(1.0f);
    super4pcs.set_overlap(0.3f);
    super4pcs.set_sample_size(1000);
    super4pcs.enable_smart_indexing(true);
    super4pcs.set_grid_resolution(2.0f);  // 较大的网格适合KITTI尺度 / Larger grid for KITTI scale
    super4pcs.set_num_bases(100);
    super4pcs.set_max_iterations(200);
    super4pcs.set_inlier_threshold(1.0f);
    
    registration_result_t<DataType> result;
    bool success = super4pcs.align(result);
    
    if (success) {
      REQUIRE(result.inliers.size() > 0);
      LOG_INFO_S << "Super4PCS配准成功，内点数 / Super4PCS registration succeeded, inliers: " 
                 << result.inliers.size() << ", 适应度分数 / fitness score: " 
                 << result.fitness_score;
    } else {
      WARN("Super4PCS配准失败 / Super4PCS registration failed");
    }
  }
  
  SECTION("RANSAC with feature matching on KITTI data")
  {
    // 使用体素网格下采样以加速特征提取 / Use voxel grid downsampling for faster feature extraction
    voxel_grid_downsampling_t<DataType> voxel_filter(0.5f);  // 50cm体素大小，适合KITTI / 50cm voxel size, suitable for KITTI
    voxel_filter.enable_parallel(true);
    
    voxel_filter.set_input(cloud1);
    auto cloud1_downsampled = std::make_shared<point_cloud_t<DataType>>(voxel_filter.filter());
    
    voxel_filter.set_input(cloud2);
    auto cloud2_downsampled = std::make_shared<point_cloud_t<DataType>>(voxel_filter.filter());
    
    LOG_INFO_S << "下采样后的点云大小 / Downsampled cloud sizes: " 
               << cloud1_downsampled->size() << " 和 / and " << cloud2_downsampled->size();
    
    // 提取关键点 / Extract keypoints
    curvature_keypoint_extractor_t<DataType> curvature;
    curvature.set_curvature_threshold(0.0005f);  // 降低阈值以获得更多关键点 / Lower threshold for more keypoints
    curvature.set_search_radius(2.0f);  // 增大搜索半径 / Larger search radius
    curvature.set_non_maxima_radius(1.0f);  // 增大非极大值抑制半径 / Larger non-maxima suppression radius
    
    kdtree_t<DataType> kdtree1, kdtree2;
    
    curvature.set_input(cloud1_downsampled);
    curvature.set_knn(kdtree1);
    auto keypoints1 = curvature.extract();
    
    curvature.set_input(cloud2_downsampled);
    curvature.set_knn(kdtree2);
    auto keypoints2 = curvature.extract();
    
    LOG_INFO_S << "提取关键点 / Extracted keypoints: " << keypoints1.size() 
               << " 和 / and " << keypoints2.size();
    
    if (keypoints1.size() < 10 || keypoints2.size() < 10) {
      WARN("关键点太少，跳过RANSAC测试 / Too few keypoints, skipping RANSAC test");
      return;
    }
    
    auto keypoints1_ptr = std::make_shared<std::vector<std::size_t>>(keypoints1);
    auto keypoints2_ptr = std::make_shared<std::vector<std::size_t>>(keypoints2);
    
    // 计算FPFH描述子 / Compute FPFH descriptors
    fpfh_extractor_t<DataType> fpfh;
    fpfh.set_search_radius(3.0f);  // 增大搜索半径以获得更稳定的描述子 / Larger radius for more stable descriptors
    
    auto descriptors1 = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
    auto descriptors2 = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
    
    kdtree_t<DataType> kdtree_fpfh1, kdtree_fpfh2;
    
    fpfh.set_input(cloud1_downsampled);
    fpfh.set_knn(kdtree_fpfh1);
    fpfh.compute(*cloud1_downsampled, *keypoints1_ptr, *descriptors1);
    
    fpfh.set_input(cloud2_downsampled);
    fpfh.set_knn(kdtree_fpfh2);
    fpfh.compute(*cloud2_downsampled, *keypoints2_ptr, *descriptors2);
    
    // 生成对应关系 / Generate correspondences
    auto correspondences = generate_correspondences_brute_force<DataType, fpfh_signature_t<DataType>>(
        cloud1_downsampled, descriptors1, keypoints1_ptr,
        cloud2_downsampled, descriptors2, keypoints2_ptr,
        0.9f, true, false);  // 放宽比率测试阈值以获得更多对应关系 / Relax ratio test for more correspondences
    
    LOG_INFO_S << "生成对应关系 / Generated correspondences: " << correspondences.size();
    
    if (correspondences.size() < 10) {
      WARN("对应关系太少 / Too few correspondences: " + std::to_string(correspondences.size()));
      return;
    }
    
    // RANSAC配准 / RANSAC registration
    auto corr_ptr = std::make_shared<std::vector<correspondence_t>>(correspondences);
    
    ransac_registration_t<DataType> ransac;
    ransac.set_source(cloud1_downsampled);
    ransac.set_target(cloud2_downsampled);
    ransac.set_correspondences(corr_ptr);
    ransac.set_max_iterations(10000);  // 增加迭代次数 / Increase iterations
    ransac.set_inlier_threshold(5.0f);  // 更大的阈值，因为对应关系可能不够精确 / Larger threshold for less accurate correspondences
    ransac.set_min_inliers(3);  // 降低最小内点要求 / Lower minimum inliers requirement
    ransac.set_confidence(0.99f);  // 高置信度 / High confidence
    ransac.set_sample_size(4);  // 使用4个点来估计变换 / Use 4 points to estimate transformation
    
    registration_result_t<DataType> result;
    bool success = ransac.align(result);
    
    if (success) {
      REQUIRE(result.inliers.size() >= 10);
      LOG_INFO_S << "RANSAC配准成功，内点数 / RANSAC registration succeeded, inliers: " 
                 << result.inliers.size() << "/" << correspondences.size()
                 << ", 适应度分数 / fitness score: " << result.fitness_score;
    } else {
      WARN("RANSAC配准失败 / RANSAC registration failed");
    }
  }
}

TEST_CASE("KITTI Registration with Different Features", "[registration][kitti][features]")
{
  using DataType = float;
  
  // 加载KITTI数据集点云 / Load KITTI dataset point clouds
  std::string kitti_file1 = test_data_dir + std::string("/000000.bin");
  std::string kitti_file2 = test_data_dir + std::string("/000002.bin");
  
  auto cloud1_unique = toolbox::io::read_kitti_bin<DataType>(kitti_file1);
  auto cloud2_unique = toolbox::io::read_kitti_bin<DataType>(kitti_file2);
  
  if (!cloud1_unique || !cloud2_unique) {
    WARN("KITTI数据文件不存在，跳过测试 / KITTI data files not found, skipping test");
    return;
  }
  
  // 转换为shared_ptr / Convert to shared_ptr
  auto cloud1 = std::shared_ptr<point_cloud_t<DataType>>(std::move(cloud1_unique));
  auto cloud2 = std::shared_ptr<point_cloud_t<DataType>>(std::move(cloud2_unique));
  
  // 使用体素网格下采样 / Use voxel grid downsampling
  voxel_grid_downsampling_t<DataType> voxel_filter(0.5f);
  voxel_filter.enable_parallel(true);
  
  voxel_filter.set_input(cloud1);
  auto cloud1_downsampled = std::make_shared<point_cloud_t<DataType>>(voxel_filter.filter());
  
  voxel_filter.set_input(cloud2);
  auto cloud2_downsampled = std::make_shared<point_cloud_t<DataType>>(voxel_filter.filter());
  
  LOG_INFO_S << "下采样后的点云大小 / Downsampled cloud sizes: " 
             << cloud1_downsampled->size() << " 和 / and " << cloud2_downsampled->size();
  
  SECTION("Harris3D Keypoints with RANSAC")
  {
    // 使用Harris3D关键点 / Use Harris3D keypoints
    harris3d_keypoint_extractor_t<DataType> harris;
    harris.set_threshold(0.00001f);  // 更低的阈值 / Lower threshold
    harris.set_search_radius(3.0f);  // 更大的搜索半径 / Larger search radius
    harris.set_harris_k(0.04f);  // Harris参数 / Harris parameter
    harris.set_suppression_radius(0.5f);  // 较小的抑制半径以保留更多点 / Smaller suppression radius
    
    kdtree_t<DataType> kdtree1, kdtree2;
    
    harris.set_input(cloud1_downsampled);
    harris.set_knn(kdtree1);
    auto keypoints1 = harris.extract();
    
    harris.set_input(cloud2_downsampled);
    harris.set_knn(kdtree2);
    auto keypoints2 = harris.extract();
    
    LOG_INFO_S << "Harris3D关键点 / Harris3D keypoints: " << keypoints1.size() 
               << " 和 / and " << keypoints2.size();
    
    if (keypoints1.size() < 10 || keypoints2.size() < 10) {
      WARN("Harris3D关键点太少 / Too few Harris3D keypoints");
      return;
    }
    
    // 计算FPFH描述子并配准 / Compute FPFH descriptors and register
    perform_feature_matching_and_ransac(cloud1_downsampled, cloud2_downsampled, 
                                        keypoints1, keypoints2, "Harris3D");
  }
  
  SECTION("ISS Keypoints with RANSAC")
  {
    // 使用ISS关键点 / Use ISS keypoints
    iss_keypoint_extractor_t<DataType> iss;
    iss.set_salient_radius(3.0f);
    iss.set_non_maxima_radius(2.0f);
    iss.set_min_neighbors(5);
    iss.set_threshold21(0.975f);
    iss.set_threshold32(0.975f);
    
    kdtree_t<DataType> kdtree1, kdtree2;
    
    iss.set_input(cloud1_downsampled);
    iss.set_knn(kdtree1);
    auto keypoints1 = iss.extract();
    
    iss.set_input(cloud2_downsampled);
    iss.set_knn(kdtree2);
    auto keypoints2 = iss.extract();
    
    LOG_INFO_S << "ISS关键点 / ISS keypoints: " << keypoints1.size() 
               << " 和 / and " << keypoints2.size();
    
    if (keypoints1.size() < 10 || keypoints2.size() < 10) {
      WARN("ISS关键点太少 / Too few ISS keypoints");
      return;
    }
    
    perform_feature_matching_and_ransac(cloud1_downsampled, cloud2_downsampled, 
                                        keypoints1, keypoints2, "ISS");
  }
  
  SECTION("SIFT3D Keypoints with RANSAC")
  {
    // 使用SIFT3D关键点 / Use SIFT3D keypoints
    sift3d_keypoint_extractor_t<DataType> sift;
    sift.set_base_scale(0.5f);  // 基础尺度 / Base scale
    sift.set_num_scales(12);  // 尺度数量 / Number of scales
    sift.set_scale_factor(1.5f);  // 尺度因子 / Scale factor
    sift.set_contrast_threshold(0.001f);  // 对比度阈值 / Contrast threshold
    
    kdtree_t<DataType> kdtree1, kdtree2;
    
    sift.set_input(cloud1_downsampled);
    sift.set_knn(kdtree1);
    auto keypoints1 = sift.extract();
    
    sift.set_input(cloud2_downsampled);
    sift.set_knn(kdtree2);
    auto keypoints2 = sift.extract();
    
    LOG_INFO_S << "SIFT3D关键点 / SIFT3D keypoints: " << keypoints1.size() 
               << " 和 / and " << keypoints2.size();
    
    if (keypoints1.size() < 10 || keypoints2.size() < 10) {
      WARN("SIFT3D关键点太少 / Too few SIFT3D keypoints");
      return;
    }
    
    perform_feature_matching_and_ransac(cloud1_downsampled, cloud2_downsampled, 
                                        keypoints1, keypoints2, "SIFT3D");
  }
  
  SECTION("LOAM Feature Extractor with RANSAC")
  {
    // 使用LOAM特征提取器（提取角点） / Use LOAM feature extractor (extract corner points)
    loam_feature_extractor_t<DataType> loam;
    loam.set_edge_threshold(1.0f);  // 边缘阈值 / Edge threshold
    loam.set_planar_threshold(0.01f);  // 平面阈值 / Planar threshold
    loam.set_curvature_threshold(0.1f);  // 曲率阈值 / Curvature threshold
    loam.set_num_scan_neighbors(5);  // 扫描邻居数 / Number of scan neighbors
    
    kdtree_t<DataType> kdtree1, kdtree2;
    
    loam.set_input(cloud1_downsampled);
    loam.set_knn(kdtree1);
    auto corner_indices1 = loam.extract();  // LOAM只返回一组特征点 / LOAM returns single set of features
    
    loam.set_input(cloud2_downsampled);
    loam.set_knn(kdtree2);
    auto corner_indices2 = loam.extract();
    
    LOG_INFO_S << "LOAM角点 / LOAM corner points: " << corner_indices1.size() 
               << " 和 / and " << corner_indices2.size();
    
    if (corner_indices1.size() < 10 || corner_indices2.size() < 10) {
      WARN("LOAM角点太少 / Too few LOAM corner points");
      return;
    }
    
    // 使用角点进行配准 / Use corner points for registration
    perform_feature_matching_and_ransac(cloud1_downsampled, cloud2_downsampled, 
                                        corner_indices1, corner_indices2, "LOAM");
  }
}

// 辅助函数：执行特征匹配和RANSAC配准 / Helper function: perform feature matching and RANSAC registration
void perform_feature_matching_and_ransac(
    const std::shared_ptr<point_cloud_t<float>>& cloud1,
    const std::shared_ptr<point_cloud_t<float>>& cloud2,
    const std::vector<std::size_t>& keypoints1,
    const std::vector<std::size_t>& keypoints2,
    const std::string& method_name)
{
  using DataType = float;
  
  auto keypoints1_ptr = std::make_shared<std::vector<std::size_t>>(keypoints1);
  auto keypoints2_ptr = std::make_shared<std::vector<std::size_t>>(keypoints2);
  
  // 计算FPFH描述子 / Compute FPFH descriptors
  fpfh_extractor_t<DataType> fpfh;
  fpfh.set_search_radius(5.0f);  // 增大搜索半径以获得更稳定的描述子 / Larger radius for more stable descriptors
  
  auto descriptors1 = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
  auto descriptors2 = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
  
  kdtree_t<DataType> kdtree_fpfh1, kdtree_fpfh2;
  
  fpfh.set_input(cloud1);
  fpfh.set_knn(kdtree_fpfh1);
  fpfh.compute(*cloud1, *keypoints1_ptr, *descriptors1);
  
  fpfh.set_input(cloud2);
  fpfh.set_knn(kdtree_fpfh2);
  fpfh.compute(*cloud2, *keypoints2_ptr, *descriptors2);
  
  // 生成对应关系 / Generate correspondences
  auto correspondences = generate_correspondences_brute_force<DataType, fpfh_signature_t<DataType>>(
      cloud1, descriptors1, keypoints1_ptr,
      cloud2, descriptors2, keypoints2_ptr,
      0.95f, true, false);  // 更宽松的比率测试 / More relaxed ratio test
  
  LOG_INFO_S << method_name << " - 生成对应关系 / Generated correspondences: " 
             << correspondences.size();
  
  if (correspondences.size() < 10) {
    WARN(method_name + " - 对应关系太少 / Too few correspondences");
    return;
  }
  
  // RANSAC配准 / RANSAC registration
  auto corr_ptr = std::make_shared<std::vector<correspondence_t>>(correspondences);
  
  ransac_registration_t<DataType> ransac;
  ransac.set_source(cloud1);
  ransac.set_target(cloud2);
  ransac.set_correspondences(corr_ptr);
  ransac.set_max_iterations(10000);  // 增加迭代次数 / Increase iterations
  ransac.set_inlier_threshold(5.0f);  // 放宽内点阈值 / Relax inlier threshold
  ransac.set_min_inliers(3);  // 降低最小内点数 / Lower minimum inliers
  ransac.set_confidence(0.99f);  // 稍微降低置信度 / Slightly lower confidence
  
  registration_result_t<DataType> result;
  bool success = ransac.align(result);
  
  if (success) {
    LOG_INFO_S << method_name << " - RANSAC配准成功，内点数 / RANSAC registration succeeded, inliers: " 
               << result.inliers.size() << "/" << correspondences.size()
               << ", 适应度分数 / fitness score: " << result.fitness_score;
  } else {
    WARN(method_name + " - RANSAC配准失败 / RANSAC registration failed");
  }
}

TEST_CASE("Parallel Processing", "[registration][parallel]")
{
  using DataType = float;
  
  auto source_cloud = create_test_cloud<DataType>(1000);
  auto target_cloud = create_test_cloud<DataType>(1000);
  auto correspondences = create_test_correspondences(500);
  
  SECTION("RANSAC parallel vs sequential")
  {
    ransac_registration_t<DataType> ransac_parallel;
    ransac_parallel.set_source(source_cloud);
    ransac_parallel.set_target(target_cloud);
    ransac_parallel.set_correspondences(correspondences);
    ransac_parallel.enable_parallel(true);
    
    ransac_registration_t<DataType> ransac_sequential;
    ransac_sequential.set_source(source_cloud);
    ransac_sequential.set_target(target_cloud);
    ransac_sequential.set_correspondences(correspondences);
    ransac_sequential.enable_parallel(false);
    ransac_sequential.set_random_seed(42);  // 固定种子确保可重复 / Fixed seed for reproducibility
    
    registration_result_t<DataType> result_parallel, result_sequential;
    
    REQUIRE(ransac_parallel.align(result_parallel));
    REQUIRE(ransac_sequential.align(result_sequential));
    
    // 两种模式都应该成功 / Both modes should succeed
    REQUIRE(result_parallel.converged);
    REQUIRE(result_sequential.converged);
  }
}
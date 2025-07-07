#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Dense>
#include <memory>
#include <random>
#include <vector>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace Catch::Matchers;

// 辅助函数：创建带有已知变换的测试数据 / Helper: create test data with known transformation
template<typename DataType>
void create_registration_test_data(
    std::shared_ptr<point_cloud_t<DataType>>& source_cloud,
    std::shared_ptr<point_cloud_t<DataType>>& target_cloud,
    std::shared_ptr<std::vector<correspondence_t>>& correspondences,
    Eigen::Matrix<DataType, 4, 4>& true_transform,
    std::size_t num_inliers = 30,
    std::size_t num_outliers = 20,
    DataType noise_level = 0.01)
{
  // 创建源点云 / Create source cloud
  source_cloud = std::make_shared<point_cloud_t<DataType>>();
  target_cloud = std::make_shared<point_cloud_t<DataType>>();
  correspondences = std::make_shared<std::vector<correspondence_t>>();

  // 定义真实变换 / Define true transformation
  true_transform = Eigen::Matrix<DataType, 4, 4>::Identity();
  
  // 旋转：绕Z轴30度 / Rotation: 30 degrees around Z axis
  DataType angle = M_PI / 6;  // 30 degrees
  true_transform(0, 0) = std::cos(angle);
  true_transform(0, 1) = -std::sin(angle);
  true_transform(1, 0) = std::sin(angle);
  true_transform(1, 1) = std::cos(angle);
  
  // 平移 / Translation
  true_transform(0, 3) = 1.5;
  true_transform(1, 3) = 2.0;
  true_transform(2, 3) = 0.5;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<DataType> coord_dist(-5.0, 5.0);
  std::uniform_real_distribution<DataType> noise_dist(-noise_level, noise_level);

  // 创建内点 / Create inliers
  for (std::size_t i = 0; i < num_inliers; ++i) {
    // 源点 / Source point
    point_t<DataType> src_pt;
    src_pt.x = coord_dist(gen);
    src_pt.y = coord_dist(gen);
    src_pt.z = coord_dist(gen);
    source_cloud->points.push_back(src_pt);

    // 变换后的目标点（添加少量噪声） / Transformed target point (with small noise)
    Eigen::Vector4d src_vec(src_pt.x, src_pt.y, src_pt.z, 1.0);
    Eigen::Vector4d tgt_vec = true_transform.template cast<double>() * src_vec;
    
    point_t<DataType> tgt_pt;
    tgt_pt.x = tgt_vec[0] + noise_dist(gen);
    tgt_pt.y = tgt_vec[1] + noise_dist(gen);
    tgt_pt.z = tgt_vec[2] + noise_dist(gen);
    target_cloud->points.push_back(tgt_pt);

    // 创建对应关系 / Create correspondence
    correspondence_t corr;
    corr.src_idx = i;
    corr.dst_idx = i;
    // 内点有较小的描述子距离 / Inliers have smaller descriptor distances
    corr.distance = 0.1f + i * 0.001f;
    correspondences->push_back(corr);
  }

  // 创建外点 / Create outliers
  std::uniform_int_distribution<std::size_t> idx_dist(0, num_inliers - 1);
  for (std::size_t i = 0; i < num_outliers; ++i) {
    // 随机源点 / Random source point
    point_t<DataType> src_pt;
    src_pt.x = coord_dist(gen);
    src_pt.y = coord_dist(gen);
    src_pt.z = coord_dist(gen);
    source_cloud->points.push_back(src_pt);

    // 随机目标点（不遵循变换） / Random target point (not following transformation)
    point_t<DataType> tgt_pt;
    tgt_pt.x = coord_dist(gen);
    tgt_pt.y = coord_dist(gen);
    tgt_pt.z = coord_dist(gen);
    target_cloud->points.push_back(tgt_pt);

    // 创建错误的对应关系 / Create wrong correspondence
    correspondence_t corr;
    corr.src_idx = num_inliers + i;
    corr.dst_idx = num_inliers + i;
    // 外点有较大的描述子距离 / Outliers have larger descriptor distances
    corr.distance = 1.0f + i * 0.1f;
    correspondences->push_back(corr);
  }
}

// 辅助函数：计算变换误差 / Helper: compute transformation error
template<typename DataType>
DataType compute_transform_error(const Eigen::Matrix<DataType, 4, 4>& T1,
                                 const Eigen::Matrix<DataType, 4, 4>& T2)
{
  Eigen::Matrix<DataType, 4, 4> diff = T1 - T2;
  return diff.norm();
}

TEST_CASE("PROSAC Basic Functionality", "[registration][prosac]")
{
  using DataType = float;
  
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  Eigen::Matrix<DataType, 4, 4> true_transform;
  
  create_registration_test_data<DataType>(
      source_cloud, target_cloud, correspondences, true_transform,
      30, 20, 0.01f);  // 30 inliers, 20 outliers, 0.01 noise

  SECTION("基本PROSAC配准 / Basic PROSAC registration")
  {
    // 使用描述子距离排序 / Sort by descriptor distance
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_correspondences(correspondences);
    sorter.set_invert_score(true);  // 距离越小质量越高 / Smaller distance = higher quality
    
    std::vector<DataType> quality_scores;
    auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

    // PROSAC配准 / PROSAC registration
    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences, sorted_indices, quality_scores);
    prosac.set_max_iterations(1000);
    prosac.set_inlier_threshold(0.05f);
    prosac.set_confidence(0.99f);
    prosac.set_sample_size(3);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    REQUIRE(success);
    REQUIRE(result.converged);
    REQUIRE(result.inliers.size() >= 25);  // 应该找到大部分内点 / Should find most inliers
    
    // 检查变换精度 / Check transformation accuracy
    DataType transform_error = compute_transform_error(result.transformation, true_transform);
    REQUIRE(transform_error < 0.1f);
  }

  SECTION("不同参数测试 / Different parameters test")
  {
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_correspondences(correspondences);
    sorter.set_invert_score(true);
    
    std::vector<DataType> quality_scores;
    auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences, sorted_indices);
    
    // 测试不同的参数 / Test different parameters
    prosac.set_initial_inlier_ratio(0.3f);  // 假设30%内点率 / Assume 30% inlier ratio
    prosac.set_non_randomness_threshold(0.01f);  // 更严格的阈值 / Stricter threshold
    prosac.set_early_stop_ratio(0.8f);
    prosac.set_refine_result(true);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    REQUIRE(success);
    REQUIRE(result.num_iterations < 1000);  // 应该提前停止 / Should stop early
  }

  SECTION("无排序索引测试 / No sorted indices test")
  {
    // 直接使用对应关系，假设已排序 / Use correspondences directly, assume sorted
    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences);  // 不提供排序索引 / No sorted indices
    prosac.set_max_iterations(2000);
    prosac.set_inlier_threshold(0.05f);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    // 因为对应关系实际上是按质量排序的，所以应该成功 / Should succeed since correspondences are actually sorted by quality
    REQUIRE(success);
  }
}

TEST_CASE("PROSAC vs RANSAC Comparison", "[registration][prosac][ransac]")
{
  using DataType = float;
  
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  Eigen::Matrix<DataType, 4, 4> true_transform;
  
  // 创建具有高外点率的数据 / Create data with high outlier ratio
  create_registration_test_data<DataType>(
      source_cloud, target_cloud, correspondences, true_transform,
      20, 80, 0.005f);  // 20 inliers, 80 outliers (80% outlier ratio)

  SECTION("速度比较 / Speed comparison")
  {
    // 排序对应关系 / Sort correspondences
    descriptor_distance_sorter_t<DataType> sorter;
    sorter.set_correspondences(correspondences);
    sorter.set_invert_score(true);
    
    std::vector<DataType> quality_scores;
    auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

    // PROSAC
    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences, sorted_indices);
    prosac.set_max_iterations(5000);
    prosac.set_inlier_threshold(0.02f);
    prosac.set_confidence(0.99f);

    registration_result_t<DataType> prosac_result;
    auto prosac_start = std::chrono::high_resolution_clock::now();
    bool prosac_success = prosac.align(prosac_result);
    auto prosac_end = std::chrono::high_resolution_clock::now();
    auto prosac_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        prosac_end - prosac_start).count();

    // RANSAC
    ransac_registration_t<DataType> ransac;
    ransac.set_source(source_cloud);
    ransac.set_target(target_cloud);
    ransac.set_correspondences(correspondences);
    ransac.set_max_iterations(5000);
    ransac.set_inlier_threshold(0.02f);
    ransac.set_confidence(0.99f);

    registration_result_t<DataType> ransac_result;
    auto ransac_start = std::chrono::high_resolution_clock::now();
    bool ransac_success = ransac.align(ransac_result);
    auto ransac_end = std::chrono::high_resolution_clock::now();
    auto ransac_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        ransac_end - ransac_start).count();

    // 两者都应该成功 / Both should succeed
    REQUIRE(prosac_success);
    REQUIRE(ransac_success);

    // PROSAC应该使用更少的迭代 / PROSAC should use fewer iterations
    REQUIRE(prosac_result.num_iterations < ransac_result.num_iterations);

    // 两者应该找到相似数量的内点 / Both should find similar number of inliers
    REQUIRE(std::abs(static_cast<int>(prosac_result.inliers.size()) - 
                     static_cast<int>(ransac_result.inliers.size())) <= 5);

    // 记录性能信息 / Log performance info
    INFO("PROSAC iterations: " << prosac_result.num_iterations << 
         ", time: " << prosac_time << "ms");
    INFO("RANSAC iterations: " << ransac_result.num_iterations << 
         ", time: " << ransac_time << "ms");
  }
}

TEST_CASE("PROSAC Edge Cases", "[registration][prosac]")
{
  using DataType = float;

  SECTION("对应关系不足 / Insufficient correspondences")
  {
    auto source_cloud = std::make_shared<point_cloud_t<DataType>>();
    auto target_cloud = std::make_shared<point_cloud_t<DataType>>();
    auto correspondences = std::make_shared<std::vector<correspondence_t>>();

    // 只创建2个对应关系（少于最小样本大小3） / Only create 2 correspondences (less than minimum sample size 3)
    for (int i = 0; i < 2; ++i) {
      point_t<DataType> pt;
      pt.x = pt.y = pt.z = i;
      source_cloud->points.push_back(pt);
      target_cloud->points.push_back(pt);

      correspondence_t corr;
      corr.src_idx = i;
      corr.dst_idx = i;
      corr.distance = 0.1f;
      correspondences->push_back(corr);
    }

    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    REQUIRE(!success);
    REQUIRE(!result.converged);
  }

  SECTION("所有对应关系都是外点 / All correspondences are outliers")
  {
    std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
    std::shared_ptr<std::vector<correspondence_t>> correspondences;
    Eigen::Matrix<DataType, 4, 4> true_transform;
    
    // 创建只有外点的数据 / Create data with only outliers
    create_registration_test_data<DataType>(
        source_cloud, target_cloud, correspondences, true_transform,
        0, 50, 0.01f);  // 0 inliers, 50 outliers

    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences);
    prosac.set_max_iterations(100);
    prosac.set_inlier_threshold(0.01f);
    prosac.set_min_inliers(5);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    REQUIRE(!success);
    REQUIRE(result.inliers.size() < 5);
  }

  SECTION("完美对应关系 / Perfect correspondences")
  {
    auto source_cloud = std::make_shared<point_cloud_t<DataType>>();
    auto target_cloud = std::make_shared<point_cloud_t<DataType>>();
    auto correspondences = std::make_shared<std::vector<correspondence_t>>();

    // 创建完美匹配的对应关系 / Create perfectly matching correspondences
    Eigen::Matrix<DataType, 4, 4> transform = Eigen::Matrix<DataType, 4, 4>::Identity();
    transform(0, 3) = 1.0f;  // 只有平移 / Translation only

    for (int i = 0; i < 20; ++i) {
      point_t<DataType> src_pt;
      src_pt.x = i * 0.5f;
      src_pt.y = i * 0.3f;
      src_pt.z = i * 0.2f;
      source_cloud->points.push_back(src_pt);

      point_t<DataType> tgt_pt;
      tgt_pt.x = src_pt.x + transform(0, 3);
      tgt_pt.y = src_pt.y + transform(1, 3);
      tgt_pt.z = src_pt.z + transform(2, 3);
      target_cloud->points.push_back(tgt_pt);

      correspondence_t corr;
      corr.src_idx = i;
      corr.dst_idx = i;
      corr.distance = 0.01f;  // 很小的距离 / Very small distance
      correspondences->push_back(corr);
    }

    prosac_registration_t<DataType> prosac;
    prosac.set_source(source_cloud);
    prosac.set_target(target_cloud);
    prosac.set_sorted_correspondences(correspondences);
    prosac.set_max_iterations(1000);
    prosac.set_inlier_threshold(0.01f);
    prosac.set_early_stop_ratio(0.9f);

    registration_result_t<DataType> result;
    bool success = prosac.align(result);

    REQUIRE(success);
    REQUIRE(result.num_iterations < 50);  // 应该很快收敛 / Should converge quickly
    REQUIRE(result.inliers.size() == 20);  // 所有都是内点 / All are inliers
    
    // 检查找到的变换 / Check found transformation
    DataType transform_error = compute_transform_error(result.transformation, transform);
    REQUIRE(transform_error < 0.001f);
  }
}
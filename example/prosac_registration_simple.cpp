/**
 * @file prosac_registration_simple.cpp
 * @brief 简化的 PROSAC 配准算法示例 / Simplified PROSAC registration example
 */

#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Dense>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>

using namespace toolbox;
using namespace toolbox::pcl;
using namespace toolbox::types;

// 日志宏 / Logger macros
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()

// 生成简单的测试数据 / Generate simple test data
template<typename DataType>
void generate_test_data(
    std::shared_ptr<point_cloud_t<DataType>>& source_cloud,
    std::shared_ptr<point_cloud_t<DataType>>& target_cloud,
    std::shared_ptr<std::vector<correspondence_t>>& correspondences,
    Eigen::Matrix<DataType, 4, 4>& true_transform,
    std::size_t num_inliers = 30,
    std::size_t num_outliers = 20)
{
  source_cloud = std::make_shared<point_cloud_t<DataType>>();
  target_cloud = std::make_shared<point_cloud_t<DataType>>();
  correspondences = std::make_shared<std::vector<correspondence_t>>();

  // 定义真实变换 / Define true transformation
  true_transform = Eigen::Matrix<DataType, 4, 4>::Identity();
  true_transform(0, 3) = 2.0;  // X平移
  true_transform(1, 3) = 1.0;  // Y平移
  true_transform(2, 3) = 0.5;  // Z平移

  std::random_device rd;
  std::mt19937 gen(42);  // 固定种子
  std::uniform_real_distribution<DataType> dist(-10.0, 10.0);
  std::normal_distribution<DataType> noise(0.0, 0.01);

  // 创建内点 / Create inliers
  for (std::size_t i = 0; i < num_inliers; ++i) {
    point_t<DataType> src_pt;
    src_pt.x = dist(gen);
    src_pt.y = dist(gen);
    src_pt.z = dist(gen);
    source_cloud->points.push_back(src_pt);

    // 应用变换 / Apply transformation
    Eigen::Vector4d src_vec(src_pt.x, src_pt.y, src_pt.z, 1.0);
    Eigen::Vector4d tgt_vec = true_transform.template cast<double>() * src_vec;
    
    point_t<DataType> tgt_pt;
    tgt_pt.x = tgt_vec[0] + noise(gen);
    tgt_pt.y = tgt_vec[1] + noise(gen);
    tgt_pt.z = tgt_vec[2] + noise(gen);
    target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = i;
    corr.dst_idx = i;
    corr.distance = 0.1f + i * 0.001f;  // 递增的质量分数
    correspondences->push_back(corr);
  }

  // 创建外点 / Create outliers
  for (std::size_t i = 0; i < num_outliers; ++i) {
    point_t<DataType> src_pt;
    src_pt.x = dist(gen);
    src_pt.y = dist(gen);
    src_pt.z = dist(gen);
    source_cloud->points.push_back(src_pt);

    point_t<DataType> tgt_pt;
    tgt_pt.x = dist(gen);
    tgt_pt.y = dist(gen);
    tgt_pt.z = dist(gen);
    target_cloud->points.push_back(tgt_pt);

    correspondence_t corr;
    corr.src_idx = num_inliers + i;
    corr.dst_idx = num_inliers + i;
    corr.distance = 1.0f + i * 0.1f;  // 较大的距离表示较低质量
    correspondences->push_back(corr);
  }
}

int main()
{
  using DataType = float;

  // 初始化日志 / Initialize logger
  auto& logger = logger::thread_logger_t::instance();
  
  LOG_INFO_S << "=== 简化的 PROSAC 配准示例 / Simplified PROSAC Registration Example ===";

  // 1. 生成测试数据 / Generate test data
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  std::shared_ptr<std::vector<correspondence_t>> correspondences;
  Eigen::Matrix<DataType, 4, 4> true_transform;
  
  generate_test_data<DataType>(
      source_cloud, target_cloud, correspondences, true_transform,
      30, 70);  // 30 内点, 70 外点 (70% 外点率)

  LOG_INFO_S << "生成的数据 / Generated data:";
  LOG_INFO_S << "  源点云点数 / Source points: " << source_cloud->points.size();
  LOG_INFO_S << "  目标点云点数 / Target points: " << target_cloud->points.size();
  LOG_INFO_S << "  对应关系数 / Correspondences: " << correspondences->size();

  // 2. 对对应关系进行排序 / Sort correspondences
  LOG_INFO_S << "\n使用描述子距离排序 / Sorting by descriptor distance...";
  
  descriptor_distance_sorter_t<DataType> sorter;
  sorter.set_correspondences(correspondences);
  sorter.set_invert_score(true);  // 距离越小质量越高
  
  std::vector<DataType> quality_scores;
  auto sorted_indices = sorter.compute_sorted_indices(quality_scores);

  // 3. PROSAC 配准 / PROSAC registration
  LOG_INFO_S << "\n运行 PROSAC 配准 / Running PROSAC registration...";
  
  prosac_registration_t<DataType> prosac;
  prosac.set_source(source_cloud);
  prosac.set_target(target_cloud);
  prosac.set_sorted_correspondences(correspondences, sorted_indices);
  prosac.set_max_iterations(1000);
  prosac.set_inlier_threshold(0.05f);
  prosac.set_confidence(0.99f);

  registration_result_t<DataType> prosac_result;
  
  auto prosac_start = std::chrono::high_resolution_clock::now();
  bool prosac_success = prosac.align(prosac_result);
  auto prosac_end = std::chrono::high_resolution_clock::now();
  
  auto prosac_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      prosac_end - prosac_start).count();

  if (prosac_success) {
    LOG_INFO_S << "PROSAC 成功 / PROSAC successful:";
    LOG_INFO_S << "  迭代次数 / Iterations: " << prosac_result.num_iterations;
    LOG_INFO_S << "  内点数 / Inliers: " << prosac_result.inliers.size() << "/" << correspondences->size();
    LOG_INFO_S << "  运行时间 / Runtime: " << prosac_time << " ms";
    
    Eigen::Matrix<DataType, 4, 4> error_matrix = prosac_result.transformation - true_transform;
    DataType transform_error = error_matrix.norm();
    LOG_INFO_S << "  变换误差 / Transform error: " << transform_error;
  } else {
    LOG_ERROR_S << "PROSAC 失败 / PROSAC failed";
  }

  // 4. RANSAC 对比 / RANSAC comparison
  LOG_INFO_S << "\n运行 RANSAC 配准进行对比 / Running RANSAC registration for comparison...";
  
  ransac_registration_t<DataType> ransac;
  ransac.set_source(source_cloud);
  ransac.set_target(target_cloud);
  ransac.set_correspondences(correspondences);
  ransac.set_max_iterations(1000);
  ransac.set_inlier_threshold(0.05f);
  ransac.set_confidence(0.99f);

  registration_result_t<DataType> ransac_result;
  
  auto ransac_start = std::chrono::high_resolution_clock::now();
  bool ransac_success = ransac.align(ransac_result);
  auto ransac_end = std::chrono::high_resolution_clock::now();
  
  auto ransac_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      ransac_end - ransac_start).count();

  if (ransac_success) {
    LOG_INFO_S << "RANSAC 成功 / RANSAC successful:";
    LOG_INFO_S << "  迭代次数 / Iterations: " << ransac_result.num_iterations;
    LOG_INFO_S << "  内点数 / Inliers: " << ransac_result.inliers.size();
    LOG_INFO_S << "  运行时间 / Runtime: " << ransac_time << " ms";
  }

  // 5. 总结 / Summary
  LOG_INFO_S << "\n=== 总结 / Summary ===";
  if (prosac_success && ransac_success) {
    DataType speedup = static_cast<DataType>(ransac_result.num_iterations) / prosac_result.num_iterations;
    LOG_INFO_S << "PROSAC 相对于 RANSAC 的加速比 / PROSAC speedup over RANSAC: " << speedup << "x";
  }

  return 0;
}
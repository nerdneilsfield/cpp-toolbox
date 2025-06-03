#include <iostream>
#include <memory>

#include <cpp-toolbox/io/formats.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/features/curvature_keypoints.hpp>
#include <cpp-toolbox/pcl/registration/registration.hpp>
#include <cpp-toolbox/utils/print.hpp>
#include <cpp-toolbox/utils/timer.hpp>

using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::io;
using namespace toolbox::utils;

/**
 * @brief 粗配准示例程序 / Coarse registration example program
 *
 * 本示例演示了如何使用cpp-toolbox中的三种粗配准算法：
 * This example demonstrates how to use three coarse registration algorithms in
 * cpp-toolbox:
 * 1. RANSAC - 基于对应关系的配准 / Correspondence-based registration
 * 2. 4PCS - 不需要初始对应关系的配准 / Registration without initial
 * correspondences
 * 3. Super4PCS - 4PCS的优化版本，适合大规模点云 / Optimized version of 4PCS for
 * large point clouds
 */

// 辅助函数：应用变换到点云 / Helper: apply transformation to point cloud
template<typename DataType>
std::shared_ptr<point_cloud_t<DataType>> transform_point_cloud(
    const std::shared_ptr<point_cloud_t<DataType>>& cloud,
    const Eigen::Matrix<DataType, 4, 4>& transform)
{
  auto transformed = std::make_shared<point_cloud_t<DataType>>(*cloud);

  for (auto& pt : transformed->points) {
    Eigen::Matrix<DataType, 4, 1> p;
    p << pt.x, pt.y, pt.z, 1.0;
    Eigen::Matrix<DataType, 4, 1> tp = transform * p;
    pt.x = tp[0];
    pt.y = tp[1];
    pt.z = tp[2];
  }

  return transformed;
}

// 辅助函数：加载PCD文件 / Helper: load PCD file
template<typename DataType>
std::shared_ptr<point_cloud_t<DataType>> load_point_cloud_file(
    const std::string& filename)
{
  auto cloud_unique = read_point_cloud<DataType>(filename);
  if (!cloud_unique) {
    std::cerr << "无法读取文件 / Failed to read file: " << filename
              << std::endl;
    return nullptr;
  }

  // Convert unique_ptr to shared_ptr
  return std::shared_ptr<point_cloud_t<DataType>>(std::move(cloud_unique));
}

// 示例1：RANSAC配准（需要特征匹配） / Example 1: RANSAC registration (requires
// feature matching)
void ransac_registration_example(const std::string& source_file,
                                 const std::string& target_file)
{
  using DataType = float;

  std::cout << "\n========== RANSAC 配准示例 / RANSAC Registration Example "
               "==========\n";

  // 1. 加载点云 / Load point clouds
  auto source_cloud = load_point_cloud_file<DataType>(source_file);
  auto target_cloud = load_point_cloud_file<DataType>(target_file);

  if (!source_cloud || !target_cloud) {
    std::cerr
        << "错误：无法加载点云文件 / Error: Failed to load point cloud files\n";
    return;
  }

  std::cout << "源点云大小 / Source cloud size: " << source_cloud->size()
            << " 点/points\n";
  std::cout << "目标点云大小 / Target cloud size: " << target_cloud->size()
            << " 点/points\n";

  // 2. 提取关键点 / Extract keypoints
  curvature_keypoint_extractor_t<DataType> curvature;
  curvature.set_curvature_threshold(0.001f);
  curvature.set_search_radius(0.05f);
  curvature.set_non_maxima_radius(0.01f);

  // 设置KNN算法 / Set KNN algorithm
  kdtree_t<DataType> kdtree_src, kdtree_tgt;

  curvature.set_input(source_cloud);
  curvature.set_knn(kdtree_src);
  auto src_keypoint_indices = curvature.extract();

  curvature.set_input(target_cloud);
  curvature.set_knn(kdtree_tgt);
  auto tgt_keypoint_indices = curvature.extract();

  std::cout << "源关键点 / Source keypoints: " << src_keypoint_indices.size()
            << std::endl;
  std::cout << "目标关键点 / Target keypoints: " << tgt_keypoint_indices.size()
            << std::endl;

  if (src_keypoint_indices.size() < 10 || tgt_keypoint_indices.size() < 10) {
    std::cerr
        << "关键点太少，跳过RANSAC / Too few keypoints, skipping RANSAC\n";
    return;
  }

  // 3. 计算描述子 / Compute descriptors
  fpfh_extractor_t<DataType> fpfh;
  fpfh.set_search_radius(0.05f);

  auto src_descriptors =
      std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
  auto tgt_descriptors =
      std::make_shared<std::vector<fpfh_signature_t<DataType>>>();

  // 设置KNN算法 / Set KNN algorithm for FPFH
  kdtree_t<DataType> kdtree_fpfh_src, kdtree_fpfh_tgt;

  fpfh.set_input(source_cloud);
  fpfh.set_knn(kdtree_fpfh_src);
  fpfh.compute(*source_cloud, src_keypoint_indices, *src_descriptors);

  fpfh.set_input(target_cloud);
  fpfh.set_knn(kdtree_fpfh_tgt);
  fpfh.compute(*target_cloud, tgt_keypoint_indices, *tgt_descriptors);

  std::cout << "计算了 " << src_descriptors->size()
            << " 个源描述子 / Computed source descriptors\n";
  std::cout << "计算了 " << tgt_descriptors->size()
            << " 个目标描述子 / Computed target descriptors\n";

  // 4. 生成对应关系 / Generate correspondences
  auto src_indices_ptr =
      std::make_shared<std::vector<std::size_t>>(src_keypoint_indices);
  auto tgt_indices_ptr =
      std::make_shared<std::vector<std::size_t>>(tgt_keypoint_indices);

  auto correspondences =
      generate_correspondences_brute_force<DataType,
                                           fpfh_signature_t<DataType>>(
          source_cloud,
          src_descriptors,
          src_indices_ptr,
          target_cloud,
          tgt_descriptors,
          tgt_indices_ptr,
          0.8f,
          true,
          false);

  std::cout << "生成了 " << correspondences.size()
            << " 个对应关系 / Generated correspondences\n";

  if (correspondences.size() < 3) {
    std::cerr << "对应关系太少，无法进行RANSAC / Too few correspondences for "
                 "RANSAC\n";
    return;
  }

  // 5. RANSAC配准 / RANSAC registration
  auto corr_ptr =
      std::make_shared<std::vector<correspondence_t>>(correspondences);

  ransac_registration_t<DataType> ransac;
  ransac.set_source(source_cloud);
  ransac.set_target(target_cloud);
  ransac.set_correspondences(corr_ptr);
  ransac.set_max_iterations(1000);
  ransac.set_inlier_threshold(0.01f);
  ransac.set_confidence(0.99f);

  stop_watch_timer_t timer("RANSAC");
  timer.start();

  registration_result_t<DataType> result;
  if (ransac.align(result)) {
    timer.stop();

    std::cout << "\nRANSAC配准成功 / RANSAC registration successful!\n";
    std::cout << "  耗时 / Time: " << timer.elapsed_time() << " 秒/s\n";
    std::cout << "  迭代次数 / Iterations: " << result.num_iterations << "\n";
    std::cout << "  内点数 / Inliers: " << result.inliers.size() << "/"
              << correspondences.size() << "\n";
    std::cout << "  适应度评分 / Fitness score: " << result.fitness_score
              << "\n";
    std::cout << "  变换矩阵 / Transformation matrix:\n"
              << result.transformation << "\n";
  } else {
    std::cout << "RANSAC配准失败 / RANSAC registration failed\n";
  }
}

// 示例2：4PCS配准（不需要初始对应关系） / Example 2: 4PCS registration (no
// initial correspondences)
void four_pcs_registration_example(const std::string& source_file,
                                   const std::string& target_file)
{
  using DataType = float;

  std::cout
      << "\n========== 4PCS 配准示例 / 4PCS Registration Example ==========\n";

  // 1. 加载点云 / Load point clouds
  auto source_cloud = load_point_cloud_file<DataType>(source_file);
  auto target_cloud = load_point_cloud_file<DataType>(target_file);

  if (!source_cloud || !target_cloud) {
    std::cerr
        << "错误：无法加载点云文件 / Error: Failed to load point cloud files\n";
    return;
  }

  std::cout << "源点云大小 / Source cloud size: " << source_cloud->size()
            << " 点/points\n";
  std::cout << "目标点云大小 / Target cloud size: " << target_cloud->size()
            << " 点/points\n";

  // 2. 设置4PCS参数 / Set 4PCS parameters
  four_pcs_registration_t<DataType> fourpcs;
  fourpcs.set_source(source_cloud);
  fourpcs.set_target(target_cloud);
  fourpcs.set_delta(0.02f);  // 配准精度 2cm / Registration accuracy 2cm
  fourpcs.set_overlap(0.5f);  // 假设50%重叠 / Assume 50% overlap
  fourpcs.set_sample_size(200);  // 采样200个点 / Sample 200 points
  fourpcs.set_num_bases(50);  // 尝试50个基 / Try 50 bases
  fourpcs.set_max_iterations(100);  // 最大100次迭代 / Max 100 iterations

  stop_watch_timer_t timer("4PCS");
  timer.start();

  registration_result_t<DataType> result;
  if (fourpcs.align(result)) {
    timer.stop();

    std::cout << "\n4PCS配准成功 / 4PCS registration successful!\n";
    std::cout << "  耗时 / Time: " << timer.elapsed_time() << " 秒/s\n";
    std::cout << "  内点数 / Inliers: " << result.inliers.size() << "\n";
    std::cout << "  适应度评分 / Fitness score: " << result.fitness_score
              << "\n";
    std::cout << "  变换矩阵 / Transformation matrix:\n"
              << result.transformation << "\n";
  } else {
    std::cout << "4PCS配准失败 / 4PCS registration failed\n";
  }
}

// 示例3：Super4PCS配准（智能索引优化） / Example 3: Super4PCS registration
// (smart indexing)
void super_four_pcs_registration_example(const std::string& source_file,
                                         const std::string& target_file)
{
  using DataType = float;

  std::cout << "\n========== Super4PCS 配准示例 / Super4PCS Registration "
               "Example ==========\n";

  // 1. 加载点云 / Load point clouds
  auto source_cloud = load_point_cloud_file<DataType>(source_file);
  auto target_cloud = load_point_cloud_file<DataType>(target_file);

  if (!source_cloud || !target_cloud) {
    std::cerr
        << "错误：无法加载点云文件 / Error: Failed to load point cloud files\n";
    return;
  }

  std::cout << "源点云大小 / Source cloud size: " << source_cloud->size()
            << " 点/points\n";
  std::cout << "目标点云大小 / Target cloud size: " << target_cloud->size()
            << " 点/points\n";

  // 2. 设置Super4PCS参数 / Set Super4PCS parameters
  super_four_pcs_registration_t<DataType> super4pcs;
  super4pcs.set_source(source_cloud);
  super4pcs.set_target(target_cloud);
  super4pcs.set_delta(0.02f);  // 配准精度 2cm / Registration accuracy 2cm
  super4pcs.set_overlap(0.5f);  // 假设50%重叠 / Assume 50% overlap
  super4pcs.set_sample_size(300);  // 采样300个点 / Sample 300 points
  super4pcs.enable_smart_indexing(
      true);  // 启用智能索引 / Enable smart indexing
  super4pcs.set_grid_resolution(0.05f);  // 网格分辨率 5cm / Grid resolution 5cm
  super4pcs.set_num_bases(30);  // 尝试30个基 / Try 30 bases
  super4pcs.set_max_iterations(100);  // 最大100次迭代 / Max 100 iterations

  stop_watch_timer_t timer("Super4PCS");
  timer.start();

  registration_result_t<DataType> result;
  if (super4pcs.align(result)) {
    timer.stop();

    std::cout << "\nSuper4PCS配准成功 / Super4PCS registration successful!\n";
    std::cout << "  算法名称 / Algorithm: " << super4pcs.get_algorithm_name()
              << "\n";
    std::cout << "  耗时 / Time: " << timer.elapsed_time() << " 秒/s\n";
    std::cout << "  内点数 / Inliers: " << result.inliers.size() << "\n";
    std::cout << "  适应度评分 / Fitness score: " << result.fitness_score
              << "\n";
    std::cout << "  变换矩阵 / Transformation matrix:\n"
              << result.transformation << "\n";
  } else {
    std::cout << "Super4PCS配准失败 / Super4PCS registration failed\n";
  }
}

// 示例4：使用快速配准函数 / Example 4: Using quick registration function
void quick_registration_example(const std::string& source_file,
                                const std::string& target_file)
{
  using DataType = float;

  std::cout
      << "\n========== 快速配准示例 / Quick Registration Example ==========\n";

  auto source_cloud = load_point_cloud_file<DataType>(source_file);
  auto target_cloud = load_point_cloud_file<DataType>(target_file);

  if (!source_cloud || !target_cloud) {
    std::cerr
        << "错误：无法加载点云文件 / Error: Failed to load point cloud files\n";
    return;
  }

  // 使用快速配准函数 / Use quick registration function
  auto result =
      quick_registration(source_cloud, target_cloud, "super4pcs", 0.5f);

  if (result.converged) {
    std::cout << "快速配准成功 / Quick registration successful!\n";
    std::cout << "  内点数 / Inliers: " << result.inliers.size() << "\n";
    std::cout << "  适应度评分 / Fitness score: " << result.fitness_score
              << "\n";
    std::cout << "  变换矩阵 / Transformation matrix:\n"
              << result.transformation << "\n";
  } else {
    std::cout << "快速配准失败 / Quick registration failed\n";
  }
}

int main(int argc, char** argv)
{
  toolbox::logger::thread_logger_t::instance().set_level(
      toolbox::logger::thread_logger_t::Level::DEBUG);

  LOG_INFO_S << "Starting registration example";
  if (argc < 3) {
    std::cout << "用法 / Usage: " << argv[0]
              << " <source.pcd> <target.pcd> [algorithm]\n";
    std::cout << "  algorithm: ransac, 4pcs, super4pcs, quick, all "
                 "(默认/default: all)\n";
    return 1;
  }

  std::string source_file = argv[1];
  std::string target_file = argv[2];
  std::string algorithm = (argc > 3) ? argv[3] : "all";

  if (algorithm == "ransac" || algorithm == "all") {
    try {
      ransac_registration_example(source_file, target_file);
    } catch (const std::exception& e) {
      std::cerr << "RANSAC错误 / RANSAC error: " << e.what() << std::endl;
    }
  }

  if (algorithm == "4pcs" || algorithm == "all") {
    try {
      four_pcs_registration_example(source_file, target_file);
    } catch (const std::exception& e) {
      std::cerr << "4PCS错误 / 4PCS error: " << e.what() << std::endl;
    }
  }

  if (algorithm == "super4pcs" || algorithm == "all") {
    try {
      super_four_pcs_registration_example(source_file, target_file);
    } catch (const std::exception& e) {
      std::cerr << "Super4PCS错误 / Super4PCS error: " << e.what() << std::endl;
    }
  }

  if (algorithm == "quick" || algorithm == "all") {
    try {
      quick_registration_example(source_file, target_file);
    } catch (const std::exception& e) {
      std::cerr << "快速配准错误 / Quick registration error: " << e.what()
                << std::endl;
    }
  }

  LOG_INFO_S << "Registration example finished";

  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
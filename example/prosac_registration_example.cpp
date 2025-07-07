/**
 * @file prosac_registration_example.cpp
 * @brief PROSAC 配准算法的综合示例 / Comprehensive example of PROSAC registration algorithm
 * 
 * 该示例展示了如何使用 PROSAC 进行点云配准，包括：
 * This example demonstrates how to use PROSAC for point cloud registration, including:
 * 1. 生成合成点云数据 / Generate synthetic point cloud data
 * 2. 提取特征和描述子 / Extract features and descriptors
 * 3. 生成对应关系 / Generate correspondences
 * 4. 使用不同的排序策略 / Use different sorting strategies
 * 5. 比较 PROSAC 和 RANSAC 的性能 / Compare PROSAC and RANSAC performance
 * 6. 可视化结果 / Visualize results
 */

#include <cpp-toolbox/io/formats.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/pcl/descriptors/descriptors.hpp>
#include <cpp-toolbox/pcl/features/features.hpp>
// #include <cpp-toolbox/pcl/filter/downsampling_filter.hpp>  // Not available yet
#include <cpp-toolbox/pcl/knn/knn.hpp>
#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Dense>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

using namespace toolbox;
using namespace toolbox::pcl;
using namespace toolbox::types;
using namespace toolbox::io;

// 日志宏 / Logger macros
#define LOG_INFO_S_S toolbox::logger::thread_logger_t::instance().info_s()
#define LOG_WARN_S_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_ERROR_S_S toolbox::logger::thread_logger_t::instance().error_s()

// 生成合成点云数据 / Generate synthetic point cloud data
template<typename DataType>
void generate_synthetic_clouds(
    std::shared_ptr<point_cloud_t<DataType>>& source_cloud,
    std::shared_ptr<point_cloud_t<DataType>>& target_cloud,
    Eigen::Matrix<DataType, 4, 4>& true_transform,
    std::size_t num_points = 1000,
    DataType noise_level = 0.01,
    DataType outlier_ratio = 0.3)
{
  source_cloud = std::make_shared<point_cloud_t<DataType>>();
  target_cloud = std::make_shared<point_cloud_t<DataType>>();

  // 定义真实变换 / Define true transformation
  true_transform = Eigen::Matrix<DataType, 4, 4>::Identity();
  
  // 旋转：组合旋转 / Rotation: combined rotation
  Eigen::AngleAxis<DataType> rotation_x(M_PI / 6, Eigen::Vector3d::UnitX());
  Eigen::AngleAxis<DataType> rotation_y(M_PI / 4, Eigen::Vector3d::UnitY());
  Eigen::AngleAxis<DataType> rotation_z(M_PI / 3, Eigen::Vector3d::UnitZ());
  
  Eigen::Matrix<DataType, 3, 3> rotation_matrix = 
      (rotation_z * rotation_y * rotation_x).toRotationMatrix().cast<DataType>();
  
  true_transform.template block<3, 3>(0, 0) = rotation_matrix;
  
  // 平移 / Translation
  true_transform(0, 3) = 2.5;
  true_transform(1, 3) = -1.0;
  true_transform(2, 3) = 3.0;

  std::random_device rd;
  std::mt19937 gen(42);  // 固定种子 / Fixed seed
  std::uniform_real_distribution<DataType> coord_dist(-5.0, 5.0);
  std::normal_distribution<DataType> noise_dist(0.0, noise_level);
  std::uniform_real_distribution<DataType> outlier_dist(0.0, 1.0);

  // 生成源点云（例如：Stanford Bunny 的简化版本） / Generate source cloud (e.g., simplified Stanford Bunny)
  for (std::size_t i = 0; i < num_points; ++i) {
    point_t<DataType> pt;
    
    // 创建一个有结构的点云（类似兔子形状） / Create structured point cloud (bunny-like shape)
    DataType theta = 2 * M_PI * i / num_points;
    DataType phi = M_PI * (i % 100) / 100 - M_PI / 2;
    DataType r = 2.0 + 0.5 * std::sin(5 * theta) * std::cos(3 * phi);
    
    pt.x = r * std::cos(theta) * std::cos(phi);
    pt.y = r * std::sin(theta) * std::cos(phi);
    pt.z = r * std::sin(phi);
    
    // 添加法线 / Add normals
    Eigen::Vector3f normal(pt.x, pt.y, pt.z);
    normal.normalize();
    pt.normal_x = normal.x();
    pt.normal_y = normal.y();
    pt.normal_z = normal.z();
    
    source_cloud->points.push_back(pt);
  }

  // 生成目标点云 / Generate target cloud
  std::size_t num_inliers = static_cast<std::size_t>(num_points * (1.0 - outlier_ratio));
  
  // 内点：变换源点云 / Inliers: transform source cloud
  for (std::size_t i = 0; i < num_inliers; ++i) {
    const auto& src_pt = source_cloud->points[i];
    
    // 应用变换 / Apply transformation
    Eigen::Vector4d src_vec(src_pt.x, src_pt.y, src_pt.z, 1.0);
    Eigen::Vector4d tgt_vec = true_transform.template cast<double>() * src_vec;
    
    point_t<DataType> tgt_pt;
    tgt_pt.x = tgt_vec[0] + noise_dist(gen);
    tgt_pt.y = tgt_vec[1] + noise_dist(gen);
    tgt_pt.z = tgt_vec[2] + noise_dist(gen);
    
    // 变换法线 / Transform normals
    Eigen::Vector3d src_normal(src_pt.normal_x, src_pt.normal_y, src_pt.normal_z);
    Eigen::Vector3d tgt_normal = rotation_matrix.template cast<double>() * src_normal;
    tgt_pt.normal_x = tgt_normal.x();
    tgt_pt.normal_y = tgt_normal.y();
    tgt_pt.normal_z = tgt_normal.z();
    
    target_cloud->points.push_back(tgt_pt);
  }
  
  // 外点：随机点 / Outliers: random points
  for (std::size_t i = num_inliers; i < num_points; ++i) {
    point_t<DataType> tgt_pt;
    tgt_pt.x = coord_dist(gen);
    tgt_pt.y = coord_dist(gen);
    tgt_pt.z = coord_dist(gen);
    
    // 随机法线 / Random normals
    Eigen::Vector3f normal(coord_dist(gen), coord_dist(gen), coord_dist(gen));
    normal.normalize();
    tgt_pt.normal_x = normal.x();
    tgt_pt.normal_y = normal.y();
    tgt_pt.normal_z = normal.z();
    
    target_cloud->points.push_back(tgt_pt);
  }
}

// 主函数 / Main function
int main(int argc, char* argv[])
{
  using DataType = float;
  
  // 初始化日志 / Initialize logger
  auto& logger = logger::thread_logger_t::instance();
  logger.set_level(logger::level_t::info);
  
  LOG_INFO_S << "=== PROSAC 配准示例 / PROSAC Registration Example ===";

  // 1. 生成或加载点云数据 / Generate or load point cloud data
  std::shared_ptr<point_cloud_t<DataType>> source_cloud, target_cloud;
  Eigen::Matrix<DataType, 4, 4> true_transform;
  
  if (argc > 2) {
    // 从文件加载 / Load from files
    LOG_INFO_S << "加载点云文件 / Loading point cloud files...";
    source_cloud = std::make_shared<point_cloud_t<DataType>>();
    target_cloud = std::make_shared<point_cloud_t<DataType>>();
    
    source_cloud = read_pcd<DataType>(argv[1]);
    target_cloud = read_pcd<DataType>(argv[2]);
    
    if (!source_cloud || !target_cloud) {
      LOG_ERROR_S << "无法加载点云文件 / Failed to load point cloud files";
      return -1;
    }
  } else {
    // 生成合成数据 / Generate synthetic data
    LOG_INFO_S << "生成合成点云数据 / Generating synthetic point cloud data...";
    generate_synthetic_clouds<DataType>(
        source_cloud, target_cloud, true_transform,
        1000,    // 点数 / Number of points
        0.01f,   // 噪声水平 / Noise level
        0.3f     // 外点率 / Outlier ratio
    );
    
    LOG_INFO_S << "真实变换矩阵 / True transformation matrix:";
    std::cout << true_transform << std::endl;
  }
  
  LOG_INFO_S << "源点云点数 / Source cloud points: " << source_cloud->points.size();
  LOG_INFO_S << "目标点云点数 / Target cloud points: " << target_cloud->points.size();

  // 2. 降采样（可选） / Downsampling (optional)
  /*
  if (source_cloud->points.size() > 5000) {
    LOG_INFO_S << "降采样点云 / Downsampling point clouds...";
    
    voxel_grid_filter_t<DataType> voxel_filter;
    voxel_filter.set_voxel_size(0.1f);  // 10cm 体素 / 10cm voxels
    
    auto source_downsampled = std::make_shared<point_cloud_t<DataType>>();
    auto target_downsampled = std::make_shared<point_cloud_t<DataType>>();
    
    voxel_filter.filter(*source_cloud, *source_downsampled);
    voxel_filter.filter(*target_cloud, *target_downsampled);
    
    source_cloud = source_downsampled;
    target_cloud = target_downsampled;
    
    LOG_INFO_S << "降采样后源点云点数 / Downsampled source points: " << source_cloud->points.size();
    LOG_INFO_S << "降采样后目标点云点数 / Downsampled target points: " << target_cloud->points.size();
  }
  */

  // 3. 提取关键点 / Extract keypoints
  LOG_INFO_S << "提取 ISS 关键点 / Extracting ISS keypoints...";
  
  iss_detector_t<DataType> iss_detector;
  iss_detector.set_salient_radius(0.3f);
  iss_detector.set_non_max_radius(0.2f);
  iss_detector.set_gamma_21(0.975f);
  iss_detector.set_gamma_32(0.975f);
  iss_detector.set_min_neighbors(5);
  
  auto src_keypoint_indices = std::make_shared<std::vector<std::size_t>>();
  auto tgt_keypoint_indices = std::make_shared<std::vector<std::size_t>>();
  
  iss_detector.detect(*source_cloud, *src_keypoint_indices);
  iss_detector.detect(*target_cloud, *tgt_keypoint_indices);
  
  LOG_INFO_S << "源关键点数 / Source keypoints: " << src_keypoint_indices->size();
  LOG_INFO_S << "目标关键点数 / Target keypoints: " << tgt_keypoint_indices->size();

  // 4. 计算描述子 / Compute descriptors
  LOG_INFO_S << "计算 FPFH 描述子 / Computing FPFH descriptors...";
  
  // 创建 KD-tree / Create KD-tree
  auto src_kdtree = std::make_shared<kdtree_t<point_t<DataType>>>();
  src_kdtree->set_input(source_cloud);
  
  auto tgt_kdtree = std::make_shared<kdtree_t<point_t<DataType>>>();
  tgt_kdtree->set_input(target_cloud);
  
  // 计算 FPFH / Compute FPFH
  fpfh_extractor_t<DataType, kdtree_t<point_t<DataType>>> fpfh_extractor;
  fpfh_extractor.set_search_radius(0.5f);
  
  auto src_descriptors = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
  auto tgt_descriptors = std::make_shared<std::vector<fpfh_signature_t<DataType>>>();
  
  fpfh_extractor.extract(*source_cloud, src_kdtree, *src_keypoint_indices, *src_descriptors);
  fpfh_extractor.extract(*target_cloud, tgt_kdtree, *tgt_keypoint_indices, *tgt_descriptors);

  // 5. 生成对应关系 / Generate correspondences
  LOG_INFO_S << "生成对应关系 / Generating correspondences...";
  
  auto correspondences = std::make_shared<std::vector<correspondence_t>>();
  
  // 使用 KNN 生成对应关系 / Use KNN for correspondence generation
  auto desc_kdtree = std::make_shared<kdtree_t<fpfh_signature_t<DataType>>>();
  desc_kdtree->set_input(tgt_descriptors);
  
  knn_correspondence_generator_t<DataType, fpfh_signature_t<DataType>, 
                                 kdtree_t<fpfh_signature_t<DataType>>> corr_gen;
  corr_gen.set_knn(desc_kdtree);
  corr_gen.set_source(source_cloud, src_descriptors, src_keypoint_indices);
  corr_gen.set_destination(target_cloud, tgt_descriptors, tgt_keypoint_indices);
  corr_gen.set_ratio(0.8f);
  corr_gen.set_mutual_verification(true);
  
  corr_gen.compute(*correspondences);
  
  LOG_INFO_S << "生成的对应关系数 / Generated correspondences: " << correspondences->size();

  // 6. 使用不同的排序策略 / Use different sorting strategies
  LOG_INFO_S << "\n=== 测试不同的排序策略 / Testing different sorting strategies ===";
  
  // 6.1 描述子距离排序 / Descriptor distance sorting
  LOG_INFO_S << "\n1. 描述子距离排序 / Descriptor distance sorting";
  descriptor_distance_sorter_t<DataType> dist_sorter;
  dist_sorter.set_correspondences(correspondences);
  dist_sorter.set_invert_score(true);
  
  std::vector<DataType> dist_scores;
  auto dist_sorted_indices = dist_sorter.compute_sorted_indices(dist_scores);
  
  // 6.2 几何一致性排序 / Geometric consistency sorting
  LOG_INFO_S << "\n2. 几何一致性排序 / Geometric consistency sorting";
  geometric_consistency_sorter_t<DataType> geom_sorter;
  geom_sorter.set_point_clouds(source_cloud, target_cloud);
  geom_sorter.set_correspondences(correspondences);
  geom_sorter.set_neighborhood_size(10);
  geom_sorter.set_distance_ratio_threshold(0.2f);
  
  std::vector<DataType> geom_scores;
  auto geom_sorted_indices = geom_sorter.compute_sorted_indices(geom_scores);
  
  // 6.3 组合排序 / Combined sorting
  LOG_INFO_S << "\n3. 组合排序 / Combined sorting";
  combined_sorter_t<DataType> combined_sorter;
  
  auto dist_sorter_ptr = std::make_shared<descriptor_distance_sorter_t<DataType>>();
  dist_sorter_ptr->set_invert_score(true);
  combined_sorter.add_sorter(dist_sorter_ptr, 0.6f);
  
  auto geom_sorter_ptr = std::make_shared<geometric_consistency_sorter_t<DataType>>();
  geom_sorter_ptr->set_neighborhood_size(10);
  combined_sorter.add_sorter(geom_sorter_ptr, 0.4f);
  
  combined_sorter.normalize_weights();
  combined_sorter.set_point_clouds(source_cloud, target_cloud);
  combined_sorter.set_correspondences(correspondences);
  
  std::vector<DataType> combined_scores;
  auto combined_sorted_indices = combined_sorter.compute_sorted_indices(combined_scores);

  // 7. 配准测试 / Registration tests
  LOG_INFO_S << "\n=== 配准测试 / Registration Tests ===";
  
  const DataType inlier_threshold = 0.1f;
  const DataType confidence = 0.99f;
  const std::size_t max_iterations = 5000;
  
  // 测试函数 / Test function
  auto test_registration = [&](const std::string& name,
                               prosac_registration_t<DataType>& prosac,
                               const std::vector<std::size_t>& sorted_indices = {}) {
    LOG_INFO_S << "\n--- " << name << " ---";
    
    if (!sorted_indices.empty()) {
      prosac.set_sorted_correspondences(correspondences, sorted_indices);
    } else {
      prosac.set_sorted_correspondences(correspondences);
    }
    
    prosac.set_max_iterations(max_iterations);
    prosac.set_inlier_threshold(inlier_threshold);
    prosac.set_confidence(confidence);
    prosac.set_refine_result(true);
    
    registration_result_t<DataType> result;
    
    auto start = std::chrono::high_resolution_clock::now();
    bool success = prosac.align(result);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (success) {
      LOG_INFO_S << "配准成功 / Registration successful";
      LOG_INFO_S << "迭代次数 / Iterations: " << result.iterations;
      LOG_INFO_S << "内点数 / Inliers: " << result.inliers.size() << "/" << correspondences->size();
      LOG_INFO_S << "适应度分数 / Fitness score: " << result.fitness_score;
      LOG_INFO_S << "运行时间 / Runtime: " << duration.count() << " ms";
      
      if (argc <= 2) {  // 如果使用合成数据，计算误差 / If using synthetic data, compute error
        Eigen::Matrix<DataType, 4, 4> error_matrix = result.transformation - true_transform;
        DataType transform_error = error_matrix.norm();
        LOG_INFO_S << "变换误差 / Transform error: " << transform_error;
      }
      
      std::cout << "估计的变换矩阵 / Estimated transformation:\n" << result.transformation << std::endl;
    } else {
      LOG_WARN_S << "配准失败 / Registration failed";
    }
    
    return result;
  };
  
  // 7.1 PROSAC with descriptor distance sorting
  prosac_registration_t<DataType> prosac1;
  prosac1.set_source(source_cloud);
  prosac1.set_target(target_cloud);
  auto result1 = test_registration("PROSAC (描述子距离排序 / Descriptor distance sorting)", 
                                   prosac1, dist_sorted_indices);
  
  // 7.2 PROSAC with geometric consistency sorting
  prosac_registration_t<DataType> prosac2;
  prosac2.set_source(source_cloud);
  prosac2.set_target(target_cloud);
  auto result2 = test_registration("PROSAC (几何一致性排序 / Geometric consistency sorting)", 
                                   prosac2, geom_sorted_indices);
  
  // 7.3 PROSAC with combined sorting
  prosac_registration_t<DataType> prosac3;
  prosac3.set_source(source_cloud);
  prosac3.set_target(target_cloud);
  auto result3 = test_registration("PROSAC (组合排序 / Combined sorting)", 
                                   prosac3, combined_sorted_indices);
  
  // 7.4 Standard RANSAC for comparison
  LOG_INFO_S << "\n--- RANSAC (对比 / For comparison) ---";
  ransac_registration_t<DataType> ransac;
  ransac.set_source(source_cloud);
  ransac.set_target(target_cloud);
  ransac.set_correspondences(correspondences);
  ransac.set_max_iterations(max_iterations);
  ransac.set_inlier_threshold(inlier_threshold);
  ransac.set_confidence(confidence);
  ransac.set_refine_result(true);
  
  registration_result_t<DataType> ransac_result;
  
  auto ransac_start = std::chrono::high_resolution_clock::now();
  bool ransac_success = ransac.align(ransac_result);
  auto ransac_end = std::chrono::high_resolution_clock::now();
  
  auto ransac_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      ransac_end - ransac_start);
  
  if (ransac_success) {
    LOG_INFO_S << "RANSAC 配准成功 / RANSAC registration successful";
    LOG_INFO_S << "迭代次数 / Iterations: " << ransac_result.iterations;
    LOG_INFO_S << "内点数 / Inliers: " << ransac_result.inliers.size();
    LOG_INFO_S << "运行时间 / Runtime: " << ransac_duration.count() << " ms";
  }

  // 8. 总结 / Summary
  LOG_INFO_S << "\n=== 总结 / Summary ===";
  LOG_INFO_S << "PROSAC 相对于 RANSAC 的加速比 / PROSAC speedup over RANSAC:";
  
  if (result1.converged && ransac_result.converged) {
    DataType speedup1 = static_cast<DataType>(ransac_result.iterations) / result1.iterations;
    LOG_INFO_S << "  描述子距离排序 / Descriptor distance: " << std::fixed 
             << std::setprecision(2) << speedup1 << "x";
  }
  
  if (result2.converged && ransac_result.converged) {
    DataType speedup2 = static_cast<DataType>(ransac_result.iterations) / result2.iterations;
    LOG_INFO_S << "  几何一致性排序 / Geometric consistency: " << std::fixed 
             << std::setprecision(2) << speedup2 << "x";
  }
  
  if (result3.converged && ransac_result.converged) {
    DataType speedup3 = static_cast<DataType>(ransac_result.iterations) / result3.iterations;
    LOG_INFO_S << "  组合排序 / Combined: " << std::fixed 
             << std::setprecision(2) << speedup3 << "x";
  }

  // 9. 保存结果（可选） / Save results (optional)
  if (argc > 3) {
    LOG_INFO_S << "\n保存变换后的源点云 / Saving transformed source cloud...";
    
    // 选择最佳结果 / Choose best result
    auto best_result = result3.converged ? result3 : 
                       (result1.converged ? result1 : result2);
    
    if (best_result.converged) {
      // 变换源点云 / Transform source cloud
      point_cloud_t<DataType> transformed_cloud;
      for (const auto& pt : source_cloud->points) {
        Eigen::Vector4d src_vec(pt.x, pt.y, pt.z, 1.0);
        Eigen::Vector4d tgt_vec = best_result.transformation.template cast<double>() * src_vec;
        
        point_t<DataType> transformed_pt = pt;
        transformed_pt.x = tgt_vec[0];
        transformed_pt.y = tgt_vec[1];
        transformed_pt.z = tgt_vec[2];
        
        transformed_cloud.points.push_back(transformed_pt);
      }
      
      if (write_pcd(argv[3], transformed_cloud, false)) {
        LOG_INFO_S << "已保存到 / Saved to: " << argv[3];
      }
    }
  }

  LOG_INFO_S << "\n示例完成！/ Example completed!";
  
  return 0;
}
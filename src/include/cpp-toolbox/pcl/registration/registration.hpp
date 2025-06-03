#pragma once

/**
 * @file registration.hpp
 * @brief 点云配准算法统一导出文件 / Unified export file for point cloud registration algorithms
 * 
 * 该文件提供了点云配准的统一接口，包括粗配准和精配准算法。
 * This file provides a unified interface for point cloud registration, including both
 * coarse and fine registration algorithms.
 * 
 * 粗配准算法 / Coarse Registration Algorithms:
 * - RANSAC: 基于对应关系的随机采样一致性算法 / Correspondence-based random sample consensus
 * - 4PCS: 4点共面集算法，不需要初始对应关系 / 4-Point Congruent Sets, no initial correspondences needed
 * - Super4PCS: 优化的4PCS，适合大规模点云 / Optimized 4PCS for large-scale point clouds
 * 
 * @code
 * #include <cpp-toolbox/pcl/registration/registration.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // RANSAC配准示例 / RANSAC registration example
 * ransac_registration_t<float> ransac;
 * ransac.set_source(source_cloud);
 * ransac.set_target(target_cloud);
 * ransac.set_correspondences(correspondences);
 * 
 * registration_result_t<float> result;
 * if (ransac.align(result)) {
 *     std::cout << "配准成功，内点数 / Registration successful, inliers: " 
 *               << result.inliers.size() << std::endl;
 * }
 * 
 * // 4PCS配准示例（不需要对应关系） / 4PCS registration example (no correspondences needed)
 * four_pcs_registration_t<float> fourpcs;
 * fourpcs.set_source(source_cloud);
 * fourpcs.set_target(target_cloud);
 * fourpcs.set_delta(0.01f);     // 1cm精度 / 1cm accuracy
 * fourpcs.set_overlap(0.5f);    // 50%重叠 / 50% overlap
 * 
 * if (fourpcs.align(result)) {
 *     // 使用result.transformation变换源点云
 *     // Use result.transformation to transform source cloud
 * }
 * @endcode
 */

// 基础类和结果结构 / Base classes and result structures
#include <cpp-toolbox/pcl/registration/registration_result.hpp>
#include <cpp-toolbox/pcl/registration/base_coarse_registration.hpp>

// 粗配准算法 / Coarse registration algorithms
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/registration/super_four_pcs_registration.hpp>

// 必要的类型定义 / Required type definitions
#include <cpp-toolbox/types/point.hpp>

// 日志宏 / Logger macros
#include <cpp-toolbox/logger/thread_logger.hpp>
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()

// 便捷类型别名 / Convenient type aliases
namespace toolbox::pcl
{

/**
 * @defgroup registration 点云配准 / Point Cloud Registration
 * @{
 */

// 单精度浮点类型别名 / Single precision float aliases
using ransac_registration = ransac_registration_t<float>;
using four_pcs_registration = four_pcs_registration_t<float>;
using super_four_pcs_registration = super_four_pcs_registration_t<float>;
using registration_result = registration_result_t<float>;

// 双精度浮点类型别名 / Double precision float aliases
using ransac_registration_d = ransac_registration_t<double>;
using four_pcs_registration_d = four_pcs_registration_t<double>;
using super_four_pcs_registration_d = super_four_pcs_registration_t<double>;
using registration_result_d = registration_result_t<double>;

/**
 * @brief 配准算法选择指南 / Registration algorithm selection guide
 * 
 * 选择合适的配准算法取决于以下因素：
 * Choosing the right registration algorithm depends on:
 * 
 * 1. **是否有对应关系 / Whether correspondences are available**
 *    - 有对应关系：使用RANSAC / With correspondences: use RANSAC
 *    - 无对应关系：使用4PCS或Super4PCS / Without: use 4PCS or Super4PCS
 * 
 * 2. **点云规模 / Point cloud scale**
 *    - 小规模（<10K点）：任意算法 / Small (<10K points): any algorithm
 *    - 中等规模（10K-100K）：RANSAC或4PCS / Medium (10K-100K): RANSAC or 4PCS
 *    - 大规模（>100K）：Super4PCS / Large (>100K): Super4PCS
 * 
 * 3. **重叠率 / Overlap ratio**
 *    - 高重叠（>70%）：任意算法 / High overlap (>70%): any algorithm
 *    - 中等重叠（30-70%）：4PCS或Super4PCS / Medium (30-70%): 4PCS or Super4PCS
 *    - 低重叠（<30%）：Super4PCS / Low overlap (<30%): Super4PCS
 * 
 * 4. **精度要求 / Accuracy requirements**
 *    - 高精度：使用小的delta值和更多迭代 / High accuracy: use small delta and more iterations
 *    - 快速近似：使用大的delta值和少量迭代 / Fast approximation: use large delta and fewer iterations
 * 
 * 5. **计算资源 / Computational resources**
 *    - 有限资源：使用采样和并行计算 / Limited: use sampling and parallel computation
 *    - 充足资源：使用完整点云和更多基 / Ample: use full clouds and more bases
 */

/**
 * @brief 快速配准便捷函数 / Quick registration convenience function
 * @tparam DataType 数据类型 / Data type
 * @param source 源点云 / Source cloud
 * @param target 目标点云 / Target cloud
 * @param algorithm 算法名称（"ransac", "4pcs", "super4pcs"） / Algorithm name
 * @param overlap 重叠率估计（仅4PCS类算法） / Overlap estimate (4PCS algorithms only)
 * @return 配准结果 / Registration result
 * 
 * @code
 * auto result = quick_registration(source, target, "super4pcs", 0.5f);
 * if (result.converged) {
 *     // 使用result.transformation
 *     // Use result.transformation
 * }
 * @endcode
 */
template<typename DataType>
registration_result_t<DataType> quick_registration(
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& source,
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& target,
    const std::string& algorithm = "super4pcs",
    DataType overlap = 0.5f)
{
  registration_result_t<DataType> result;
  result.transformation.setIdentity();
  result.converged = false;
  
  if (algorithm == "4pcs") {
    four_pcs_registration_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_overlap(overlap);
    reg.set_delta(static_cast<DataType>(0.02));  // 2cm默认精度 / 2cm default accuracy
    reg.align(result);
  } else if (algorithm == "super4pcs") {
    super_four_pcs_registration_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_overlap(overlap);
    reg.set_delta(static_cast<DataType>(0.02));
    reg.enable_smart_indexing(true);
    reg.align(result);
  } else {
    LOG_ERROR_S << "未知算法 / Unknown algorithm: " << algorithm;
  }
  
  return result;
}

/** @} */ // end of registration group

}  // namespace toolbox::pcl
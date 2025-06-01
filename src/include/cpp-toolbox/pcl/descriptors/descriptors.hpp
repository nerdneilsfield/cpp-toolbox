#pragma once

/**
 * @file descriptors.hpp
 * @brief 描述子提取算法统一导出文件 / Unified export file for descriptor extraction algorithms
 * 
 * 该文件提供了所有点云描述子提取算法的统一接口。描述子是用于描述点云局部几何特征的向量，
 * 在点云配准、物体识别、场景理解等任务中起着关键作用。
 * This file provides a unified interface for all point cloud descriptor extraction algorithms.
 * Descriptors are vectors that describe local geometric features of point clouds, playing
 * a crucial role in tasks such as point cloud registration, object recognition, and scene understanding.
 * 
 * @code
 * #include <cpp-toolbox/pcl/descriptors/descriptors.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // 加载点云并检测关键点 / Load point cloud and detect keypoints
 * point_cloud_t<float> cloud = load_point_cloud("object.pcd");
 * auto keypoints = detect_keypoints(cloud);
 * 
 * // 使用FPFH描述子 / Using FPFH descriptor
 * fpfh_extractor_t<float> fpfh;
 * fpfh.set_search_radius(0.05f);
 * 
 * std::vector<fpfh_signature_t<float>> descriptors;
 * fpfh.compute(cloud, keypoints, descriptors);
 * 
 * // 使用描述子进行匹配 / Use descriptors for matching
 * auto matches = match_descriptors(descriptors, target_descriptors);
 * @endcode
 */

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/pfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/shot_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/vfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/3dsc_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/cvfh_extractor.hpp>
#include <cpp-toolbox/pcl/descriptors/rops_extractor.hpp>

namespace toolbox::pcl
{

/**
 * @defgroup descriptors 描述子提取 / Descriptor Extraction
 * @{
 */

/**
 * @brief 描述子类型及其特点 / Descriptor types and their characteristics
 * 
 * 局部描述子 / Local Descriptors:
 * - PFH (Point Feature Histogram): 125维，精确但计算较慢 / 125D, accurate but slow
 * - FPFH (Fast PFH): 33维，PFH的快速近似版本 / 33D, fast approximation of PFH
 * - SHOT (Signature of Histograms of OrienTations): 352维，包含颜色和形状信息 / 352D, includes color and shape
 * - 3DSC (3D Shape Context): 1980维，基于球形网格的形状描述 / 1980D, spherical grid-based shape description
 * - ROPS (Rotational Projection Statistics): 135维，旋转不变性 / 135D, rotation invariant
 * 
 * 全局描述子 / Global Descriptors:
 * - VFH (Viewpoint Feature Histogram): 308维，整个物体的描述 / 308D, describes entire object
 * - CVFH (Clustered VFH): 308维×聚类数，处理遮挡情况 / 308D×clusters, handles occlusion
 * 
 * @code
 * // 选择合适的描述子 / Choose appropriate descriptor
 * if (need_fast_matching) {
 *     // FPFH: 快速且效果良好 / FPFH: fast and effective
 *     fpfh_extractor_t<float> extractor;
 * } else if (need_high_accuracy) {
 *     // SHOT: 更高的区分度 / SHOT: higher discriminative power
 *     shot_extractor_t<float> extractor;
 * } else if (need_global_descriptor) {
 *     // VFH: 全局物体描述 / VFH: global object description
 *     vfh_extractor_t<float> extractor;
 * }
 * @endcode
 */

/**
 * @brief 描述子匹配辅助函数 / Descriptor matching helper function
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @param source_descriptors 源描述子集 / Source descriptor set
 * @param target_descriptors 目标描述子集 / Target descriptor set
 * @param max_distance 最大匹配距离 / Maximum matching distance
 * @return 匹配对的索引 / Indices of matched pairs
 * 
 * @code
 * // 匹配两个点云的描述子 / Match descriptors between two point clouds
 * auto matches = match_descriptors(source_desc, target_desc, 0.25f);
 * 
 * for (const auto& [src_idx, tgt_idx] : matches) {
 *     std::cout << "匹配 / Match: " << src_idx << " -> " << tgt_idx << std::endl;
 * }
 * @endcode
 */
template<typename Signature>
std::vector<std::pair<size_t, size_t>> match_descriptors(
    const std::vector<Signature>& source_descriptors,
    const std::vector<Signature>& target_descriptors,
    typename Signature::data_type max_distance)
{
    std::vector<std::pair<size_t, size_t>> matches;
    
    for (size_t i = 0; i < source_descriptors.size(); ++i) {
        typename Signature::data_type min_dist = std::numeric_limits<typename Signature::data_type>::max();
        size_t best_match = 0;
        
        for (size_t j = 0; j < target_descriptors.size(); ++j) {
            auto dist = source_descriptors[i].distance(target_descriptors[j]);
            if (dist < min_dist) {
                min_dist = dist;
                best_match = j;
            }
        }
        
        if (min_dist < max_distance) {
            matches.emplace_back(i, best_match);
        }
    }
    
    return matches;
}

/**
 * @brief 使用比率测试的描述子匹配 / Descriptor matching with ratio test
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @param source_descriptors 源描述子集 / Source descriptor set
 * @param target_descriptors 目标描述子集 / Target descriptor set
 * @param ratio_threshold 比率阈值（通常0.7-0.8） / Ratio threshold (typically 0.7-0.8)
 * @return 匹配对的索引 / Indices of matched pairs
 * 
 * @code
 * // 使用Lowe's ratio test进行鲁棒匹配 / Robust matching using Lowe's ratio test
 * auto matches = match_descriptors_ratio_test(source_desc, target_desc, 0.8f);
 * @endcode
 */
template<typename Signature>
std::vector<std::pair<size_t, size_t>> match_descriptors_ratio_test(
    const std::vector<Signature>& source_descriptors,
    const std::vector<Signature>& target_descriptors,
    float ratio_threshold = 0.8f)
{
    std::vector<std::pair<size_t, size_t>> matches;
    
    for (size_t i = 0; i < source_descriptors.size(); ++i) {
        typename Signature::data_type best_dist = std::numeric_limits<typename Signature::data_type>::max();
        typename Signature::data_type second_best_dist = std::numeric_limits<typename Signature::data_type>::max();
        size_t best_match = 0;
        
        // 找到最佳和次佳匹配 / Find best and second-best matches
        for (size_t j = 0; j < target_descriptors.size(); ++j) {
            auto dist = source_descriptors[i].distance(target_descriptors[j]);
            if (dist < best_dist) {
                second_best_dist = best_dist;
                best_dist = dist;
                best_match = j;
            } else if (dist < second_best_dist) {
                second_best_dist = dist;
            }
        }
        
        // 应用比率测试 / Apply ratio test
        if (best_dist < ratio_threshold * second_best_dist) {
            matches.emplace_back(i, best_match);
        }
    }
    
    return matches;
}

/**
 * @brief 计算描述子集的统计信息 / Compute statistics of descriptor set
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @param descriptors 描述子集 / Descriptor set
 * @return 平均描述子（用于聚类等） / Average descriptor (for clustering etc.)
 * 
 * @code
 * // 计算描述子集的中心 / Compute centroid of descriptor set
 * auto centroid = compute_descriptor_centroid(descriptors);
 * @endcode
 */
template<typename Signature>
Signature compute_descriptor_centroid(const std::vector<Signature>& descriptors) {
    if (descriptors.empty()) {
        return Signature{};
    }
    
    // 这里需要根据具体的Signature类型实现 / Implementation depends on specific Signature type
    // 示例实现适用于具有histogram成员的签名 / Example implementation for signatures with histogram member
    Signature centroid = descriptors[0];
    
    // 具体实现依赖于描述子类型 / Specific implementation depends on descriptor type
    // 通常需要对所有维度求平均 / Usually need to average all dimensions
    
    return centroid;
}

/**
 * @brief 描述子性能评估辅助函数 / Descriptor performance evaluation helper
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @param descriptors1 第一组描述子 / First descriptor set
 * @param descriptors2 第二组描述子 / Second descriptor set
 * @param ground_truth_matches 真实匹配对 / Ground truth matches
 * @return 准确率和召回率 / Precision and recall
 * 
 * @code
 * // 评估描述子匹配性能 / Evaluate descriptor matching performance
 * auto [precision, recall] = evaluate_descriptor_matching(
 *     desc1, desc2, ground_truth, 0.25f);
 * std::cout << "准确率 / Precision: " << precision << std::endl;
 * std::cout << "召回率 / Recall: " << recall << std::endl;
 * @endcode
 */
template<typename Signature>
std::pair<float, float> evaluate_descriptor_matching(
    const std::vector<Signature>& descriptors1,
    const std::vector<Signature>& descriptors2,
    const std::vector<std::pair<size_t, size_t>>& ground_truth_matches,
    typename Signature::data_type max_distance)
{
    auto predicted_matches = match_descriptors(descriptors1, descriptors2, max_distance);
    
    // 转换为set以便快速查找 / Convert to set for fast lookup
    std::set<std::pair<size_t, size_t>> gt_set(
        ground_truth_matches.begin(), ground_truth_matches.end());
    
    int true_positives = 0;
    for (const auto& match : predicted_matches) {
        if (gt_set.count(match) > 0) {
            true_positives++;
        }
    }
    
    float precision = predicted_matches.empty() ? 0.0f : 
        static_cast<float>(true_positives) / predicted_matches.size();
    float recall = ground_truth_matches.empty() ? 0.0f : 
        static_cast<float>(true_positives) / ground_truth_matches.size();
    
    return {precision, recall};
}

/** @} */ // end of descriptors group

}  // namespace toolbox::pcl
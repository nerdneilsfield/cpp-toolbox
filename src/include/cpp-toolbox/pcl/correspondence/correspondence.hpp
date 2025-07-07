#pragma once

/**
 * @file correspondence.hpp
 * @brief 对应点生成算法统一导出文件 / Unified export file for correspondence generation algorithms
 * 
 * 该文件提供了点云对应关系生成的统一接口。对应关系是点云配准、物体识别等任务的基础，
 * 通过匹配描述子来建立源点云和目标点云之间的点对应关系。
 * This file provides a unified interface for point cloud correspondence generation.
 * Correspondences are fundamental for tasks like point cloud registration and object
 * recognition, establishing point-to-point relationships between source and target
 * clouds through descriptor matching.
 * 
 * @code
 * #include <cpp-toolbox/pcl/correspondence/correspondence.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // 准备数据 / Prepare data
 * auto src_cloud = load_point_cloud("source.pcd");
 * auto dst_cloud = load_point_cloud("target.pcd");
 * 
 * // 检测关键点和计算描述子 / Detect keypoints and compute descriptors
 * auto src_keypoints = detect_keypoints(src_cloud);
 * auto dst_keypoints = detect_keypoints(dst_cloud);
 * 
 * auto src_descriptors = compute_descriptors(src_cloud, src_keypoints);
 * auto dst_descriptors = compute_descriptors(dst_cloud, dst_keypoints);
 * 
 * // 生成对应关系 / Generate correspondences
 * auto correspondences = generate_correspondences(
 *     src_cloud, src_descriptors, src_keypoints,
 *     dst_cloud, dst_descriptors, dst_keypoints);
 * 
 * // 使用对应关系进行配准 / Use correspondences for registration
 * auto transformation = estimate_transformation(correspondences);
 * @endcode
 */

#include <cpp-toolbox/pcl/correspondence/base_correspondence_generator.hpp>
#include <cpp-toolbox/pcl/correspondence/knn_correspondence_generator.hpp>
#include <cpp-toolbox/pcl/correspondence/brute_force_correspondence_generator.hpp>
#include <cpp-toolbox/pcl/correspondence/correspondence_sorter.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <memory>
#include <vector>

// Logger macros
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()

namespace toolbox::pcl
{

/**
 * @defgroup correspondence 对应点生成 / Correspondence Generation
 * @{
 */

/**
 * @brief 对应关系生成策略 / Correspondence generation strategies
 * 
 * 不同的策略适用于不同的场景：
 * Different strategies are suitable for different scenarios:
 * 
 * - 最近邻匹配：简单快速，但可能产生很多错误匹配 / 
 *   Nearest neighbor: Simple and fast, but may produce many false matches
 * - 比率测试：Lowe's ratio test，有效减少错误匹配 / 
 *   Ratio test: Lowe's ratio test, effectively reduces false matches
 * - 双向验证：要求匹配是相互的，进一步提高准确性 / 
 *   Mutual verification: Requires mutual matching, further improves accuracy
 * - 几何一致性：检查匹配的几何关系，最可靠但计算量大 / 
 *   Geometric consistency: Checks geometric relationships, most reliable but computationally intensive
 * 
 * @code
 * // 配置对应点生成器 / Configure correspondence generator
 * correspondence_generator_t<float, fpfh_signature_t<float>, knn_t> gen;
 * 
 * // 宽松匹配（更多对应点，可能有错误） / Loose matching (more correspondences, possibly with errors)
 * gen.set_ratio(0.9f);
 * gen.set_mutual_verification(false);
 * 
 * // 严格匹配（更少但更可靠的对应点） / Strict matching (fewer but more reliable correspondences)
 * gen.set_ratio(0.7f);
 * gen.set_mutual_verification(true);
 * gen.set_distance_threshold(0.2f);
 * @endcode
 */

/**
 * @brief 快速生成对应关系的便捷函数（使用KNN） / Convenience function for quick correspondence generation (using KNN)
 * @tparam DataType 数据类型 / Data type
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @tparam KNN KNN搜索算法类型 / KNN search algorithm type
 * @param src_cloud 源点云 / Source point cloud
 * @param src_descriptors 源描述子 / Source descriptors
 * @param src_keypoints 源关键点索引 / Source keypoint indices
 * @param dst_cloud 目标点云 / Target point cloud
 * @param dst_descriptors 目标描述子 / Target descriptors
 * @param dst_keypoints 目标关键点索引 / Target keypoint indices
 * @param ratio 比率测试阈值（默认0.8） / Ratio test threshold (default 0.8)
 * @param mutual 是否启用双向验证（默认true） / Whether to enable mutual verification (default true)
 * @return 对应关系列表 / List of correspondences
 * 
 * @code
 * // 使用KNN快速生成对应关系 / Quick correspondence generation using KNN
 * auto correspondences = generate_correspondences_knn<float, fpfh_signature_t<float>, kdtree_t<fpfh_signature_t<float>>>(
 *     src_cloud, src_descriptors, src_keypoints,
 *     dst_cloud, dst_descriptors, dst_keypoints,
 *     0.8f, true);
 * @endcode
 */
template<typename DataType, typename Signature, typename KNN>
std::vector<correspondence_t> generate_correspondences_knn(
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& src_cloud,
    const std::shared_ptr<std::vector<Signature>>& src_descriptors,
    const std::shared_ptr<std::vector<std::size_t>>& src_keypoints,
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& dst_cloud,
    const std::shared_ptr<std::vector<Signature>>& dst_descriptors,
    const std::shared_ptr<std::vector<std::size_t>>& dst_keypoints,
    float ratio = 0.8f,
    bool mutual = true)
{
    knn_correspondence_generator_t<DataType, Signature, KNN> gen;
    
    auto knn = std::make_shared<KNN>();
    gen.set_knn(knn);
    gen.set_source(src_cloud, src_descriptors, src_keypoints);
    gen.set_destination(dst_cloud, dst_descriptors, dst_keypoints);
    gen.set_ratio(ratio);
    gen.set_mutual_verification(mutual);
    
    std::vector<correspondence_t> correspondences;
    gen.compute(correspondences);
    
    return correspondences;
}

/**
 * @brief 快速生成对应关系的便捷函数（使用暴力搜索） / Convenience function for quick correspondence generation (using brute force)
 * @tparam DataType 数据类型 / Data type
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @param src_cloud 源点云 / Source point cloud
 * @param src_descriptors 源描述子 / Source descriptors
 * @param src_keypoints 源关键点索引 / Source keypoint indices
 * @param dst_cloud 目标点云 / Target point cloud
 * @param dst_descriptors 目标描述子 / Target descriptors
 * @param dst_keypoints 目标关键点索引 / Target keypoint indices
 * @param ratio 比率测试阈值（默认0.8） / Ratio test threshold (default 0.8)
 * @param mutual 是否启用双向验证（默认true） / Whether to enable mutual verification (default true)
 * @param parallel 是否启用并行计算（默认false） / Whether to enable parallel computation (default false)
 * @return 对应关系列表 / List of correspondences
 * 
 * @code
 * // 使用暴力搜索生成对应关系 / Generate correspondences using brute force
 * auto correspondences = generate_correspondences_brute_force<float, fpfh_signature_t<float>>(
 *     src_cloud, src_descriptors, src_keypoints,
 *     dst_cloud, dst_descriptors, dst_keypoints,
 *     0.8f, true, true);  // 启用并行 / Enable parallel
 * @endcode
 */
template<typename DataType, typename Signature>
std::vector<correspondence_t> generate_correspondences_brute_force(
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& src_cloud,
    const std::shared_ptr<std::vector<Signature>>& src_descriptors,
    const std::shared_ptr<std::vector<std::size_t>>& src_keypoints,
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& dst_cloud,
    const std::shared_ptr<std::vector<Signature>>& dst_descriptors,
    const std::shared_ptr<std::vector<std::size_t>>& dst_keypoints,
    float ratio = 0.8f,
    bool mutual = true,
    bool parallel = false)
{
    brute_force_correspondence_generator_t<DataType, Signature> gen;
    
    gen.enable_parallel(parallel);
    gen.set_source(src_cloud, src_descriptors, src_keypoints);
    gen.set_destination(dst_cloud, dst_descriptors, dst_keypoints);
    gen.set_ratio(ratio);
    gen.set_mutual_verification(mutual);
    
    std::vector<correspondence_t> correspondences;
    gen.compute(correspondences);
    
    return correspondences;
}

/**
 * @brief 过滤对应关系的辅助函数 / Helper function to filter correspondences
 * @param correspondences [in/out] 对应关系列表 / List of correspondences
 * @param max_distance 最大允许距离 / Maximum allowed distance
 * @return 被过滤掉的对应关系数量 / Number of filtered correspondences
 * 
 * @code
 * // 过滤掉距离过大的对应关系 / Filter out correspondences with large distances
 * size_t filtered = filter_correspondences_by_distance(correspondences, 0.5f);
 * std::cout << "过滤掉 / Filtered out " << filtered << " 个对应关系 / correspondences" << std::endl;
 * @endcode
 */
inline size_t filter_correspondences_by_distance(
    std::vector<correspondence_t>& correspondences,
    float max_distance)
{
    size_t original_size = correspondences.size();
    
    correspondences.erase(
        std::remove_if(correspondences.begin(), correspondences.end(),
                      [max_distance](const correspondence_t& corr) {
                          return corr.distance > max_distance;
                      }),
        correspondences.end());
    
    return original_size - correspondences.size();
}

/**
 * @brief 计算对应关系的统计信息 / Compute statistics of correspondences
 * @param correspondences 对应关系列表 / List of correspondences
 * @return 包含均值、标准差、最小值、最大值的元组 / Tuple of mean, std, min, max
 * 
 * @code
 * auto [mean, std, min, max] = compute_correspondence_statistics(correspondences);
 * std::cout << "距离统计 / Distance statistics: "
 *           << "均值 / mean=" << mean 
 *           << ", 标准差 / std=" << std 
 *           << ", 范围 / range=[" << min << ", " << max << "]" << std::endl;
 * @endcode
 */
inline std::tuple<float, float, float, float> compute_correspondence_statistics(
    const std::vector<correspondence_t>& correspondences)
{
    if (correspondences.empty()) {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }
    
    float sum = 0.0f;
    float min_dist = std::numeric_limits<float>::max();
    float max_dist = std::numeric_limits<float>::lowest();
    
    for (const auto& corr : correspondences) {
        sum += corr.distance;
        min_dist = std::min(min_dist, corr.distance);
        max_dist = std::max(max_dist, corr.distance);
    }
    
    float mean = sum / correspondences.size();
    
    float variance = 0.0f;
    for (const auto& corr : correspondences) {
        float diff = corr.distance - mean;
        variance += diff * diff;
    }
    variance /= correspondences.size();
    float std_dev = std::sqrt(variance);
    
    return {mean, std_dev, min_dist, max_dist};
}

/**
 * @brief 可视化对应关系的辅助信息 / Helper to visualize correspondence information
 * @param correspondences 对应关系列表 / List of correspondences
 * @param max_display 最多显示的对应关系数量 / Maximum number of correspondences to display
 * 
 * @code
 * // 打印前10个对应关系 / Print first 10 correspondences
 * print_correspondences(correspondences, 10);
 * @endcode
 */
inline void print_correspondences(
    const std::vector<correspondence_t>& correspondences,
    size_t max_display = 10)
{
    LOG_INFO_S << "对应关系数量 / Number of correspondences: " << correspondences.size();
    
    size_t count = std::min(max_display, correspondences.size());
    for (size_t i = 0; i < count; ++i) {
        const auto& corr = correspondences[i];
        LOG_INFO_S << "  [" << i << "] " << corr.src_idx << " <-> " << corr.dst_idx 
                   << ", 距离 / distance = " << corr.distance;
    }
    
    if (correspondences.size() > max_display) {
        LOG_INFO_S << "  ... 还有 / and " << (correspondences.size() - max_display)
                   << " 个对应关系 / more correspondences";
    }
}

/** @} */ // end of correspondence group

}  // namespace toolbox::pcl
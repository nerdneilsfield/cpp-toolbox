#pragma once

/**
 * @file norm.hpp
 * @brief 法向量提取算法统一导出文件 / Unified export file for normal extraction algorithms
 * 
 * 该文件提供了所有法向量提取算法的统一接口。法向量是点云处理中的基础特征，
 * 用于描述局部表面的方向，是许多高级算法（如特征描述、表面重建等）的基础。
 * This file provides a unified interface for all normal extraction algorithms.
 * Normals are fundamental features in point cloud processing, describing the
 * direction of local surfaces, and serve as the basis for many advanced algorithms
 * (such as feature description, surface reconstruction, etc.).
 * 
 * @code
 * #include <cpp-toolbox/pcl/norm/norm.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // 加载点云 / Load point cloud
 * point_cloud_t<float> cloud = load_point_cloud("data.pcd");
 * 
 * // 使用PCA方法提取法向量 / Extract normals using PCA method
 * pca_norm_extractor_t<float> norm_extractor;
 * norm_extractor.set_input(cloud);
 * norm_extractor.set_num_neighbors(30);
 * 
 * // 提取法向量 / Extract normals
 * auto normals = norm_extractor.extract();
 * 
 * // 可视化或保存法向量 / Visualize or save normals
 * save_normals(normals, "normals.pcd");
 * @endcode
 */

#include <cpp-toolbox/pcl/norm/base_norm.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>

namespace toolbox::pcl
{

/**
 * @defgroup norm 法向量提取 / Normal Extraction
 * @{
 */

/**
 * @brief 法向量提取算法的选择指南 / Guide for choosing normal extraction algorithms
 * 
 * 当前实现：
 * - pca_norm_extractor_t: 基于PCA的法向量提取，最常用和稳定的方法 /
 *   PCA-based normal extraction, the most commonly used and stable method
 * 
 * 参数选择建议 / Parameter selection recommendations:
 * - 近邻数量：通常选择10-50个点，取决于点云密度和噪声水平 /
 *   Number of neighbors: typically 10-50 points, depending on point cloud density and noise level
 * - 小的近邻数量：保留更多细节，但对噪声敏感 /
 *   Small number of neighbors: preserves more details but sensitive to noise
 * - 大的近邻数量：更加平滑，但可能丢失细节 /
 *   Large number of neighbors: smoother but may lose details
 * 
 * @code
 * // 根据点云特性选择参数 / Choose parameters based on point cloud characteristics
 * size_t num_neighbors;
 * if (has_high_noise) {
 *     num_neighbors = 50;  // 噪声较大时使用更多近邻 / Use more neighbors for high noise
 * } else if (need_fine_details) {
 *     num_neighbors = 10;  // 需要细节时使用较少近邻 / Use fewer neighbors for fine details
 * } else {
 *     num_neighbors = 30;  // 一般情况下的默认值 / Default value for general cases
 * }
 * @endcode
 */

/**
 * @brief 创建带有默认KNN的法向量提取器 / Create normal extractor with default KNN
 * @tparam T 数据类型 / Data type
 * @return PCA法向量提取器 / PCA normal extractor
 * 
 * @code
 * auto norm_extractor = create_normal_extractor<float>();
 * norm_extractor->set_input(cloud);
 * norm_extractor->set_num_neighbors(30);
 * auto normals = norm_extractor->extract();
 * @endcode
 */
template<typename T>
inline auto create_normal_extractor() {
    return std::make_unique<pca_norm_extractor_t<T>>();
}

/**
 * @brief 法向量方向一致性处理 / Normal orientation consistency processing
 * 
 * 确保所有法向量指向一致的方向（如朝向视点或朝外）
 * Ensures all normals point in a consistent direction (e.g., towards viewpoint or outward)
 * 
 * @tparam T 数据类型 / Data type
 * @param normals 法向量点云 / Normal point cloud
 * @param viewpoint 视点位置 / Viewpoint position
 * @param cloud 原始点云 / Original point cloud
 * 
 * @code
 * // 使法向量朝向视点 / Make normals point towards viewpoint
 * point_t<float> viewpoint = {0, 0, 0};  // 相机位置 / Camera position
 * orient_normals_towards_viewpoint(normals, viewpoint, cloud);
 * @endcode
 */
template<typename T>
void orient_normals_towards_viewpoint(
    toolbox::types::point_cloud_t<T>& normals,
    const point_t<T>& viewpoint,
    const toolbox::types::point_cloud_t<T>& cloud)
{
    for (size_t i = 0; i < normals.size(); ++i) {
        // 计算从点到视点的向量 / Compute vector from point to viewpoint
        T vx = viewpoint.x - cloud.points[i].x;
        T vy = viewpoint.y - cloud.points[i].y;
        T vz = viewpoint.z - cloud.points[i].z;
        
        // 计算点积 / Compute dot product
        T dot = normals.points[i].x * vx + 
                normals.points[i].y * vy + 
                normals.points[i].z * vz;
        
        // 如果法向量背离视点，则翻转 / If normal points away from viewpoint, flip it
        if (dot < 0) {
            normals.points[i].x = -normals.points[i].x;
            normals.points[i].y = -normals.points[i].y;
            normals.points[i].z = -normals.points[i].z;
        }
    }
}

/**
 * @brief 验证法向量的有效性 / Validate normal validity
 * @tparam T 数据类型 / Data type
 * @param normals 法向量点云 / Normal point cloud
 * @return 有效法向量的比例 / Ratio of valid normals
 * 
 * @code
 * auto validity_ratio = validate_normals(normals);
 * if (validity_ratio < 0.9) {
 *     std::cerr << "警告：只有 / Warning: Only " << validity_ratio * 100 
 *               << "% 的法向量有效 / of normals are valid" << std::endl;
 * }
 * @endcode
 */
template<typename T>
double validate_normals(const toolbox::types::point_cloud_t<T>& normals) {
    size_t valid_count = 0;
    const T epsilon = std::numeric_limits<T>::epsilon();
    
    for (const auto& normal : normals.points) {
        T length = std::sqrt(normal.x * normal.x + 
                            normal.y * normal.y + 
                            normal.z * normal.z);
        
        // 检查是否为单位向量且不包含NaN / Check if unit vector and not NaN
        if (std::abs(length - T(1)) < T(0.01) && 
            !std::isnan(normal.x) && 
            !std::isnan(normal.y) && 
            !std::isnan(normal.z)) {
            valid_count++;
        }
    }
    
    return static_cast<double>(valid_count) / normals.size();
}

/** @} */ // end of norm group

}  // namespace toolbox::pcl
#pragma once

/**
 * @file features.hpp
 * @brief PCL特征提取器集合头文件 / PCL feature extractors collection header file
 * 
 * @details 本文件包含了所有可用的点云特征提取算法，并提供了便利的类型别名 / This file includes all available point cloud feature extraction algorithms and provides convenient type aliases
 */

#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/features/curvature_keypoints.hpp>
#include <cpp-toolbox/pcl/features/iss_keypoints.hpp>
#include <cpp-toolbox/pcl/features/harris3d_keypoints.hpp>
#include <cpp-toolbox/pcl/features/sift3d_keypoints.hpp>
#include <cpp-toolbox/pcl/features/loam_feature_extractor.hpp>
#include <cpp-toolbox/pcl/features/susan_keypoints.hpp>
#include <cpp-toolbox/pcl/features/agast_keypoints.hpp>
#include <cpp-toolbox/pcl/features/mls_keypoints.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>

namespace toolbox::pcl
{

/**
 * @brief 特征提取器类型别名 / Feature extractor type aliases
 * 
 * @details 为每种特征提取器和KNN算法的组合提供便利的类型别名，简化使用 / Provides convenient type aliases for each combination of feature extractor and KNN algorithm
 * 
 * @code
 * // 使用示例 - 选择合适的特征提取器 / Usage example - choosing appropriate feature extractor
 * using namespace toolbox::pcl;
 * 
 * // 1. 曲率关键点提取器（快速，适合一般用途） / Curvature keypoint extractor (fast, general purpose)
 * curvature_keypoint_extractor_kdtree_t<float> curvature_extractor;
 * 
 * // 2. ISS关键点提取器（稳定，适合配准） / ISS keypoint extractor (stable, good for registration)
 * iss_keypoint_extractor_kdtree_t<float> iss_extractor;
 * 
 * // 3. Harris3D关键点提取器（经典，检测角点） / Harris3D keypoint extractor (classic, corner detection)
 * harris3d_keypoint_extractor_kdtree_t<float> harris_extractor;
 * 
 * // 4. SIFT3D关键点提取器（尺度不变，描述能力强） / SIFT3D keypoint extractor (scale-invariant, strong descriptive power)
 * sift3d_keypoint_extractor_kdtree_t<float> sift_extractor;
 * 
 * // 5. LOAM特征提取器（激光雷达SLAM专用） / LOAM feature extractor (specialized for LiDAR SLAM)
 * loam_feature_extractor_kdtree_t<float> loam_extractor;
 * 
 * // 6. SUSAN关键点提取器（噪声鲁棒） / SUSAN keypoint extractor (noise robust)
 * susan_keypoint_extractor_kdtree_t<float> susan_extractor;
 * 
 * // 7. AGAST关键点提取器（快速角点检测） / AGAST keypoint extractor (fast corner detection)
 * agast_keypoint_extractor_kdtree_t<float> agast_extractor;
 * 
 * // 8. MLS关键点提取器（基于曲面拟合） / MLS keypoint extractor (based on surface fitting)
 * mls_keypoint_extractor_kdtree_t<float> mls_extractor;
 * @endcode
 * 
 * @code
 * // 选择不同的KNN算法 / Choosing different KNN algorithms
 * // KDTree - 适合低维数据，查询效率高 / KDTree - suitable for low-dimensional data, efficient queries
 * curvature_keypoint_extractor_kdtree_t<float> kdtree_extractor;
 * 
 * // BruteForce - 适合小数据集，实现简单 / BruteForce - suitable for small datasets, simple implementation
 * curvature_keypoint_extractor_bfknn_t<float> bf_extractor;
 * 
 * // Parallel BruteForce - 适合多核CPU，大数据集 / Parallel BruteForce - suitable for multi-core CPU, large datasets
 * curvature_keypoint_extractor_bfknn_parallel_t<float> parallel_extractor;
 * @endcode
 */

// 曲率关键点提取器类型别名 / Curvature keypoint extractor type aliases
template<typename DataType>
using curvature_keypoint_extractor_kdtree_t = curvature_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using curvature_keypoint_extractor_bfknn_t = curvature_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using curvature_keypoint_extractor_bfknn_parallel_t = curvature_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// ISS关键点提取器类型别名 / ISS keypoint extractor type aliases
template<typename DataType>
using iss_keypoint_extractor_kdtree_t = iss_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using iss_keypoint_extractor_bfknn_t = iss_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using iss_keypoint_extractor_bfknn_parallel_t = iss_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// Harris3D关键点提取器类型别名 / Harris3D keypoint extractor type aliases
template<typename DataType>
using harris3d_keypoint_extractor_kdtree_t = harris3d_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using harris3d_keypoint_extractor_bfknn_t = harris3d_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using harris3d_keypoint_extractor_bfknn_parallel_t = harris3d_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// SIFT3D关键点提取器类型别名 / SIFT3D keypoint extractor type aliases
template<typename DataType>
using sift3d_keypoint_extractor_kdtree_t = sift3d_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using sift3d_keypoint_extractor_bfknn_t = sift3d_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using sift3d_keypoint_extractor_bfknn_parallel_t = sift3d_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// LOAM特征提取器类型别名 / LOAM feature extractor type aliases
template<typename DataType>
using loam_feature_extractor_kdtree_t = loam_feature_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using loam_feature_extractor_bfknn_t = loam_feature_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using loam_feature_extractor_bfknn_parallel_t = loam_feature_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// SUSAN关键点提取器类型别名 / SUSAN keypoint extractor type aliases
template<typename DataType>
using susan_keypoint_extractor_kdtree_t = susan_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using susan_keypoint_extractor_bfknn_t = susan_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using susan_keypoint_extractor_bfknn_parallel_t = susan_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// AGAST关键点提取器类型别名 / AGAST keypoint extractor type aliases
template<typename DataType>
using agast_keypoint_extractor_kdtree_t = agast_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using agast_keypoint_extractor_bfknn_t = agast_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using agast_keypoint_extractor_bfknn_parallel_t = agast_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

// MLS关键点提取器类型别名 / MLS keypoint extractor type aliases
template<typename DataType>
using mls_keypoint_extractor_kdtree_t = mls_keypoint_extractor_t<DataType, kdtree_t<DataType>>;

template<typename DataType>
using mls_keypoint_extractor_bfknn_t = mls_keypoint_extractor_t<DataType, bfknn_t<DataType>>;

template<typename DataType>
using mls_keypoint_extractor_bfknn_parallel_t = mls_keypoint_extractor_t<DataType, bfknn_parallel_t<DataType>>;

}  // namespace toolbox::pcl
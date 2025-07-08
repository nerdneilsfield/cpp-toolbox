#pragma once

#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/io/formats/kitti_extended.hpp>

#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace toolbox::io {

using toolbox::types::point_t;
using toolbox::types::point_cloud_t;

// ==================== KITTI Odometry Data Structures ====================

/**
 * @brief Single frame data from KITTI odometry dataset
 * @tparam DataType The floating point type (float or double)
 */
template<typename DataType>
struct kitti_odometry_frame_t {
    /// Point cloud data
    std::unique_ptr<point_cloud_t<DataType>> cloud;
    
    /// Global pose (4x4 transformation matrix)
    Eigen::Matrix<DataType, 4, 4> pose;
    
    /// Frame index in the sequence
    std::size_t frame_index;
    
    /// Optional timestamp string
    std::string timestamp;
    
    /// Default constructor
    kitti_odometry_frame_t() = default;
    
    /// Move constructor
    kitti_odometry_frame_t(kitti_odometry_frame_t&&) = default;
    
    /// Move assignment
    kitti_odometry_frame_t& operator=(kitti_odometry_frame_t&&) = default;
    
    /// Deleted copy operations
    kitti_odometry_frame_t(const kitti_odometry_frame_t&) = delete;
    kitti_odometry_frame_t& operator=(const kitti_odometry_frame_t&) = delete;
};

/**
 * @brief Frame pair data for registration tasks
 * @tparam DataType The floating point type (float or double)
 */
template<typename DataType>
struct kitti_odometry_frame_pair_t {
    /// Source point cloud
    std::unique_ptr<point_cloud_t<DataType>> source_cloud;
    
    /// Target point cloud
    std::unique_ptr<point_cloud_t<DataType>> target_cloud;
    
    /// Source global pose
    Eigen::Matrix<DataType, 4, 4> source_pose;
    
    /// Target global pose
    Eigen::Matrix<DataType, 4, 4> target_pose;
    
    /// Relative transformation from source to target (T_target_source)
    Eigen::Matrix<DataType, 4, 4> relative_transform;
    
    /// Source frame index
    std::size_t source_index;
    
    /// Target frame index
    std::size_t target_index;
    
    /// Default constructor
    kitti_odometry_frame_pair_t() = default;
    
    /// Move constructor
    kitti_odometry_frame_pair_t(kitti_odometry_frame_pair_t&&) = default;
    
    /// Move assignment
    kitti_odometry_frame_pair_t& operator=(kitti_odometry_frame_pair_t&&) = default;
    
    /// Deleted copy operations
    kitti_odometry_frame_pair_t(const kitti_odometry_frame_pair_t&) = delete;
    kitti_odometry_frame_pair_t& operator=(const kitti_odometry_frame_pair_t&) = delete;
};

// ==================== Semantic KITTI Data Structures ====================

/**
 * @brief Single frame data from Semantic KITTI dataset
 * @tparam DataType The floating point type (float or double)
 */
template<typename DataType>
struct semantic_kitti_frame_t {
    /// Point cloud data
    std::unique_ptr<point_cloud_t<DataType>> cloud;
    
    /// Semantic labels for each point (lower 16 bits: label ID, upper 16 bits: instance ID)
    std::vector<uint32_t> labels;
    
    /// Global pose
    Eigen::Matrix<DataType, 4, 4> pose;
    
    /// Frame index
    std::size_t frame_index;
    
    /**
     * @brief Extract point cloud containing only specified label
     * @param label The semantic label ID (ignoring instance)
     * @return Point cloud with only the specified label
     */
    std::unique_ptr<point_cloud_t<DataType>> get_labeled_cloud(uint16_t label) const;
    
    /**
     * @brief Extract point cloud containing any of the specified labels
     * @param label_ids Vector of label IDs to extract
     * @return Point cloud with points matching any of the labels
     */
    std::unique_ptr<point_cloud_t<DataType>> get_labeled_cloud(
        const std::vector<uint16_t>& label_ids) const;
    
    /**
     * @brief Get unique label IDs in this frame
     * @return Set of unique label IDs (lower 16 bits only)
     */
    std::vector<uint16_t> get_unique_labels() const;
    
    /**
     * @brief Count points for each label
     * @return Map from label ID to point count
     */
    std::map<uint16_t, std::size_t> get_label_statistics() const;
    
    /// Default constructor
    semantic_kitti_frame_t() = default;
    
    /// Move constructor
    semantic_kitti_frame_t(semantic_kitti_frame_t&&) = default;
    
    /// Move assignment
    semantic_kitti_frame_t& operator=(semantic_kitti_frame_t&&) = default;
    
    /// Deleted copy operations
    semantic_kitti_frame_t(const semantic_kitti_frame_t&) = delete;
    semantic_kitti_frame_t& operator=(const semantic_kitti_frame_t&) = delete;
};

/**
 * @brief Frame pair data from Semantic KITTI dataset
 * @tparam DataType The floating point type (float or double)
 */
template<typename DataType>
struct semantic_kitti_frame_pair_t {
    /// Source point cloud
    std::unique_ptr<point_cloud_t<DataType>> source_cloud;
    
    /// Target point cloud
    std::unique_ptr<point_cloud_t<DataType>> target_cloud;
    
    /// Source point labels
    std::vector<uint32_t> source_labels;
    
    /// Target point labels
    std::vector<uint32_t> target_labels;
    
    /// Source global pose
    Eigen::Matrix<DataType, 4, 4> source_pose;
    
    /// Target global pose
    Eigen::Matrix<DataType, 4, 4> target_pose;
    
    /// Relative transformation
    Eigen::Matrix<DataType, 4, 4> relative_transform;
    
    /// Source frame index
    std::size_t source_index;
    
    /// Target frame index
    std::size_t target_index;
    
    /**
     * @brief Extract static points from both clouds (buildings, road, etc.)
     * @return Pair of point clouds containing only static objects
     */
    std::pair<std::unique_ptr<point_cloud_t<DataType>>, 
              std::unique_ptr<point_cloud_t<DataType>>> 
    extract_static_points() const;
    
    /**
     * @brief Extract dynamic points from both clouds (cars, people, etc.)
     * @return Pair of point clouds containing only dynamic objects
     */
    std::pair<std::unique_ptr<point_cloud_t<DataType>>, 
              std::unique_ptr<point_cloud_t<DataType>>> 
    extract_dynamic_points() const;
    
    /// Default constructor
    semantic_kitti_frame_pair_t() = default;
    
    /// Move constructor
    semantic_kitti_frame_pair_t(semantic_kitti_frame_pair_t&&) = default;
    
    /// Move assignment
    semantic_kitti_frame_pair_t& operator=(semantic_kitti_frame_pair_t&&) = default;
    
    /// Deleted copy operations
    semantic_kitti_frame_pair_t(const semantic_kitti_frame_pair_t&) = delete;
    semantic_kitti_frame_pair_t& operator=(const semantic_kitti_frame_pair_t&) = delete;
};


// ==================== Error Recovery Policies ====================

/**
 * @brief Error recovery policy for dataset loading
 */
enum class error_recovery_policy_t : uint8_t {
    strict,    ///< Throw exception on any error
    lenient,   ///< Skip problematic frames and continue
    best_effort ///< Try to recover partial data
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataset/impl/kitti_types_impl.hpp>
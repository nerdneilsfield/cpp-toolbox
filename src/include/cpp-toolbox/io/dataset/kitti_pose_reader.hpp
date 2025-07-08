#pragma once

#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <cpp-toolbox/io/dataset/kitti_exceptions.hpp>
#include <cpp-toolbox/io/formats/kitti_extended.hpp>

#include <Eigen/Dense>
#include <string>
#include <vector>
#include <optional>

namespace toolbox::io {

/**
 * @brief Reader for KITTI pose files
 * @tparam DataType The floating point type (float or double)
 */
template<typename DataType>
class kitti_pose_reader_t {
public:
    /**
     * @brief Default constructor
     */
    kitti_pose_reader_t() = default;
    
    /**
     * @brief Load poses from file
     * @param poses_file Path to poses file (e.g., "00.txt")
     * @return True if successful
     */
    bool load(const std::string& poses_file);
    
    /**
     * @brief Get the number of poses
     * @return Number of loaded poses
     */
    [[nodiscard]] std::size_t size() const noexcept { return poses_.size(); }
    
    /**
     * @brief Check if any poses are loaded
     * @return True if no poses are loaded
     */
    [[nodiscard]] bool empty() const noexcept { return poses_.empty(); }
    
    /**
     * @brief Get pose at specific index
     * @param index Frame index
     * @return 4x4 transformation matrix
     * @throws kitti_index_out_of_range if index is invalid
     */
    [[nodiscard]] Eigen::Matrix<DataType, 4, 4> get_pose(std::size_t index) const;
    
    /**
     * @brief Get pose at specific index (safe version)
     * @param index Frame index
     * @return Optional pose matrix
     */
    [[nodiscard]] std::optional<Eigen::Matrix<DataType, 4, 4>> 
    try_get_pose(std::size_t index) const;
    
    /**
     * @brief Compute relative transformation between two frames
     * @param from_index Source frame index
     * @param to_index Target frame index
     * @return Relative transformation T_to_from
     * @throws kitti_index_out_of_range if indices are invalid
     */
    [[nodiscard]] Eigen::Matrix<DataType, 4, 4> get_relative_transform(
        std::size_t from_index, 
        std::size_t to_index) const;
    
    /**
     * @brief Check if index is valid
     * @param index Frame index to check
     * @return True if index is in range
     */
    [[nodiscard]] bool is_valid_index(std::size_t index) const noexcept {
        return index < poses_.size();
    }
    
    /**
     * @brief Clear all loaded poses
     */
    void clear() noexcept { poses_.clear(); }
    
    /**
     * @brief Get all poses
     * @return Vector of all pose matrices
     */
    [[nodiscard]] const std::vector<Eigen::Matrix<DataType, 4, 4>>& 
    get_all_poses() const noexcept { return poses_; }
    
    /**
     * @brief Compute trajectory length
     * @return Total distance traveled
     */
    [[nodiscard]] DataType compute_trajectory_length() const;
    
    /**
     * @brief Get trajectory bounds
     * @return Min and max points of the trajectory
     */
    [[nodiscard]] std::pair<Eigen::Vector3<DataType>, Eigen::Vector3<DataType>> 
    get_trajectory_bounds() const;

private:
    /// Stored pose matrices
    std::vector<Eigen::Matrix<DataType, 4, 4>> poses_;
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataset/impl/kitti_pose_reader_impl.hpp>
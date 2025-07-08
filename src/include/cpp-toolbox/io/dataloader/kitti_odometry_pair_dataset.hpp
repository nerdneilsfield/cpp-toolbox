#pragma once

#include <cpp-toolbox/io/dataset/dataset.hpp>
#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>

#include <memory>
#include <string>
#include <optional>

namespace toolbox::io {

/**
 * @brief KITTI Odometry frame pair dataset loader
 * @tparam DataType The floating point type (float or double)
 * 
 * This dataset provides access to pairs of frames from KITTI sequences,
 * useful for registration, odometry, and SLAM applications.
 * 
 * @code
 * // Load pairs with skip=1 (consecutive frames)
 * kitti_odometry_pair_dataset_t<float> dataset("/path/to/kitti/sequences/00", 1);
 * 
 * for (const auto& pair : dataset) {
 *     // Use relative transform as initial guess for ICP
 *     icp.set_initial_transform(pair.relative_transform);
 *     auto result = icp.align(pair.source_cloud, pair.target_cloud);
 * }
 * @endcode
 */
template<typename DataType = float>
class kitti_odometry_pair_dataset_t : 
    public dataset_t<kitti_odometry_pair_dataset_t<DataType>, 
                     kitti_odometry_frame_pair_t<DataType>> {
public:
    using pair_type = kitti_odometry_frame_pair_t<DataType>;
    using base_type = dataset_t<kitti_odometry_pair_dataset_t<DataType>, pair_type>;
    
    /**
     * @brief Constructor
     * @param sequence_path Path to sequence directory
     * @param skip Number of frames to skip between source and target (default 1)
     * @throws kitti_invalid_sequence if directory structure is invalid
     */
    explicit kitti_odometry_pair_dataset_t(const std::string& sequence_path, 
                                          std::size_t skip = 1);
    
    /**
     * @brief Set skip parameter
     * @param skip Number of frames between source and target
     */
    void set_skip(std::size_t skip) { 
        skip_ = skip;
        // Clear cache as pairs have changed
        pair_cache_.clear();
    }
    
    /**
     * @brief Get current skip value
     * @return Number of frames between source and target
     */
    [[nodiscard]] std::size_t get_skip() const { return skip_; }
    
    /**
     * @brief Set whether to compute overlap ratio between frames
     * @param enable Enable overlap computation (can be slow)
     */
    void enable_overlap_computation(bool enable) {
        compute_overlap_ = enable;
    }
    
    /**
     * @brief Set maximum pair cache size
     * @param size Maximum number of pairs to cache
     */
    void set_cache_size(std::size_t size) {
        max_cache_size_ = size;
        // Trim cache if needed
        while (pair_cache_.size() > max_cache_size_) {
            pair_cache_.erase(pair_cache_.begin());
        }
    }
    
    // ==================== Dataset Interface ====================
    
    /**
     * @brief Get number of frame pairs
     * @return Number of available pairs
     */
    [[nodiscard]] std::size_t size_impl() const {
        const auto num_frames = base_dataset_.size();
        return (num_frames > skip_) ? (num_frames - skip_) : 0;
    }
    
    /**
     * @brief Load frame pair at specific index
     * @param index Pair index (0-based)
     * @return Frame pair data or nullopt on error
     */
    [[nodiscard]] std::optional<pair_type> at_impl(std::size_t index) const;
    
    // ==================== Additional Functionality ====================
    
    /**
     * @brief Get the underlying base dataset
     * @return Reference to base dataset
     */
    [[nodiscard]] const kitti_odometry_dataset_t<DataType>& 
    get_base_dataset() const { return base_dataset_; }
    
    /**
     * @brief Get source frame index for a pair
     * @param pair_index Index of the pair
     * @return Source frame index in the sequence
     */
    [[nodiscard]] std::size_t get_source_frame_index(std::size_t pair_index) const {
        return pair_index;
    }
    
    /**
     * @brief Get target frame index for a pair
     * @param pair_index Index of the pair
     * @return Target frame index in the sequence
     */
    [[nodiscard]] std::size_t get_target_frame_index(std::size_t pair_index) const {
        return pair_index + skip_;
    }
    
    /**
     * @brief Clear pair cache
     */
    void clear_cache() const {
        pair_cache_.clear();
        base_dataset_.clear_cache();
    }
    
    /**
     * @brief Get pairs with specific relative motion threshold
     * @param min_translation Minimum translation between frames (meters)
     * @param min_rotation Minimum rotation between frames (radians)
     * @return Indices of pairs meeting the criteria
     */
    [[nodiscard]] std::vector<std::size_t> get_pairs_with_motion(
        DataType min_translation = 0.1,
        DataType min_rotation = 0.01) const;

private:
    /**
     * @brief Compute overlap ratio between two point clouds
     * @param source Source cloud
     * @param target Target cloud in source frame
     * @param threshold Distance threshold for overlap
     * @return Ratio of overlapping points
     */
    DataType compute_overlap_ratio(
        const point_cloud_t<DataType>& source,
        const point_cloud_t<DataType>& target,
        DataType threshold = 0.1) const;
    
    // ==================== Member Variables ====================
    
    /// Base dataset for single frames
    mutable kitti_odometry_dataset_t<DataType> base_dataset_;
    
    /// Number of frames to skip
    std::size_t skip_;
    
    /// Cache for loaded pairs
    mutable std::map<std::size_t, pair_type> pair_cache_;
    
    /// Maximum cache size
    std::size_t max_cache_size_ = 50;
    
    /// Whether to compute overlap ratio
    bool compute_overlap_ = false;
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataloader/impl/kitti_odometry_pair_dataset_impl.hpp>
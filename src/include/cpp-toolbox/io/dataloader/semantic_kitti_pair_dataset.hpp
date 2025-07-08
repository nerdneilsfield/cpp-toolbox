#pragma once

#include <cpp-toolbox/io/dataset/dataset.hpp>
#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <cpp-toolbox/io/dataloader/semantic_kitti_dataset.hpp>

#include <memory>
#include <string>
#include <optional>

namespace toolbox::io {

/**
 * @brief Semantic KITTI frame pair dataset loader
 * @tparam DataType The floating point type (float or double)
 * 
 * This dataset provides pairs of frames with semantic labels,
 * useful for semantic SLAM and scene flow estimation.
 * 
 * @code
 * semantic_kitti_pair_dataset_t<float> dataset("/path/to/semantic_kitti/sequences/08", 5);
 * 
 * for (const auto& pair : dataset) {
 *     // Extract static points for registration
 *     auto [src_static, tgt_static] = pair.extract_static_points();
 *     
 *     // Register static points
 *     icp.set_initial_transform(pair.relative_transform);
 *     auto result = icp.align(src_static, tgt_static);
 *     
 *     // Extract dynamic objects for tracking
 *     auto [src_dynamic, tgt_dynamic] = pair.extract_dynamic_points();
 * }
 * @endcode
 */
template<typename DataType = float>
class semantic_kitti_pair_dataset_t : 
    public dataset_t<semantic_kitti_pair_dataset_t<DataType>, 
                     semantic_kitti_frame_pair_t<DataType>> {
public:
    using pair_type = semantic_kitti_frame_pair_t<DataType>;
    using base_type = dataset_t<semantic_kitti_pair_dataset_t<DataType>, pair_type>;
    
    /**
     * @brief Constructor
     * @param sequence_path Path to sequence directory
     * @param skip Number of frames to skip between source and target
     * @throws kitti_invalid_sequence if directory structure is invalid
     */
    explicit semantic_kitti_pair_dataset_t(const std::string& sequence_path, 
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
     * @brief Set maximum pair cache size
     * @param size Maximum number of pairs to cache
     */
    void set_cache_size(std::size_t size) {
        max_cache_size_ = size;
        base_dataset_.set_cache_size(size * 2); // Cache more frames
        // Trim cache if needed
        while (pair_cache_.size() > max_cache_size_) {
            pair_cache_.erase(pair_cache_.begin());
        }
    }
    
    /**
     * @brief Enable/disable motion filtering
     * @param enable Enable filtering of pairs with insufficient motion
     */
    void enable_motion_filter(bool enable) {
        filter_by_motion_ = enable;
    }
    
    /**
     * @brief Set motion thresholds for filtering
     * @param min_translation Minimum translation (meters)
     * @param min_rotation Minimum rotation (radians)
     */
    void set_motion_thresholds(DataType min_translation, DataType min_rotation) {
        min_translation_ = min_translation;
        min_rotation_ = min_rotation;
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
     * @param index Pair index
     * @return Frame pair data with labels or nullopt on error
     */
    [[nodiscard]] std::optional<pair_type> at_impl(std::size_t index) const;
    
    // ==================== Additional Functionality ====================
    
    /**
     * @brief Get the underlying base dataset
     * @return Reference to semantic KITTI dataset
     */
    [[nodiscard]] const semantic_kitti_dataset_t<DataType>& 
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
     * @brief Clear all caches
     */
    void clear_cache() const {
        pair_cache_.clear();
        base_dataset_.clear_cache();
    }
    
    /**
     * @brief Get pairs with specific semantic constraints
     * @param require_labels Labels that must be present in both frames
     * @param min_points Minimum points per label
     * @return Indices of pairs meeting criteria
     */
    [[nodiscard]] std::vector<std::size_t> get_pairs_with_labels(
        const std::vector<uint16_t>& require_labels,
        std::size_t min_points = 100) const;
    
    /**
     * @brief Compute motion statistics for the dataset
     * @return Map with statistics (mean/std translation, rotation, etc.)
     */
    [[nodiscard]] std::map<std::string, DataType> compute_motion_statistics() const;

private:
    /**
     * @brief Check if pair meets motion criteria
     * @param relative_transform Transform between frames
     * @return True if motion is sufficient
     */
    bool meets_motion_criteria(
        const Eigen::Matrix<DataType, 4, 4>& relative_transform) const;
    
    // ==================== Member Variables ====================
    
    /// Base semantic dataset
    mutable semantic_kitti_dataset_t<DataType> base_dataset_;
    
    /// Number of frames to skip
    std::size_t skip_;
    
    /// Cache for loaded pairs
    mutable std::map<std::size_t, pair_type> pair_cache_;
    
    /// Maximum cache size
    std::size_t max_cache_size_ = 30;
    
    /// Motion filtering settings
    bool filter_by_motion_ = false;
    DataType min_translation_ = 0.1;  // meters
    DataType min_rotation_ = 0.01;    // radians
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataloader/impl/semantic_kitti_pair_dataset_impl.hpp>
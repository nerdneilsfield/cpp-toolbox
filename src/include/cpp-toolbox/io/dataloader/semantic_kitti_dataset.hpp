#pragma once

#include <cpp-toolbox/io/dataset/dataset.hpp>
#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>

#include <memory>
#include <string>
#include <optional>
#include <set>
#include <map>

namespace toolbox::io {

/**
 * @brief Semantic KITTI dataset loader
 * @tparam DataType The floating point type (float or double)
 * 
 * This dataset extends KITTI odometry with semantic labels for each point.
 * 
 * @code
 * semantic_kitti_dataset_t<float> dataset("/path/to/semantic_kitti/sequences/08");
 * 
 * // Get label statistics
 * auto stats = dataset.compute_label_statistics();
 * for (const auto& [label, count] : stats) {
 *     std::cout << kitti_semantic_labels::get_label_name(label) 
 *               << ": " << count << " points\n";
 * }
 * 
 * // Process frames
 * for (const auto& frame : dataset) {
 *     // Extract road points
 *     auto road_cloud = frame.get_labeled_cloud(kitti_semantic_labels::ROAD);
 *     
 *     // Extract all static objects
 *     auto static_cloud = frame.get_labeled_cloud({
 *         kitti_semantic_labels::ROAD,
 *         kitti_semantic_labels::BUILDING,
 *         kitti_semantic_labels::VEGETATION
 *     });
 * }
 * @endcode
 */
template<typename DataType = float>
class semantic_kitti_dataset_t : 
    public dataset_t<semantic_kitti_dataset_t<DataType>, 
                     semantic_kitti_frame_t<DataType>> {
public:
    using frame_type = semantic_kitti_frame_t<DataType>;
    using base_type = dataset_t<semantic_kitti_dataset_t<DataType>, frame_type>;
    
    /**
     * @brief Constructor
     * @param sequence_path Path to sequence directory
     * @throws kitti_invalid_sequence if directory structure is invalid
     */
    explicit semantic_kitti_dataset_t(const std::string& sequence_path);
    
    /**
     * @brief Set whether to validate labels match point count
     * @param validate Enable validation
     */
    void enable_label_validation(bool validate) { 
        validate_labels_ = validate; 
    }
    
    /**
     * @brief Set cache size for loaded frames
     * @param max_cached_frames Maximum number of frames to keep in memory
     */
    void set_cache_size(std::size_t max_cached_frames) {
        base_dataset_.set_cache_size(max_cached_frames);
        label_cache_size_ = max_cached_frames;
        // Trim label cache if needed
        while (label_cache_.size() > label_cache_size_) {
            label_cache_.erase(label_cache_.begin());
        }
    }
    
    // ==================== Dataset Interface ====================
    
    /**
     * @brief Get number of frames in the sequence
     * @return Number of available frames
     */
    [[nodiscard]] std::size_t size_impl() const { 
        return base_dataset_.size(); 
    }
    
    /**
     * @brief Load frame at specific index
     * @param index Frame index
     * @return Frame data with labels or nullopt on error
     */
    [[nodiscard]] std::optional<frame_type> at_impl(std::size_t index) const;
    
    // ==================== Semantic-Specific Methods ====================
    
    /**
     * @brief Get all unique labels in the dataset
     * @param scan_all If true, scan all frames (slow). If false, use cached info.
     * @return Set of unique label IDs
     */
    [[nodiscard]] std::set<uint16_t> get_unique_labels(bool scan_all = false) const;
    
    /**
     * @brief Get label name mapping
     * @return Map from label ID to human-readable name
     */
    [[nodiscard]] std::map<uint16_t, std::string> get_label_names() const {
        return kitti_semantic_labels::get_label_map();
    }
    
    /**
     * @brief Compute label statistics across all frames
     * @param max_frames Maximum frames to scan (0 = all)
     * @return Map from label ID to total point count
     */
    [[nodiscard]] std::map<uint16_t, std::size_t> 
    compute_label_statistics(std::size_t max_frames = 0) const;
    
    /**
     * @brief Get frames containing specific label
     * @param label Label ID to search for
     * @param min_points Minimum number of points with this label
     * @return Frame indices containing the label
     */
    [[nodiscard]] std::vector<std::size_t> get_frames_with_label(
        uint16_t label,
        std::size_t min_points = 1) const;
    
    /**
     * @brief Check if labels are available
     * @return True if label files exist
     */
    [[nodiscard]] bool has_labels() const { return has_labels_; }
    
    /**
     * @brief Get label file path for specific frame
     * @param index Frame index
     * @return Path to .label file
     */
    [[nodiscard]] std::string get_label_file(std::size_t index) const {
        if (index >= label_files_.size()) {
            throw kitti_index_out_of_range(index, label_files_.size());
        }
        return label_files_[index];
    }
    
    /**
     * @brief Get the underlying base dataset
     * @return Reference to base KITTI odometry dataset
     */
    [[nodiscard]] const kitti_odometry_dataset_t<DataType>& 
    get_base_dataset() const { return base_dataset_; }
    
    /**
     * @brief Clear all caches
     */
    void clear_cache() const {
        base_dataset_.clear_cache();
        label_cache_.clear();
        unique_labels_cache_.clear();
    }

private:
    /**
     * @brief Scan for label files
     */
    void scan_label_files();
    
    /**
     * @brief Load labels for specific frame
     * @param index Frame index
     * @return Labels or empty vector on error
     */
    std::vector<uint32_t> load_labels(std::size_t index) const;
    
    // ==================== Member Variables ====================
    
    /// Base dataset for point clouds and poses
    mutable kitti_odometry_dataset_t<DataType> base_dataset_;
    
    /// Labels directory path
    std::string labels_path_;
    
    /// List of label files (sorted)
    std::vector<std::string> label_files_;
    
    /// Whether label files are available
    bool has_labels_;
    
    /// Whether to validate label count matches point count
    bool validate_labels_ = true;
    
    /// Cache for loaded labels
    mutable std::map<std::size_t, std::vector<uint32_t>> label_cache_;
    
    /// Label cache size
    std::size_t label_cache_size_ = 100;
    
    /// Cached unique labels
    mutable std::set<uint16_t> unique_labels_cache_;
    mutable bool unique_labels_cached_ = false;
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataloader/impl/semantic_kitti_dataset_impl.hpp>
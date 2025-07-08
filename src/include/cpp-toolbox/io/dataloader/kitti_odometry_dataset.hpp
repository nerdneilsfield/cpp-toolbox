#pragma once

#include <cpp-toolbox/io/dataset/dataset.hpp>
#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <cpp-toolbox/io/dataset/kitti_pose_reader.hpp>
#include <cpp-toolbox/io/dataset/kitti_exceptions.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/io/formats/kitti_extended.hpp>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>

namespace toolbox::io {

/**
 * @brief KITTI Odometry dataset loader
 * @tparam DataType The floating point type (float or double)
 * 
 * This dataset provides access to KITTI odometry sequences, including:
 * - Point cloud data from Velodyne lidar
 * - Global poses for each frame
 * - Optional calibration data
 * 
 * @code
 * kitti_odometry_dataset_t<float> dataset("/path/to/kitti/sequences/00");
 * 
 * // Access frames
 * for (size_t i = 0; i < dataset.size(); ++i) {
 *     auto frame = dataset[i];
 *     if (frame) {
 *         auto& cloud = frame->cloud;
 *         auto& pose = frame->pose;
 *         // Process frame...
 *     }
 * }
 * @endcode
 */
template<typename DataType = float>
class kitti_odometry_dataset_t : 
    public dataset_t<kitti_odometry_dataset_t<DataType>, 
                     kitti_odometry_frame_t<DataType>> {
public:
    using frame_type = kitti_odometry_frame_t<DataType>;
    using base_type = dataset_t<kitti_odometry_dataset_t<DataType>, frame_type>;
    
    /**
     * @brief Constructor
     * @param sequence_path Path to sequence directory (e.g., "/path/to/sequences/00")
     * @throws kitti_invalid_sequence if directory structure is invalid
     */
    explicit kitti_odometry_dataset_t(const std::string& sequence_path);
    
    /**
     * @brief Set cache size for loaded frames
     * @param max_cached_frames Maximum number of frames to keep in memory
     */
    void set_cache_size(std::size_t max_cached_frames) {
        max_cache_size_ = max_cached_frames;
        // If cache is larger than new size, evict oldest entries
        while (cache_.size() > max_cache_size_) {
            auto oldest = lru_list_.back();
            cache_.erase(oldest);
            lru_list_.pop_back();
        }
    }
    
    /**
     * @brief Enable/disable intensity loading
     * @param enable Whether to load intensity values
     */
    void enable_intensity(bool enable) { load_intensity_ = enable; }
    
    /**
     * @brief Set error recovery policy
     * @param policy How to handle errors during loading
     */
    void set_error_policy(error_recovery_policy_t policy) { 
        error_policy_ = policy; 
    }
    
    // ==================== Dataset Interface ====================
    
    /**
     * @brief Get number of frames in the sequence
     * @return Number of available frames
     */
    [[nodiscard]] std::size_t size_impl() const { return cloud_files_.size(); }
    
    /**
     * @brief Load frame at specific index
     * @param index Frame index
     * @return Frame data or nullopt on error
     */
    [[nodiscard]] std::optional<frame_type> at_impl(std::size_t index) const;
    
    // ==================== Additional Functionality ====================
    
    /**
     * @brief Get cloud file path for specific frame
     * @param index Frame index
     * @return Full path to .bin file
     */
    [[nodiscard]] std::string get_cloud_file(std::size_t index) const {
        if (index >= cloud_files_.size()) {
            throw kitti_index_out_of_range(index, cloud_files_.size());
        }
        return cloud_files_[index];
    }
    
    /**
     * @brief Get pose for specific frame
     * @param index Frame index
     * @return 4x4 transformation matrix
     */
    [[nodiscard]] Eigen::Matrix<DataType, 4, 4> get_pose(std::size_t index) const {
        return pose_reader_.get_pose(index);
    }
    
    /**
     * @brief Check if calibration data is available
     * @return True if calibration was loaded
     */
    [[nodiscard]] bool has_calibration() const { 
        return calibration_.has_value(); 
    }
    
    /**
     * @brief Get calibration data
     * @return Calibration data
     * @throws std::runtime_error if no calibration available
     */
    [[nodiscard]] const kitti_calibration_t<DataType>& get_calibration() const {
        if (!calibration_) {
            throw std::runtime_error("No calibration data available");
        }
        return *calibration_;
    }
    
    /**
     * @brief Get sequence name
     * @return Sequence name (e.g., "00", "01")
     */
    [[nodiscard]] const std::string& get_sequence_name() const { 
        return sequence_name_; 
    }
    
    /**
     * @brief Get sequence path
     * @return Full path to sequence directory
     */
    [[nodiscard]] const std::string& get_sequence_path() const { 
        return sequence_path_; 
    }
    
    /**
     * @brief Clear frame cache
     */
    void clear_cache() const {
        cache_.clear();
        lru_list_.clear();
    }

private:
    // ==================== Private Methods ====================
    
    /**
     * @brief Scan directory for point cloud files
     */
    void scan_cloud_files();
    
    /**
     * @brief Load poses from file
     * @param poses_path Path to poses directory
     */
    void load_poses(const std::string& poses_path);
    
    /**
     * @brief Load calibration data
     */
    void load_calibration();
    
    /**
     * @brief Load point cloud from file
     * @param file_path Path to .bin file
     * @return Point cloud or nullptr on error
     */
    std::unique_ptr<point_cloud_t<DataType>> load_cloud(
        const std::string& file_path) const;
    
    /**
     * @brief Update LRU cache
     * @param index Frame index
     * @param frame Frame data to cache
     */
    void update_cache(std::size_t index, frame_type&& frame) const;
    
    /**
     * @brief Get frame from cache if available
     * @param index Frame index
     * @return Cached frame or nullopt
     */
    std::optional<frame_type> get_from_cache(std::size_t index) const;
    
    // ==================== Member Variables ====================
    
    /// Sequence directory path
    std::string sequence_path_;
    
    /// Velodyne data directory path
    std::string velodyne_path_;
    
    /// Sequence name
    std::string sequence_name_;
    
    /// Pose reader
    kitti_pose_reader_t<DataType> pose_reader_;
    
    /// Optional calibration data
    std::optional<kitti_calibration_t<DataType>> calibration_;
    
    /// List of cloud files (sorted)
    std::vector<std::string> cloud_files_;
    
    /// LRU cache for loaded frames
    mutable std::map<std::size_t, frame_type> cache_;
    
    /// LRU list (front = most recent, back = least recent)
    mutable std::list<std::size_t> lru_list_;
    
    /// Maximum cache size
    std::size_t max_cache_size_ = 100;
    
    /// Configuration options
    bool load_intensity_ = true;
    error_recovery_policy_t error_policy_ = error_recovery_policy_t::lenient;
};

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/dataloader/impl/kitti_odometry_dataset_impl.hpp>
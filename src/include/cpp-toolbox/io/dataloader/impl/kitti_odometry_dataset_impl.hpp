#pragma once

#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <filesystem>
#include <algorithm>

namespace toolbox::io {

template<typename DataType>
kitti_odometry_dataset_t<DataType>::kitti_odometry_dataset_t(
    const std::string& sequence_path)
    : sequence_path_(sequence_path) {
    
    namespace fs = std::filesystem;
    
    // Validate directory structure
    if (!validate_kitti_sequence_directory(sequence_path)) {
        throw kitti_invalid_sequence(sequence_path);
    }
    
    // Extract sequence name
    fs::path path(sequence_path);
    sequence_name_ = path.filename().string();
    
    // Set paths
    velodyne_path_ = (path / "velodyne").string();
    
    // Scan for point cloud files
    scan_cloud_files();
    
    if (cloud_files_.empty()) {
        throw kitti_invalid_sequence("No point cloud files found in: " + velodyne_path_);
    }
    
    // Load poses
    // Poses are typically in the parent directory structure
    fs::path poses_dir = path.parent_path().parent_path() / "poses";
    std::string poses_file = (poses_dir / (sequence_name_ + ".txt")).string();
    
    if (fs::exists(poses_file)) {
        load_poses(poses_file);
    } else {
        LOG_WARN_S << "No poses file found at: " << poses_file;
    }
    
    // Load calibration
    load_calibration();
    
    LOG_INFO_S << "Loaded KITTI sequence " << sequence_name_ 
               << " with " << cloud_files_.size() << " frames";
}

template<typename DataType>
void kitti_odometry_dataset_t<DataType>::scan_cloud_files() {
    cloud_files_ = list_kitti_cloud_files(velodyne_path_);
    
    // Verify files are properly numbered
    for (std::size_t i = 0; i < cloud_files_.size(); ++i) {
        int frame_idx = parse_kitti_frame_index(cloud_files_[i]);
        if (frame_idx != static_cast<int>(i)) {
            LOG_WARN_S << "Frame index mismatch: expected " << i 
                      << " but got " << frame_idx 
                      << " for file " << cloud_files_[i];
        }
    }
}

template<typename DataType>
void kitti_odometry_dataset_t<DataType>::load_poses(const std::string& poses_file) {
    if (!pose_reader_.load(poses_file)) {
        LOG_ERROR_S << "Failed to load poses from: " << poses_file;
        return;
    }
    
    // Verify pose count matches frame count
    if (pose_reader_.size() != cloud_files_.size()) {
        LOG_WARN_S << "Pose count (" << pose_reader_.size() 
                  << ") does not match frame count (" << cloud_files_.size() << ")";
    }
}

template<typename DataType>
void kitti_odometry_dataset_t<DataType>::load_calibration() {
    namespace fs = std::filesystem;
    
    fs::path calib_file = fs::path(sequence_path_) / "calib.txt";
    
    if (fs::exists(calib_file)) {
        try {
            calibration_ = read_kitti_calibration<DataType>(calib_file.string());
            LOG_INFO_S << "Loaded calibration from: " << calib_file;
        } catch (const std::exception& e) {
            LOG_WARN_S << "Failed to load calibration: " << e.what();
        }
    }
}

template<typename DataType>
std::optional<kitti_odometry_frame_t<DataType>> 
kitti_odometry_dataset_t<DataType>::at_impl(std::size_t index) const {
    
    if (index >= cloud_files_.size()) {
        if (error_policy_ == error_recovery_policy_t::strict) {
            throw kitti_index_out_of_range(index, cloud_files_.size());
        }
        return std::nullopt;
    }
    
    // Check cache first
    auto cached = get_from_cache(index);
    if (cached) {
        return cached;
    }
    
    // Load point cloud
    auto cloud = load_cloud(cloud_files_[index]);
    if (!cloud) {
        if (error_policy_ == error_recovery_policy_t::strict) {
            throw kitti_corrupted_data("Failed to load cloud at index " + 
                                     std::to_string(index));
        }
        return std::nullopt;
    }
    
    // Create frame
    frame_type frame;
    frame.cloud = std::move(cloud);
    frame.frame_index = index;
    
    // Get pose if available
    if (index < pose_reader_.size()) {
        frame.pose = pose_reader_.get_pose(index);
    } else {
        // Use identity if no pose available
        frame.pose = Eigen::Matrix<DataType, 4, 4>::Identity();
        if (error_policy_ != error_recovery_policy_t::best_effort) {
            LOG_WARN_S << "No pose available for frame " << index;
        }
    }
    
    // Set timestamp (frame index as string for now)
    frame.timestamp = format_kitti_frame_index(index);
    
    // Update cache before returning
    update_cache(index, std::move(frame));
    
    // Return from cache to avoid move issues
    return get_from_cache(index);
}

template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> 
kitti_odometry_dataset_t<DataType>::load_cloud(
    const std::string& file_path) const {
    
    try {
        auto cloud = read_kitti_bin<DataType>(file_path);
        
        if (!cloud) {
            LOG_ERROR_S << "Failed to read point cloud from: " << file_path;
            return nullptr;
        }
        
        // Optionally clear intensity if not needed
        if (!load_intensity_) {
            cloud->intensity = 0;
        }
        
        return cloud;
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Exception loading point cloud from " << file_path 
                   << ": " << e.what();
        return nullptr;
    }
}

template<typename DataType>
void kitti_odometry_dataset_t<DataType>::update_cache(
    std::size_t index, frame_type&& frame) const {
    
    // Remove from LRU list if already present
    auto it = std::find(lru_list_.begin(), lru_list_.end(), index);
    if (it != lru_list_.end()) {
        lru_list_.erase(it);
    }
    
    // Add to front of LRU list
    lru_list_.push_front(index);
    
    // Insert/update in cache
    cache_[index] = std::move(frame);
    
    // Evict oldest if cache is full
    while (cache_.size() > max_cache_size_ && !lru_list_.empty()) {
        auto oldest = lru_list_.back();
        cache_.erase(oldest);
        lru_list_.pop_back();
    }
}

template<typename DataType>
std::optional<kitti_odometry_frame_t<DataType>> 
kitti_odometry_dataset_t<DataType>::get_from_cache(std::size_t index) const {
    
    auto it = cache_.find(index);
    if (it != cache_.end()) {
        // Move to front of LRU list
        auto lru_it = std::find(lru_list_.begin(), lru_list_.end(), index);
        if (lru_it != lru_list_.end()) {
            lru_list_.erase(lru_it);
        }
        lru_list_.push_front(index);
        
        // Return copy to maintain cache
        return it->second;
    }
    
    return std::nullopt;
}

} // namespace toolbox::io
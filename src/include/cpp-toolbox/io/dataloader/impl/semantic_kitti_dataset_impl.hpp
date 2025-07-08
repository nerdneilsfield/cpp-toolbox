#pragma once

#include <cpp-toolbox/io/dataloader/semantic_kitti_dataset.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <filesystem>
#include <algorithm>

namespace toolbox::io {

template<typename DataType>
semantic_kitti_dataset_t<DataType>::semantic_kitti_dataset_t(
    const std::string& sequence_path)
    : base_dataset_(sequence_path) {
    
    namespace fs = std::filesystem;
    
    // Set labels path
    labels_path_ = (fs::path(sequence_path) / "labels").string();
    
    // Check if labels directory exists
    has_labels_ = fs::exists(labels_path_) && fs::is_directory(labels_path_);
    
    if (has_labels_) {
        scan_label_files();
        
        // Verify label count matches frame count
        if (!label_files_.empty() && label_files_.size() != base_dataset_.size()) {
            LOG_WARN_S << "Label file count (" << label_files_.size() 
                      << ") does not match frame count (" << base_dataset_.size() << ")";
        }
    } else {
        LOG_WARN_S << "No labels directory found at: " << labels_path_;
    }
    
    LOG_INFO_S << "Loaded Semantic KITTI sequence " << base_dataset_.get_sequence_name()
               << " with " << base_dataset_.size() << " frames"
               << (has_labels_ ? " (with labels)" : " (no labels)");
}

template<typename DataType>
void semantic_kitti_dataset_t<DataType>::scan_label_files() {
    label_files_ = list_kitti_label_files(labels_path_);
    
    // Verify files are properly numbered
    for (std::size_t i = 0; i < label_files_.size(); ++i) {
        int frame_idx = parse_kitti_frame_index(label_files_[i]);
        if (frame_idx != static_cast<int>(i)) {
            LOG_WARN_S << "Label index mismatch: expected " << i 
                      << " but got " << frame_idx 
                      << " for file " << label_files_[i];
        }
    }
}

template<typename DataType>
std::optional<semantic_kitti_frame_t<DataType>> 
semantic_kitti_dataset_t<DataType>::at_impl(std::size_t index) const {
    
    // Get base frame
    auto base_frame = base_dataset_[index];
    if (!base_frame) {
        return std::nullopt;
    }
    
    // Create semantic frame
    frame_type frame;
    frame.cloud = std::move(base_frame->cloud);
    frame.pose = base_frame->pose;
    frame.frame_index = base_frame->frame_index;
    
    // Load labels if available
    if (has_labels_ && index < label_files_.size()) {
        frame.labels = load_labels(index);
        
        // Validate if enabled
        if (validate_labels_ && frame.cloud && 
            frame.labels.size() != frame.cloud->size()) {
            LOG_ERROR_S << "Label count (" << frame.labels.size() 
                       << ") does not match point count (" << frame.cloud->size() 
                       << ") for frame " << index;
            
            // Resize labels to match (fill with unlabeled)
            frame.labels.resize(frame.cloud->size(), 
                              kitti_semantic_labels::UNLABELED);
        }
    } else {
        // No labels - fill with unlabeled
        if (frame.cloud) {
            frame.labels.resize(frame.cloud->size(), 
                              kitti_semantic_labels::UNLABELED);
        }
    }
    
    return frame;
}

template<typename DataType>
std::vector<uint32_t> semantic_kitti_dataset_t<DataType>::load_labels(
    std::size_t index) const {
    
    // Check cache first
    auto cache_it = label_cache_.find(index);
    if (cache_it != label_cache_.end()) {
        return cache_it->second;
    }
    
    if (index >= label_files_.size()) {
        return {};
    }
    
    try {
        auto labels = read_kitti_labels(label_files_[index]);
        
        // Update cache
        label_cache_[index] = labels;
        
        // Trim cache if needed
        while (label_cache_.size() > label_cache_size_) {
            label_cache_.erase(label_cache_.begin());
        }
        
        return labels;
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Failed to load labels from " << label_files_[index] 
                   << ": " << e.what();
        return {};
    }
}

template<typename DataType>
std::set<uint16_t> semantic_kitti_dataset_t<DataType>::get_unique_labels(
    bool scan_all) const {
    
    if (!scan_all && unique_labels_cached_) {
        return unique_labels_cache_;
    }
    
    std::set<uint16_t> unique_labels;
    
    if (!has_labels_) {
        unique_labels.insert(kitti_semantic_labels::UNLABELED);
        return unique_labels;
    }
    
    // Scan frames to find unique labels
    const std::size_t max_scan = scan_all ? size_impl() : 
                                 std::min(size_impl(), std::size_t(10));
    
    for (std::size_t i = 0; i < max_scan; ++i) {
        auto labels = load_labels(i);
        for (const auto& label : labels) {
            unique_labels.insert(get_kitti_label_id(label));
        }
    }
    
    if (scan_all) {
        unique_labels_cache_ = unique_labels;
        unique_labels_cached_ = true;
    }
    
    return unique_labels;
}

template<typename DataType>
std::map<uint16_t, std::size_t> 
semantic_kitti_dataset_t<DataType>::compute_label_statistics(
    std::size_t max_frames) const {
    
    std::map<uint16_t, std::size_t> stats;
    
    const std::size_t frames_to_scan = (max_frames == 0) ? 
                                       size_impl() : 
                                       std::min(max_frames, size_impl());
    
    for (std::size_t i = 0; i < frames_to_scan; ++i) {
        auto frame = at_impl(i);
        if (!frame) continue;
        
        auto frame_stats = frame->get_label_statistics();
        for (const auto& [label, count] : frame_stats) {
            stats[label] += count;
        }
    }
    
    return stats;
}

template<typename DataType>
std::vector<std::size_t> semantic_kitti_dataset_t<DataType>::get_frames_with_label(
    uint16_t label,
    std::size_t min_points) const {
    
    std::vector<std::size_t> result;
    
    for (std::size_t i = 0; i < size_impl(); ++i) {
        auto labels = load_labels(i);
        
        std::size_t count = 0;
        for (const auto& full_label : labels) {
            if (get_kitti_label_id(full_label) == label) {
                count++;
                if (count >= min_points) {
                    result.push_back(i);
                    break;
                }
            }
        }
    }
    
    return result;
}

} // namespace toolbox::io
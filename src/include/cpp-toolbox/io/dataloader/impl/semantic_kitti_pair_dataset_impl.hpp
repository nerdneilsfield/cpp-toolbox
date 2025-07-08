#pragma once

#include <cpp-toolbox/io/dataloader/semantic_kitti_pair_dataset.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <cmath>

namespace toolbox::io {

template<typename DataType>
semantic_kitti_pair_dataset_t<DataType>::semantic_kitti_pair_dataset_t(
    const std::string& sequence_path, 
    std::size_t skip)
    : base_dataset_(sequence_path), skip_(skip) {
    
    if (skip_ == 0) {
        throw std::invalid_argument("Skip must be at least 1");
    }
    
    LOG_INFO_S << "Created Semantic KITTI pair dataset with skip=" << skip_ 
               << ", " << size_impl() << " pairs available"
               << (base_dataset_.has_labels() ? " (with labels)" : " (no labels)");
}

template<typename DataType>
std::optional<semantic_kitti_frame_pair_t<DataType>> 
semantic_kitti_pair_dataset_t<DataType>::at_impl(std::size_t index) const {
    
    // Check if pair is valid
    if (index >= size_impl()) {
        return std::nullopt;
    }
    
    // Note: Caching disabled for pair datasets due to unique_ptr ownership
    // Each access creates a new pair with fresh point cloud copies
    
    // Compute frame indices
    const std::size_t source_idx = get_source_frame_index(index);
    const std::size_t target_idx = get_target_frame_index(index);
    
    // Load source frame
    auto source_frame = base_dataset_[source_idx];
    if (!source_frame) {
        LOG_ERROR_S << "Failed to load source frame " << source_idx;
        return std::nullopt;
    }
    
    // Load target frame
    auto target_frame = base_dataset_[target_idx];
    if (!target_frame) {
        LOG_ERROR_S << "Failed to load target frame " << target_idx;
        return std::nullopt;
    }
    
    // Create pair
    pair_type pair;
    pair.source_cloud = std::move(source_frame->cloud);
    pair.target_cloud = std::move(target_frame->cloud);
    pair.source_labels = std::move(source_frame->labels);
    pair.target_labels = std::move(target_frame->labels);
    pair.source_pose = source_frame->pose;
    pair.target_pose = target_frame->pose;
    pair.source_index = source_idx;
    pair.target_index = target_idx;
    
    // Compute relative transform
    pair.relative_transform = compute_relative_transform(
        pair.source_pose, pair.target_pose);
    
    // Check motion criteria if filtering is enabled
    if (filter_by_motion_ && !meets_motion_criteria(pair.relative_transform)) {
        LOG_DEBUG_S << "Pair " << index << " filtered out due to insufficient motion";
        return std::nullopt;
    }
    
    return pair;
}

template<typename DataType>
bool semantic_kitti_pair_dataset_t<DataType>::meets_motion_criteria(
    const Eigen::Matrix<DataType, 4, 4>& relative_transform) const {
    
    // Extract translation
    Eigen::Vector3<DataType> translation = 
        relative_transform.template block<3, 1>(0, 3);
    DataType trans_norm = translation.norm();
    
    // Extract rotation angle
    Eigen::Matrix3<DataType> R = 
        relative_transform.template block<3, 3>(0, 0);
    DataType trace = R.trace();
    DataType angle = std::acos(std::min(std::max((trace - 1) / 2, 
                                                 DataType(-1)), 
                                       DataType(1)));
    
    return trans_norm >= min_translation_ || angle >= min_rotation_;
}

template<typename DataType>
std::vector<std::size_t> 
semantic_kitti_pair_dataset_t<DataType>::get_pairs_with_labels(
    const std::vector<uint16_t>& require_labels,
    std::size_t min_points) const {
    
    std::vector<std::size_t> result;
    
    // Convert to set for faster lookup
    std::set<uint16_t> label_set(require_labels.begin(), require_labels.end());
    
    for (std::size_t i = 0; i < size_impl(); ++i) {
        auto pair = at_impl(i);
        if (!pair) continue;
        
        // Check if both frames have required labels
        bool source_has_labels = true;
        bool target_has_labels = true;
        
        for (const auto& label : require_labels) {
            // Count points with this label in source
            std::size_t source_count = 0;
            for (const auto& l : pair->source_labels) {
                if (get_kitti_label_id(l) == label) {
                    source_count++;
                }
            }
            
            // Count points with this label in target
            std::size_t target_count = 0;
            for (const auto& l : pair->target_labels) {
                if (get_kitti_label_id(l) == label) {
                    target_count++;
                }
            }
            
            if (source_count < min_points) source_has_labels = false;
            if (target_count < min_points) target_has_labels = false;
            
            if (!source_has_labels || !target_has_labels) break;
        }
        
        if (source_has_labels && target_has_labels) {
            result.push_back(i);
        }
    }
    
    return result;
}

template<typename DataType>
std::map<std::string, DataType> 
semantic_kitti_pair_dataset_t<DataType>::compute_motion_statistics() const {
    
    std::map<std::string, DataType> stats;
    std::vector<DataType> translations;
    std::vector<DataType> rotations;
    
    for (std::size_t i = 0; i < size_impl(); ++i) {
        auto pair = at_impl(i);
        if (!pair) continue;
        
        // Extract translation
        Eigen::Vector3<DataType> translation = 
            pair->relative_transform.template block<3, 1>(0, 3);
        translations.push_back(translation.norm());
        
        // Extract rotation angle
        Eigen::Matrix3<DataType> R = 
            pair->relative_transform.template block<3, 3>(0, 0);
        DataType trace = R.trace();
        DataType angle = std::acos(std::min(std::max((trace - 1) / 2, 
                                                     DataType(-1)), 
                                           DataType(1)));
        rotations.push_back(angle);
    }
    
    if (!translations.empty()) {
        // Compute translation statistics
        DataType trans_sum = 0;
        DataType trans_sq_sum = 0;
        DataType trans_min = translations[0];
        DataType trans_max = translations[0];
        
        for (const auto& t : translations) {
            trans_sum += t;
            trans_sq_sum += t * t;
            trans_min = std::min(trans_min, t);
            trans_max = std::max(trans_max, t);
        }
        
        DataType trans_mean = trans_sum / translations.size();
        DataType trans_var = (trans_sq_sum / translations.size()) - (trans_mean * trans_mean);
        DataType trans_std = std::sqrt(std::max(DataType(0), trans_var));
        
        stats["translation_mean"] = trans_mean;
        stats["translation_std"] = trans_std;
        stats["translation_min"] = trans_min;
        stats["translation_max"] = trans_max;
        
        // Compute rotation statistics
        DataType rot_sum = 0;
        DataType rot_sq_sum = 0;
        DataType rot_min = rotations[0];
        DataType rot_max = rotations[0];
        
        for (const auto& r : rotations) {
            rot_sum += r;
            rot_sq_sum += r * r;
            rot_min = std::min(rot_min, r);
            rot_max = std::max(rot_max, r);
        }
        
        DataType rot_mean = rot_sum / rotations.size();
        DataType rot_var = (rot_sq_sum / rotations.size()) - (rot_mean * rot_mean);
        DataType rot_std = std::sqrt(std::max(DataType(0), rot_var));
        
        stats["rotation_mean_rad"] = rot_mean;
        stats["rotation_std_rad"] = rot_std;
        stats["rotation_min_rad"] = rot_min;
        stats["rotation_max_rad"] = rot_max;
        stats["rotation_mean_deg"] = rot_mean * 180.0 / M_PI;
        stats["rotation_std_deg"] = rot_std * 180.0 / M_PI;
    }
    
    stats["num_pairs"] = static_cast<DataType>(translations.size());
    stats["skip"] = static_cast<DataType>(skip_);
    
    return stats;
}

} // namespace toolbox::io
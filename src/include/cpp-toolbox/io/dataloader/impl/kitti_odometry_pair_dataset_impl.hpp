#pragma once

#include <cpp-toolbox/io/dataloader/kitti_odometry_pair_dataset.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <cmath>

namespace toolbox::io {

template<typename DataType>
kitti_odometry_pair_dataset_t<DataType>::kitti_odometry_pair_dataset_t(
    const std::string& sequence_path, 
    std::size_t skip)
    : base_dataset_(sequence_path), skip_(skip) {
    
    if (skip_ == 0) {
        throw std::invalid_argument("Skip must be at least 1");
    }
    
    LOG_INFO_S << "Created KITTI pair dataset with skip=" << skip_ 
               << ", " << size_impl() << " pairs available";
}

template<typename DataType>
std::optional<kitti_odometry_frame_pair_t<DataType>> 
kitti_odometry_pair_dataset_t<DataType>::at_impl(std::size_t index) const {
    
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
    pair.source_pose = source_frame->pose;
    pair.target_pose = target_frame->pose;
    pair.source_index = source_idx;
    pair.target_index = target_idx;
    
    // Compute relative transform
    pair.relative_transform = compute_relative_transform(
        pair.source_pose, pair.target_pose);
    
    // Optionally compute overlap
    if (compute_overlap_ && pair.source_cloud && pair.target_cloud) {
        // Transform target to source frame for overlap computation
        Eigen::Matrix<DataType, 4, 4> inverse_transform = pair.relative_transform.inverse();
        auto target_in_source = transform_point_cloud(
            *pair.target_cloud, inverse_transform);
        
        auto overlap = compute_overlap_ratio(
            *pair.source_cloud, *target_in_source);
        
        LOG_DEBUG_S << "Pair " << index << " overlap ratio: " << overlap;
    }
    
    return pair;
}

template<typename DataType>
std::vector<std::size_t> 
kitti_odometry_pair_dataset_t<DataType>::get_pairs_with_motion(
    DataType min_translation,
    DataType min_rotation) const {
    
    std::vector<std::size_t> result;
    
    for (std::size_t i = 0; i < size_impl(); ++i) {
        auto pair = at_impl(i);
        if (!pair) continue;
        
        // Extract translation
        Eigen::Vector3<DataType> translation = 
            pair->relative_transform.template block<3, 1>(0, 3);
        DataType trans_norm = translation.norm();
        
        // Extract rotation angle (from rotation matrix)
        Eigen::Matrix3<DataType> R = 
            pair->relative_transform.template block<3, 3>(0, 0);
        DataType trace = R.trace();
        DataType angle = std::acos(std::min(std::max((trace - 1) / 2, 
                                                     DataType(-1)), 
                                           DataType(1)));
        
        if (trans_norm >= min_translation || angle >= min_rotation) {
            result.push_back(i);
        }
    }
    
    return result;
}

template<typename DataType>
DataType kitti_odometry_pair_dataset_t<DataType>::compute_overlap_ratio(
    const point_cloud_t<DataType>& source,
    const point_cloud_t<DataType>& target,
    DataType threshold) const {
    
    if (source.empty() || target.empty()) {
        return DataType(0);
    }
    
    // Simple brute-force overlap computation
    // In practice, you'd want to use a KD-tree for efficiency
    std::size_t overlap_count = 0;
    
    for (const auto& src_pt : source.points) {
        bool has_close_point = false;
        
        for (const auto& tgt_pt : target.points) {
            DataType dist = src_pt.distance(tgt_pt);
            if (dist < threshold) {
                has_close_point = true;
                break;
            }
        }
        
        if (has_close_point) {
            overlap_count++;
        }
    }
    
    return static_cast<DataType>(overlap_count) / 
           static_cast<DataType>(source.size());
}

} // namespace toolbox::io
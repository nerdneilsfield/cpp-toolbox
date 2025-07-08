#pragma once

#include <cpp-toolbox/io/dataset/kitti_pose_reader.hpp>

namespace toolbox::io {

template<typename DataType>
bool kitti_pose_reader_t<DataType>::load(const std::string& poses_file) {
    try {
        poses_ = read_kitti_poses<DataType>(poses_file);
        return !poses_.empty();
    } catch (const std::exception&) {
        poses_.clear();
        return false;
    }
}

template<typename DataType>
Eigen::Matrix<DataType, 4, 4> kitti_pose_reader_t<DataType>::get_pose(
    std::size_t index) const {
    
    if (index >= poses_.size()) {
        throw kitti_index_out_of_range(index, poses_.size());
    }
    
    return poses_[index];
}

template<typename DataType>
std::optional<Eigen::Matrix<DataType, 4, 4>> 
kitti_pose_reader_t<DataType>::try_get_pose(std::size_t index) const {
    
    if (index >= poses_.size()) {
        return std::nullopt;
    }
    
    return poses_[index];
}

template<typename DataType>
Eigen::Matrix<DataType, 4, 4> kitti_pose_reader_t<DataType>::get_relative_transform(
    std::size_t from_index, 
    std::size_t to_index) const {
    
    if (from_index >= poses_.size()) {
        throw kitti_index_out_of_range(from_index, poses_.size());
    }
    
    if (to_index >= poses_.size()) {
        throw kitti_index_out_of_range(to_index, poses_.size());
    }
    
    return compute_relative_transform(
        poses_[from_index], poses_[to_index]);
}

template<typename DataType>
DataType kitti_pose_reader_t<DataType>::compute_trajectory_length() const {
    if (poses_.size() < 2) {
        return DataType(0);
    }
    
    DataType total_length = 0;
    
    for (std::size_t i = 1; i < poses_.size(); ++i) {
        // Extract translations
        Eigen::Vector3<DataType> pos_prev = poses_[i-1].template block<3, 1>(0, 3);
        Eigen::Vector3<DataType> pos_curr = poses_[i].template block<3, 1>(0, 3);
        
        // Compute distance
        total_length += (pos_curr - pos_prev).norm();
    }
    
    return total_length;
}

template<typename DataType>
std::pair<Eigen::Vector3<DataType>, Eigen::Vector3<DataType>> 
kitti_pose_reader_t<DataType>::get_trajectory_bounds() const {
    
    if (poses_.empty()) {
        return std::make_pair(
            Eigen::Vector3<DataType>::Zero(),
            Eigen::Vector3<DataType>::Zero()
        );
    }
    
    // Initialize with first position
    Eigen::Vector3<DataType> min_point = poses_[0].template block<3, 1>(0, 3);
    Eigen::Vector3<DataType> max_point = min_point;
    
    // Find bounds
    for (const auto& pose : poses_) {
        Eigen::Vector3<DataType> position = pose.template block<3, 1>(0, 3);
        
        min_point.x() = std::min(min_point.x(), position.x());
        min_point.y() = std::min(min_point.y(), position.y());
        min_point.z() = std::min(min_point.z(), position.z());
        
        max_point.x() = std::max(max_point.x(), position.x());
        max_point.y() = std::max(max_point.y(), position.y());
        max_point.z() = std::max(max_point.z(), position.z());
    }
    
    return std::make_pair(min_point, max_point);
}

} // namespace toolbox::io
#pragma once

#include <cpp-toolbox/io/dataset/kitti_types.hpp>
#include <algorithm>
#include <set>

namespace toolbox::io {

// ==================== semantic_kitti_frame_t Implementation ====================

template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> 
semantic_kitti_frame_t<DataType>::get_labeled_cloud(uint16_t label) const {
    auto result = std::make_unique<point_cloud_t<DataType>>();
    
    if (!cloud || labels.size() != cloud->size()) {
        return result;
    }
    
    result->points.reserve(cloud->size() / 10); // Rough estimate
    
    for (std::size_t i = 0; i < cloud->size(); ++i) {
        if (get_kitti_label_id(labels[i]) == label) {
            result->points.push_back(cloud->points[i]);
        }
    }
    
    result->points.shrink_to_fit();
    return result;
}

template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> 
semantic_kitti_frame_t<DataType>::get_labeled_cloud(
    const std::vector<uint16_t>& label_ids) const {
    
    auto result = std::make_unique<point_cloud_t<DataType>>();
    
    if (!cloud || labels.size() != cloud->size()) {
        return result;
    }
    
    // Convert to set for faster lookup
    std::set<uint16_t> label_set(label_ids.begin(), label_ids.end());
    
    result->points.reserve(cloud->size() / 2); // Rough estimate
    
    for (std::size_t i = 0; i < cloud->size(); ++i) {
        if (label_set.count(get_kitti_label_id(labels[i])) > 0) {
            result->points.push_back(cloud->points[i]);
        }
    }
    
    result->points.shrink_to_fit();
    return result;
}

template<typename DataType>
std::vector<uint16_t> semantic_kitti_frame_t<DataType>::get_unique_labels() const {
    std::set<uint16_t> unique_labels;
    
    for (const auto& label : labels) {
        unique_labels.insert(get_kitti_label_id(label));
    }
    
    return std::vector<uint16_t>(unique_labels.begin(), unique_labels.end());
}

template<typename DataType>
std::map<uint16_t, std::size_t> 
semantic_kitti_frame_t<DataType>::get_label_statistics() const {
    std::map<uint16_t, std::size_t> stats;
    
    for (const auto& label : labels) {
        stats[get_kitti_label_id(label)]++;
    }
    
    return stats;
}

// ==================== semantic_kitti_frame_pair_t Implementation ====================

template<typename DataType>
std::pair<std::unique_ptr<point_cloud_t<DataType>>, 
          std::unique_ptr<point_cloud_t<DataType>>> 
semantic_kitti_frame_pair_t<DataType>::extract_static_points() const {
    
    auto source_static = std::make_unique<point_cloud_t<DataType>>();
    auto target_static = std::make_unique<point_cloud_t<DataType>>();
    
    // Extract static points from source
    if (source_cloud && source_labels.size() == source_cloud->size()) {
        for (std::size_t i = 0; i < source_cloud->size(); ++i) {
            uint16_t label_id = get_kitti_label_id(source_labels[i]);
            if (kitti_semantic_labels::is_static(label_id)) {
                source_static->points.push_back(source_cloud->points[i]);
            }
        }
    }
    
    // Extract static points from target
    if (target_cloud && target_labels.size() == target_cloud->size()) {
        for (std::size_t i = 0; i < target_cloud->size(); ++i) {
            uint16_t label_id = get_kitti_label_id(target_labels[i]);
            if (kitti_semantic_labels::is_static(label_id)) {
                target_static->points.push_back(target_cloud->points[i]);
            }
        }
    }
    
    return std::make_pair(std::move(source_static), std::move(target_static));
}

template<typename DataType>
std::pair<std::unique_ptr<point_cloud_t<DataType>>, 
          std::unique_ptr<point_cloud_t<DataType>>> 
semantic_kitti_frame_pair_t<DataType>::extract_dynamic_points() const {
    
    auto source_dynamic = std::make_unique<point_cloud_t<DataType>>();
    auto target_dynamic = std::make_unique<point_cloud_t<DataType>>();
    
    // Extract dynamic points from source
    if (source_cloud && source_labels.size() == source_cloud->size()) {
        for (std::size_t i = 0; i < source_cloud->size(); ++i) {
            uint16_t label_id = get_kitti_label_id(source_labels[i]);
            if (kitti_semantic_labels::is_dynamic(label_id)) {
                source_dynamic->points.push_back(source_cloud->points[i]);
            }
        }
    }
    
    // Extract dynamic points from target
    if (target_cloud && target_labels.size() == target_cloud->size()) {
        for (std::size_t i = 0; i < target_cloud->size(); ++i) {
            uint16_t label_id = get_kitti_label_id(target_labels[i]);
            if (kitti_semantic_labels::is_dynamic(label_id)) {
                target_dynamic->points.push_back(target_cloud->points[i]);
            }
        }
    }
    
    return std::make_pair(std::move(source_dynamic), std::move(target_dynamic));
}

} // namespace toolbox::io
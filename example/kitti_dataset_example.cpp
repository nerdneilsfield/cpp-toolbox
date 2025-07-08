#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_pair_dataset.hpp>
#include <cpp-toolbox/io/dataloader/semantic_kitti_dataset.hpp>
#include <cpp-toolbox/io/dataloader/semantic_kitti_pair_dataset.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/utils/print.hpp>

#include <iostream>
#include <iomanip>

using namespace toolbox::io;
using namespace toolbox::logger;

// Example 1: Basic KITTI odometry dataset usage
void example_basic_kitti_dataset() {
    LOG_INFO_S << "\n=== Example 1: Basic KITTI Odometry Dataset ===";
    
    try {
        // Load KITTI sequence 00
        kitti_odometry_dataset_t<float> dataset("/path/to/kitti/sequences/00");
        
        LOG_INFO_S << "Loaded sequence " << dataset.get_sequence_name() 
                        << " with " << dataset.size() << " frames";
        
        // Access individual frames
        auto frame = dataset[0];
        if (frame) {
            LOG_INFO_S << "Frame 0:";
            LOG_INFO_S << "  - Points: " << frame->cloud->size();
            LOG_INFO_S << "  - Pose:\n" << frame->pose;
        }
        
        // Process first 10 frames
        for (std::size_t i = 0; i < std::min(dataset.size(), std::size_t(10)); ++i) {
            auto frame = dataset[i];
            if (!frame) continue;
            
            // Compute point cloud statistics
            Eigen::Vector3f min_pt(frame->cloud->points[0].x, 
                                   frame->cloud->points[0].y, 
                                   frame->cloud->points[0].z);
            Eigen::Vector3f max_pt = min_pt;
            
            for (const auto& point : frame->cloud->points) {
                Eigen::Vector3f pt(point.x, point.y, point.z);
                min_pt = min_pt.cwiseMin(pt);
                max_pt = max_pt.cwiseMax(pt);
            }
            
            LOG_DEBUG_S << "Frame " << i << " bounding box: "
                             << "[" << min_pt.transpose() << "] to ["
                             << max_pt.transpose() << "]";
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Error loading dataset: " << e.what();
    }
}

// Example 2: KITTI pair dataset for registration
void example_kitti_pair_dataset() {
    LOG_INFO_S << "\n=== Example 2: KITTI Pair Dataset for Registration ===";
    
    try {
        // Create pair dataset with skip=5 (pairs 5 frames apart)
        kitti_odometry_pair_dataset_t<float> dataset("/path/to/kitti/sequences/00", 5);
        
        LOG_INFO_S << "Created pair dataset with " << dataset.size() << " pairs";
        
        // Get pairs with significant motion
        auto moving_pairs = dataset.get_pairs_with_motion(1.0f, 0.05f); // 1m or 0.05 rad
        LOG_INFO_S << "Found " << moving_pairs.size() 
                        << " pairs with significant motion";
        
        // Process first pair
        if (dataset.size() > 0) {
            auto pair = dataset[0];
            if (pair) {
                LOG_INFO_S << "First pair:";
                LOG_INFO_S << "  - Source frame: " << pair->source_index;
                LOG_INFO_S << "  - Target frame: " << pair->target_index;
                LOG_INFO_S << "  - Source points: " << pair->source_cloud->size();
                LOG_INFO_S << "  - Target points: " << pair->target_cloud->size();
                
                // Extract translation and rotation
                Eigen::Vector3f translation = 
                    pair->relative_transform.block<3, 1>(0, 3);
                Eigen::Matrix3f rotation = 
                    pair->relative_transform.block<3, 3>(0, 0);
                
                float angle = std::acos((rotation.trace() - 1) / 2);
                LOG_INFO_S << "  - Translation: " << translation.norm() << " m";
                LOG_INFO_S << "  - Rotation: " << angle * 180 / M_PI << " deg";
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Error creating pair dataset: " << e.what();
    }
}

// Example 3: Semantic KITTI dataset
void example_semantic_kitti_dataset() {
    LOG_INFO_S << "\n=== Example 3: Semantic KITTI Dataset ===";
    
    try {
        // Load Semantic KITTI sequence
        semantic_kitti_dataset_t<float> dataset("/path/to/semantic_kitti/sequences/08");
        
        LOG_INFO_S << "Loaded Semantic KITTI sequence with " 
                        << dataset.size() << " frames";
        LOG_INFO_S << "Has labels: " << (dataset.has_labels() ? "Yes" : "No");
        
        if (dataset.has_labels()) {
            // Get label statistics for first 100 frames
            auto stats = dataset.compute_label_statistics(100);
            
            LOG_INFO_S << "\nLabel statistics (first 100 frames):";
            auto label_names = dataset.get_label_names();
            
            for (const auto& [label_id, count] : stats) {
                auto name_it = label_names.find(label_id);
                std::string label_name = (name_it != label_names.end()) ? 
                                        name_it->second : "Unknown";
                
                LOG_INFO_S << "  - " << std::setw(20) << std::left 
                                << label_name << ": " << count << " points";
            }
            
            // Find frames containing cars
            auto car_frames = dataset.get_frames_with_label(
                kitti_semantic_labels::CAR, 100); // At least 100 car points
            
            LOG_INFO_S << "\nFound " << car_frames.size() 
                            << " frames with at least 100 car points";
            
            // Process a frame with labels
            if (dataset.size() > 0) {
                auto frame = dataset[0];
                if (frame) {
                    // Extract road points only
                    auto road_cloud = frame->get_labeled_cloud(
                        kitti_semantic_labels::ROAD);
                    
                    LOG_INFO_S << "\nFrame 0:";
                    LOG_INFO_S << "  - Total points: " << frame->cloud->size();
                    LOG_INFO_S << "  - Road points: " << road_cloud->size();
                    
                    // Extract all static infrastructure
                    auto static_cloud = frame->get_labeled_cloud({
                        kitti_semantic_labels::ROAD,
                        kitti_semantic_labels::PARKING,
                        kitti_semantic_labels::SIDEWALK,
                        kitti_semantic_labels::BUILDING,
                        kitti_semantic_labels::FENCE,
                        kitti_semantic_labels::VEGETATION,
                        kitti_semantic_labels::TERRAIN
                    });
                    
                    LOG_INFO_S << "  - Static points: " << static_cloud->size();
                }
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Error loading Semantic KITTI: " << e.what();
    }
}

// Example 4: Semantic KITTI pair dataset for scene flow
void example_semantic_kitti_pair_dataset() {
    LOG_INFO_S << "\n=== Example 4: Semantic KITTI Pair Dataset ===";
    
    try {
        // Create semantic pair dataset
        semantic_kitti_pair_dataset_t<float> dataset(
            "/path/to/semantic_kitti/sequences/08", 1);
        
        // Enable motion filtering
        dataset.enable_motion_filter(true);
        dataset.set_motion_thresholds(0.1f, 0.01f); // 10cm or ~0.6 degrees
        
        LOG_INFO_S << "Created semantic pair dataset with " 
                        << dataset.size() << " pairs";
        
        // Get motion statistics
        auto motion_stats = dataset.compute_motion_statistics();
        LOG_INFO_S << "\nMotion statistics:";
        LOG_INFO_S << "  - Mean translation: " 
                        << motion_stats["translation_mean"] << " m";
        LOG_INFO_S << "  - Mean rotation: " 
                        << motion_stats["rotation_mean_deg"] << " deg";
        
        // Find pairs with moving objects (cars, trucks, etc.)
        std::vector<uint16_t> moving_classes = {
            kitti_semantic_labels::CAR,
            kitti_semantic_labels::TRUCK,
            kitti_semantic_labels::OTHER_VEHICLE,
            kitti_semantic_labels::PERSON,
            kitti_semantic_labels::BICYCLIST,
            kitti_semantic_labels::MOTORCYCLIST
        };
        
        auto dynamic_pairs = dataset.get_pairs_with_labels(moving_classes, 50);
        LOG_INFO_S << "\nFound " << dynamic_pairs.size() 
                        << " pairs with moving objects";
        
        // Process first pair with semantic information
        if (!dynamic_pairs.empty()) {
            auto pair = dataset[dynamic_pairs[0]];
            if (pair) {
                // Separate static and dynamic points
                auto [src_static, tgt_static] = pair->extract_static_points();
                auto [src_dynamic, tgt_dynamic] = pair->extract_dynamic_points();
                
                LOG_INFO_S << "\nPair " << dynamic_pairs[0] << ":";
                LOG_INFO_S << "  - Source static: " << src_static->size();
                LOG_INFO_S << "  - Source dynamic: " << src_dynamic->size();
                LOG_INFO_S << "  - Target static: " << tgt_static->size();
                LOG_INFO_S << "  - Target dynamic: " << tgt_dynamic->size();
                
                // The static points can be used for ego-motion estimation
                // The dynamic points can be used for object tracking
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Error with semantic pair dataset: " << e.what();
    }
}

// Example 5: Custom dataset processing pipeline
void example_custom_pipeline() {
    LOG_INFO_S << "\n=== Example 5: Custom Processing Pipeline ===";
    
    try {
        // Configure dataset with custom settings
        kitti_odometry_dataset_t<double> dataset("/path/to/kitti/sequences/00");
        dataset.set_cache_size(100); // Cache last 100 frames
        
        // Process dataset in batches
        const std::size_t batch_size = 50;
        std::size_t num_batches = (dataset.size() + batch_size - 1) / batch_size;
        
        for (std::size_t batch = 0; batch < num_batches; ++batch) {
            std::size_t start = batch * batch_size;
            std::size_t end = std::min(start + batch_size, dataset.size());
            
            LOG_INFO_S << "\nProcessing batch " << batch + 1 
                            << "/" << num_batches
                            << " (frames " << start << "-" << end-1 << ")";
            
            // Accumulate points for mapping
            std::size_t total_points = 0;
            
            for (std::size_t i = start; i < end; ++i) {
                auto frame = dataset[i];
                if (!frame) continue;
                
                total_points += frame->cloud->size();
                
                // Apply custom processing here
                // e.g., filtering, feature extraction, etc.
            }
            
            LOG_INFO_S << "  - Total points in batch: " << total_points;
            
            // Clear cache between batches to manage memory
            if (batch < num_batches - 1) {
                dataset.clear_cache();
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR_S << "Error in custom pipeline: " << e.what();
    }
}

int main(int argc, char* argv[]) {
    // Initialize logger
    toolbox::logger::thread_logger_t::instance().set_level(toolbox::logger::thread_logger_t::Level::DEBUG);
    
    // Print usage if no dataset path provided
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <kitti_dataset_path>\n";
        std::cout << "This will run with mock paths. "
                  << "Provide actual KITTI dataset path for real data.\n\n";
    }
    
    // Run examples
    example_basic_kitti_dataset();
    example_kitti_pair_dataset();
    example_semantic_kitti_dataset();
    example_semantic_kitti_pair_dataset();
    example_custom_pipeline();
    
    // Example of using the new helper function
    LOG_INFO_S << "\n=== Example: Reading KITTI with Labels ===";
    try {
        std::vector<uint32_t> labels;
        auto cloud = read_kitti_with_labels<float>(
            "/path/to/kitti/velodyne/000000.bin",
            "/path/to/kitti/labels/000000.label",
            labels
        );
        
        if (cloud && cloud->size() == labels.size()) {
            LOG_INFO_S << "Successfully loaded " << cloud->size() 
                      << " points with labels";
            
            // Count points per semantic class
            std::map<uint16_t, std::size_t> class_counts;
            for (const auto& label : labels) {
                uint16_t semantic_id = get_kitti_label_id(label);
                class_counts[semantic_id]++;
            }
            
            LOG_INFO_S << "Points per class:";
            for (const auto& [class_id, count] : class_counts) {
                LOG_INFO_S << "  - Class " << class_id << ": " << count << " points";
            }
        }
    } catch (const std::exception& e) {
        LOG_DEBUG_S << "Could not load example files: " << e.what();
    }
    
    // Shutdown logger
    toolbox::logger::thread_logger_t::instance().shutdown();
    
    return 0;
}
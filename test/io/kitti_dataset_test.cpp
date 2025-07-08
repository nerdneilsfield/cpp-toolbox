#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_pair_dataset.hpp>
#include <cpp-toolbox/io/dataloader/semantic_kitti_dataset.hpp>
#include <cpp-toolbox/io/dataloader/semantic_kitti_pair_dataset.hpp>
#include <cpp-toolbox/io/formats/kitti_extended.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <filesystem>
#include <fstream>
#include <random>

#include "test_data_dir.hpp"

using namespace toolbox::io;
using namespace toolbox::types;

namespace fs = std::filesystem;

// Helper function to create mock KITTI dataset structure
void create_mock_kitti_sequence(const fs::path& base_path,
                               int num_frames = 5,
                               bool with_poses = true,
                               bool with_labels = false) {
    // Create directories
    fs::create_directories(base_path / "velodyne");
    if (with_poses) {
        fs::create_directories(base_path.parent_path() / "poses");
    }
    if (with_labels) {
        fs::create_directories(base_path / "labels");
    }
    fs::create_directories(base_path / "calib");
    
    // Create mock point cloud files
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> coord_dist(-50.0f, 50.0f);
    std::uniform_real_distribution<float> intensity_dist(0.0f, 1.0f);
    
    for (int i = 0; i < num_frames; ++i) {
        // Create point cloud file
        char filename[16];
        std::sprintf(filename, "%06d.bin", i);
        fs::path cloud_path = base_path / "velodyne" / filename;
        
        std::ofstream cloud_file(cloud_path, std::ios::binary);
        
        // Write 100 random points per frame
        for (int j = 0; j < 100; ++j) {
            float data[4] = {
                coord_dist(gen),
                coord_dist(gen),
                coord_dist(gen),
                intensity_dist(gen)
            };
            cloud_file.write(reinterpret_cast<const char*>(data), sizeof(data));
        }
        cloud_file.close();
        
        // Create label file if requested
        if (with_labels) {
            std::sprintf(filename, "%06d.label", i);
            fs::path label_path = base_path / "labels" / filename;
            
            std::ofstream label_file(label_path, std::ios::binary);
            
            // Write 100 random labels
            std::uniform_int_distribution<uint16_t> label_dist(0, 50);
            for (int j = 0; j < 100; ++j) {
                uint32_t label = label_dist(gen); // Simple label ID
                label_file.write(reinterpret_cast<const char*>(&label), sizeof(label));
            }
            label_file.close();
        }
    }
    
    // Create pose file if requested
    if (with_poses) {
        fs::create_directories(base_path.parent_path().parent_path() / "poses");
        std::string sequence_name = base_path.filename().string();
        fs::path pose_path = base_path.parent_path().parent_path() / "poses" / (sequence_name + ".txt");
        
        std::ofstream pose_file(pose_path);
        
        // Write identity poses with small offsets
        for (int i = 0; i < num_frames; ++i) {
            // Simple forward motion
            float offset = static_cast<float>(i) * 0.5f;
            pose_file << "1 0 0 " << offset << " "
                     << "0 1 0 0 "
                     << "0 0 1 0\n";
        }
        pose_file.close();
    }
    
    // Create calibration file
    fs::path calib_path = base_path / "calib.txt";
    std::ofstream calib_file(calib_path);
    calib_file << "Tr: 1 0 0 0 0 1 0 0 0 0 1 0\n";
    calib_file.close();
}

// Helper to clean up test data
void cleanup_mock_sequence(const fs::path& base_path) {
    if (fs::exists(base_path)) {
        fs::remove_all(base_path);
    }
    // Also clean up poses directory
    fs::path poses_dir = base_path.parent_path().parent_path() / "poses";
    if (fs::exists(poses_dir)) {
        fs::remove_all(poses_dir);
    }
}

TEST_CASE("KITTI Odometry Dataset", "[io][kitti][dataset]") {
    const fs::path test_sequence_path = fs::temp_directory_path() / "kitti_test" / "sequences" / "00";
    
    // Cleanup before test
    cleanup_mock_sequence(test_sequence_path);
    
    SECTION("Basic dataset loading") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, false);
        
        kitti_odometry_dataset_t<float> dataset(test_sequence_path.string());
        
        REQUIRE(dataset.size() == 10);
        REQUIRE(dataset.get_sequence_name() == "00");
        
        // Test frame access
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        REQUIRE(frame->cloud != nullptr);
        REQUIRE(frame->cloud->size() == 100);
        REQUIRE(frame->frame_index == 0);
        
        // Test pose
        Eigen::Matrix4f expected_pose = Eigen::Matrix4f::Identity();
        REQUIRE(frame->pose.isApprox(expected_pose));
        
        // Test iteration
        int count = 0;
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            auto f = dataset[i];
            REQUIRE(f.has_value());
            REQUIRE(f->cloud != nullptr);
            count++;
        }
        REQUIRE(count == 10);
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Dataset without poses") {
        create_mock_kitti_sequence(test_sequence_path, 5, false, false);
        
        kitti_odometry_dataset_t<float> dataset(test_sequence_path.string());
        
        REQUIRE(dataset.size() == 5);
        // No has_poses method, just check that frames have identity poses
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        // Should have identity pose when no pose file exists
        REQUIRE(frame->pose.isApprox(Eigen::Matrix4f::Identity()));
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Cache functionality") {
        create_mock_kitti_sequence(test_sequence_path, 20, true, false);
        
        kitti_odometry_dataset_t<float> dataset(test_sequence_path.string());
        dataset.set_cache_size(5);
        
        // Access frames to fill cache
        for (int i = 0; i < 10; ++i) {
            auto frame = dataset[i];
            REQUIRE(frame.has_value());
        }
        
        // Clear cache and re-access
        dataset.clear_cache();
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Invalid sequence path") {
        REQUIRE_THROWS_AS(
            kitti_odometry_dataset_t<float>("/non/existent/path"),
            kitti_invalid_sequence
        );
    }
}

TEST_CASE("KITTI Odometry Pair Dataset", "[io][kitti][dataset][pair]") {
    const fs::path test_sequence_path = fs::temp_directory_path() / "kitti_test" / "sequences" / "01";
    
    cleanup_mock_sequence(test_sequence_path);
    
    SECTION("Basic pair dataset") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, false);
        
        kitti_odometry_pair_dataset_t<float> dataset(test_sequence_path.string(), 1);
        
        REQUIRE(dataset.size() == 9); // 10 frames - 1 skip = 9 pairs
        REQUIRE(dataset.get_skip() == 1);
        
        auto pair = dataset[0];
        REQUIRE(pair.has_value());
        REQUIRE(pair->source_cloud != nullptr);
        REQUIRE(pair->target_cloud != nullptr);
        REQUIRE(pair->source_index == 0);
        REQUIRE(pair->target_index == 1);
        
        // Test relative transform
        // relative = source_pose.inverse() * target_pose
        // source at x=0, target at x=0.5, so relative should have x=0.5
        Eigen::Matrix4f expected_relative = Eigen::Matrix4f::Identity();
        expected_relative(0, 3) = 0.5f; // Target is 0.5 forward from source
        REQUIRE(pair->relative_transform.isApprox(expected_relative, 1e-4f));
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Pair dataset with skip") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, false);
        
        kitti_odometry_pair_dataset_t<float> dataset(test_sequence_path.string(), 3);
        
        REQUIRE(dataset.size() == 7); // 10 frames - 3 skip = 7 pairs
        
        auto pair = dataset[0];
        REQUIRE(pair.has_value());
        REQUIRE(pair->source_index == 0);
        REQUIRE(pair->target_index == 3);
        
        // Test changing skip
        dataset.set_skip(5);
        REQUIRE(dataset.size() == 5); // 10 frames - 5 skip = 5 pairs
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Motion filtering") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, false);
        
        kitti_odometry_pair_dataset_t<float> dataset(test_sequence_path.string(), 1);
        
        // Get pairs with minimum motion
        auto moving_pairs = dataset.get_pairs_with_motion(0.4f, 0.0f);
        REQUIRE(moving_pairs.size() == 9); // All pairs have 0.5m motion
        
        auto large_motion_pairs = dataset.get_pairs_with_motion(1.0f, 0.0f);
        REQUIRE(large_motion_pairs.size() == 0); // No pairs have 1m+ motion
        
        cleanup_mock_sequence(test_sequence_path);
    }
}

TEST_CASE("Semantic KITTI Dataset", "[io][kitti][dataset][semantic]") {
    const fs::path test_sequence_path = fs::temp_directory_path() / "kitti_test" / "sequences" / "02";
    
    cleanup_mock_sequence(test_sequence_path);
    
    SECTION("Dataset with labels") {
        create_mock_kitti_sequence(test_sequence_path, 5, true, true);
        
        semantic_kitti_dataset_t<float> dataset(test_sequence_path.string());
        
        REQUIRE(dataset.size() == 5);
        REQUIRE(dataset.has_labels());
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        REQUIRE(frame->cloud != nullptr);
        REQUIRE(frame->labels.size() == frame->cloud->size());
        
        // Test label statistics
        auto stats = dataset.compute_label_statistics(1);
        REQUIRE(stats.size() > 0);
        
        // Test unique labels
        auto unique_labels = dataset.get_unique_labels(false);
        REQUIRE(unique_labels.size() > 0);
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Dataset without labels") {
        create_mock_kitti_sequence(test_sequence_path, 5, true, false);
        
        semantic_kitti_dataset_t<float> dataset(test_sequence_path.string());
        
        REQUIRE(dataset.size() == 5);
        REQUIRE_FALSE(dataset.has_labels());
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        // Should fill with UNLABELED
        REQUIRE(frame->labels.size() == frame->cloud->size());
        for (const auto& label : frame->labels) {
            REQUIRE(get_kitti_label_id(label) == kitti_semantic_labels::UNLABELED);
        }
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Frame methods") {
        create_mock_kitti_sequence(test_sequence_path, 3, true, true);
        
        semantic_kitti_dataset_t<float> dataset(test_sequence_path.string());
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        
        // Test label extraction
        auto stats = frame->get_label_statistics();
        REQUIRE(stats.size() > 0);
        
        // Test filtered cloud extraction
        auto filtered = frame->get_labeled_cloud({0, 1, 2});
        REQUIRE(filtered != nullptr);
        REQUIRE(filtered->size() <= frame->cloud->size());
        
        cleanup_mock_sequence(test_sequence_path);
    }
}

TEST_CASE("Semantic KITTI Pair Dataset", "[io][kitti][dataset][semantic][pair]") {
    const fs::path test_sequence_path = fs::temp_directory_path() / "kitti_test" / "sequences" / "03";
    
    cleanup_mock_sequence(test_sequence_path);
    
    SECTION("Basic semantic pairs") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, true);
        
        semantic_kitti_pair_dataset_t<float> dataset(test_sequence_path.string(), 2);
        
        REQUIRE(dataset.size() == 8); // 10 frames - 2 skip = 8 pairs
        
        auto pair = dataset[0];
        REQUIRE(pair.has_value());
        REQUIRE(pair->source_cloud != nullptr);
        REQUIRE(pair->target_cloud != nullptr);
        REQUIRE(pair->source_labels.size() == pair->source_cloud->size());
        REQUIRE(pair->target_labels.size() == pair->target_cloud->size());
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Motion filtering") {
        create_mock_kitti_sequence(test_sequence_path, 10, true, true);
        
        semantic_kitti_pair_dataset_t<float> dataset(test_sequence_path.string(), 1);
        
        dataset.enable_motion_filter(true);
        dataset.set_motion_thresholds(0.4f, 0.0f);
        
        // Compute motion statistics
        auto stats = dataset.compute_motion_statistics();
        REQUIRE(stats.count("translation_mean") > 0);
        REQUIRE(stats.count("rotation_mean_rad") > 0);
        REQUIRE(stats["num_pairs"] > 0);
        
        cleanup_mock_sequence(test_sequence_path);
    }
    
    SECTION("Pair frame methods") {
        create_mock_kitti_sequence(test_sequence_path, 5, true, true);
        
        semantic_kitti_pair_dataset_t<float> dataset(test_sequence_path.string(), 1);
        
        auto pair = dataset[0];
        REQUIRE(pair.has_value());
        
        // Test static/dynamic point extraction
        auto [src_static, tgt_static] = pair->extract_static_points();
        REQUIRE(src_static != nullptr);
        REQUIRE(tgt_static != nullptr);
        
        auto [src_dynamic, tgt_dynamic] = pair->extract_dynamic_points();
        REQUIRE(src_dynamic != nullptr);
        REQUIRE(tgt_dynamic != nullptr);
        
        // Static + dynamic should equal total (approximately)
        REQUIRE(src_static->size() + src_dynamic->size() <= pair->source_cloud->size());
        
        cleanup_mock_sequence(test_sequence_path);
    }
}

TEST_CASE("KITTI Extended Format Functions", "[io][kitti][extended]") {
    const fs::path test_dir = fs::temp_directory_path() / "kitti_extended_test";
    fs::create_directories(test_dir);
    
    SECTION("Pose reading/writing") {
        // Create test pose file
        fs::path pose_file = test_dir / "poses.txt";
        std::ofstream out(pose_file);
        out << "1 0 0 1.5 0 1 0 0 0 0 1 0\n";
        out << "0.866 -0.5 0 2.0 0.5 0.866 0 1.0 0 0 1 0.5\n";
        out.close();
        
        // Read poses
        auto poses = read_kitti_poses<float>(pose_file.string());
        REQUIRE(poses.size() == 2);
        
        // Check first pose
        REQUIRE(poses[0](0, 3) == Catch::Approx(1.5f));
        
        // Check second pose has rotation
        REQUIRE(poses[1](0, 0) == Catch::Approx(0.866f).epsilon(0.001f));
        REQUIRE(poses[1](0, 1) == Catch::Approx(-0.5f));
        
        fs::remove_all(test_dir);
    }
    
    SECTION("Label reading/writing") {
        fs::path label_file = test_dir / "test.label";
        
        // Write test labels
        std::vector<uint32_t> test_labels = {
            0, 10, 20, 30, 40,
            (1u << 16) | 10, // With instance ID
            (2u << 16) | 20
        };
        write_kitti_labels(label_file.string(), test_labels);
        
        // Read back
        auto read_labels = read_kitti_labels(label_file.string());
        REQUIRE(read_labels.size() == test_labels.size());
        
        for (size_t i = 0; i < test_labels.size(); ++i) {
            REQUIRE(read_labels[i] == test_labels[i]);
            
            // Test label extraction
            uint16_t label_id = get_kitti_label_id(read_labels[i]);
            uint16_t instance_id = get_kitti_instance_id(read_labels[i]);
            
            if (i < 5) {
                REQUIRE(label_id == i * 10);
                REQUIRE(instance_id == 0);
            } else {
                REQUIRE(instance_id > 0);
            }
        }
        
        fs::remove_all(test_dir);
    }
    
    SECTION("File listing utilities") {
        // Create test files
        fs::create_directories(test_dir / "velodyne");
        fs::create_directories(test_dir / "labels");
        
        for (int i = 0; i < 5; ++i) {
            char filename[16];
            std::sprintf(filename, "%06d.bin", i);
            std::ofstream(test_dir / "velodyne" / filename).close();
            
            std::sprintf(filename, "%06d.label", i);
            std::ofstream(test_dir / "labels" / filename).close();
        }
        
        // Test listing
        auto bin_files = list_kitti_cloud_files((test_dir / "velodyne").string());
        REQUIRE(bin_files.size() == 5);
        
        auto label_files = list_kitti_label_files((test_dir / "labels").string());
        REQUIRE(label_files.size() == 5);
        
        // Test frame index parsing
        for (int i = 0; i < 5; ++i) {
            REQUIRE(parse_kitti_frame_index(bin_files[i]) == i);
            REQUIRE(parse_kitti_frame_index(label_files[i]) == i);
        }
        
        fs::remove_all(test_dir);
    }
    
    SECTION("Read point cloud with labels") {
        fs::path test_file_bin = test_dir / "test.bin";
        fs::path test_file_label = test_dir / "test.label";
        
        // Create test point cloud with 10 points
        std::vector<float> points;
        std::vector<uint32_t> labels;
        
        for (int i = 0; i < 10; ++i) {
            // x, y, z, intensity
            points.push_back(i * 1.0f);     // x
            points.push_back(i * 2.0f);     // y
            points.push_back(i * 3.0f);     // z
            points.push_back(0.5f);         // intensity (will be overwritten)
            
            // Label with semantic class and instance ID
            uint16_t semantic_id = i % 5;  // 5 different classes
            uint16_t instance_id = i / 5;  // 2 instances per class
            labels.push_back((instance_id << 16) | semantic_id);
        }
        
        // Write binary point cloud
        std::ofstream bin_file(test_file_bin, std::ios::binary);
        bin_file.write(reinterpret_cast<const char*>(points.data()), 
                      points.size() * sizeof(float));
        bin_file.close();
        
        // Write labels
        write_kitti_labels(test_file_label.string(), labels);
        
        // Read with helper function
        std::vector<uint32_t> read_labels;
        auto cloud = read_kitti_with_labels<float>(
            test_file_bin.string(), 
            test_file_label.string(),
            read_labels
        );
        
        REQUIRE(cloud != nullptr);
        REQUIRE(cloud->size() == 10);
        REQUIRE(read_labels.size() == 10);
        
        // Check that labels were correctly read
        for (int i = 0; i < 10; ++i) {
            REQUIRE(read_labels[i] == labels[i]);
            
            // Verify we can extract semantic and instance IDs
            uint16_t semantic_id = get_kitti_label_id(read_labels[i]);
            uint16_t instance_id = get_kitti_instance_id(read_labels[i]);
            
            REQUIRE(semantic_id == (i % 5));
            REQUIRE(instance_id == (i / 5));
            
            // Verify point coordinates
            REQUIRE(cloud->points[i].x == Catch::Approx(i * 1.0f));
            REQUIRE(cloud->points[i].y == Catch::Approx(i * 2.0f));
            REQUIRE(cloud->points[i].z == Catch::Approx(i * 3.0f));
        }
        
        // Test error cases
        std::vector<uint32_t> dummy_labels;
        REQUIRE_THROWS_AS(
            read_kitti_with_labels<float>("/non/existent.bin", test_file_label.string(), dummy_labels),
            std::runtime_error
        );
        
        REQUIRE_THROWS_AS(
            read_kitti_with_labels<float>(test_file_bin.string(), "/non/existent.label", dummy_labels),
            std::runtime_error
        );
        
        // Test size mismatch
        std::vector<uint32_t> wrong_labels = {1, 2, 3}; // Only 3 labels for 10 points
        write_kitti_labels(test_file_label.string(), wrong_labels);
        
        std::vector<uint32_t> dummy_labels2;
        REQUIRE_THROWS_AS(
            read_kitti_with_labels<float>(test_file_bin.string(), test_file_label.string(), dummy_labels2),
            std::runtime_error
        );
        
        fs::remove_all(test_dir);
    }
}
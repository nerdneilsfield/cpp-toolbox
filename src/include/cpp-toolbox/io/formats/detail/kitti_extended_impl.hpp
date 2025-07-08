#pragma once

#include <cpp-toolbox/io/formats/kitti_extended.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <map>

namespace toolbox::io {

// ==================== Label I/O Implementation ====================

inline std::vector<uint32_t> read_kitti_labels(const std::string& file_path) {
    std::vector<uint32_t> labels;
    
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open label file: " + file_path);
    }
    
    const auto file_size = file.tellg();
    file.seekg(0);
    
    if (file_size % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Label file size is not a multiple of uint32_t");
    }
    
    const auto num_labels = static_cast<std::size_t>(file_size) / sizeof(uint32_t);
    labels.resize(num_labels);
    
    file.read(reinterpret_cast<char*>(labels.data()), file_size);
    
    if (!file) {
        throw std::runtime_error("Failed to read labels from: " + file_path);
    }
    
    return labels;
}

inline bool write_kitti_labels(const std::string& file_path,
                              const std::vector<uint32_t>& labels) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(labels.data()),
               labels.size() * sizeof(uint32_t));
    
    return file.good();
}

// ==================== Pose I/O Implementation ====================

template<typename DataType>
Eigen::Matrix<DataType, 4, 4> parse_kitti_pose_line(const std::string& line) {
    std::istringstream iss(line);
    Eigen::Matrix<DataType, 4, 4> pose = Eigen::Matrix<DataType, 4, 4>::Identity();
    
    // Read 3x4 matrix values
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            DataType value;
            if (!(iss >> value)) {
                throw std::invalid_argument("Invalid pose format in line: " + line);
            }
            pose(row, col) = value;
        }
    }
    
    // Last row is always [0, 0, 0, 1]
    pose(3, 0) = 0;
    pose(3, 1) = 0;
    pose(3, 2) = 0;
    pose(3, 3) = 1;
    
    return pose;
}

template<typename DataType>
std::vector<Eigen::Matrix<DataType, 4, 4>> read_kitti_poses(const std::string& file_path) {
    std::vector<Eigen::Matrix<DataType, 4, 4>> poses;
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open poses file: " + file_path);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        
        poses.push_back(parse_kitti_pose_line<DataType>(line));
    }
    
    return poses;
}

template<typename DataType>
std::string format_kitti_pose(const Eigen::Matrix<DataType, 4, 4>& pose) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(9);
    
    // Write first 3 rows
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (row > 0 || col > 0) oss << " ";
            oss << pose(row, col);
        }
    }
    
    return oss.str();
}

template<typename DataType>
bool write_kitti_poses(const std::string& file_path,
                      const std::vector<Eigen::Matrix<DataType, 4, 4>>& poses) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& pose : poses) {
        file << format_kitti_pose(pose) << "\n";
    }
    
    return file.good();
}

// ==================== Calibration I/O Implementation ====================

template<typename DataType>
kitti_calibration_t<DataType> read_kitti_calibration(const std::string& file_path) {
    kitti_calibration_t<DataType> calib;
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open calibration file: " + file_path);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        
        if (!(iss >> key)) continue;
        
        // Remove the colon
        if (!key.empty() && key.back() == ':') {
            key.pop_back();
        }
        
        if (key == "P0") {
            for (int i = 0; i < 12; ++i) {
                iss >> calib.P0(i / 4, i % 4);
            }
        } else if (key == "P1") {
            for (int i = 0; i < 12; ++i) {
                iss >> calib.P1(i / 4, i % 4);
            }
        } else if (key == "P2") {
            for (int i = 0; i < 12; ++i) {
                iss >> calib.P2(i / 4, i % 4);
            }
        } else if (key == "P3") {
            for (int i = 0; i < 12; ++i) {
                iss >> calib.P3(i / 4, i % 4);
            }
        } else if (key == "R0_rect") {
            for (int i = 0; i < 9; ++i) {
                iss >> calib.R0_rect(i / 3, i % 3);
            }
        } else if (key == "Tr_velo_to_cam") {
            for (int i = 0; i < 12; ++i) {
                iss >> calib.Tr_velo_to_cam(i / 4, i % 4);
            }
            calib.Tr_velo_to_cam(3, 0) = 0;
            calib.Tr_velo_to_cam(3, 1) = 0;
            calib.Tr_velo_to_cam(3, 2) = 0;
            calib.Tr_velo_to_cam(3, 3) = 1;
        } else if (key == "Tr_imu_to_velo") {
            Eigen::Matrix<DataType, 4, 4> imu_to_velo = 
                Eigen::Matrix<DataType, 4, 4>::Identity();
            for (int i = 0; i < 12; ++i) {
                iss >> imu_to_velo(i / 4, i % 4);
            }
            imu_to_velo(3, 0) = 0;
            imu_to_velo(3, 1) = 0;
            imu_to_velo(3, 2) = 0;
            imu_to_velo(3, 3) = 1;
            calib.Tr_imu_to_velo = imu_to_velo;
        }
    }
    
    return calib;
}

template<typename DataType>
bool write_kitti_calibration(const std::string& file_path,
                            const kitti_calibration_t<DataType>& calib) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << std::fixed << std::setprecision(9);
    
    // Write P0-P3
    file << "P0: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.P0(i, j);
        }
    }
    file << "\n";
    
    file << "P1: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.P1(i, j);
        }
    }
    file << "\n";
    
    file << "P2: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.P2(i, j);
        }
    }
    file << "\n";
    
    file << "P3: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.P3(i, j);
        }
    }
    file << "\n";
    
    // Write R0_rect
    file << "R0_rect: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.R0_rect(i, j);
        }
    }
    file << "\n";
    
    // Write Tr_velo_to_cam
    file << "Tr_velo_to_cam: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i > 0 || j > 0) file << " ";
            file << calib.Tr_velo_to_cam(i, j);
        }
    }
    file << "\n";
    
    // Write Tr_imu_to_velo if available
    if (calib.Tr_imu_to_velo.has_value()) {
        file << "Tr_imu_to_velo: ";
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (i > 0 || j > 0) file << " ";
                file << calib.Tr_imu_to_velo.value()(i, j);
            }
        }
        file << "\n";
    }
    
    return file.good();
}

// ==================== Utility Functions Implementation ====================

template<typename DataType>
Eigen::Matrix<DataType, 4, 4> compute_relative_transform(
    const Eigen::Matrix<DataType, 4, 4>& from_pose,
    const Eigen::Matrix<DataType, 4, 4>& to_pose) {
    // T_to_from = T_to_world * T_world_from = to_pose * from_pose.inverse()
    return to_pose * from_pose.inverse();
}

template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> transform_point_cloud(
    const point_cloud_t<DataType>& cloud,
    const Eigen::Matrix<DataType, 4, 4>& transform) {
    
    auto transformed = std::make_unique<point_cloud_t<DataType>>();
    transformed->points.reserve(cloud.points.size());
    transformed->intensity = cloud.intensity;
    
    for (const auto& pt : cloud.points) {
        Eigen::Vector4d homogeneous(pt.x, pt.y, pt.z, 1.0);
        Eigen::Vector4d transformed_pt = transform.template cast<double>() * homogeneous;
        
        point_t<DataType> new_pt;
        new_pt.x = static_cast<DataType>(transformed_pt[0]);
        new_pt.y = static_cast<DataType>(transformed_pt[1]);
        new_pt.z = static_cast<DataType>(transformed_pt[2]);
        
        transformed->points.push_back(new_pt);
    }
    
    return transformed;
}

template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> read_kitti_with_labels(
    const std::string& bin_path,
    const std::string& label_path,
    std::vector<uint32_t>& labels) {
    
    // Read point cloud
    auto cloud = read_kitti_bin<DataType>(bin_path);
    if (!cloud) {
        throw std::runtime_error("Failed to read point cloud from: " + bin_path);
    }
    
    // Read labels
    try {
        labels = read_kitti_labels(label_path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to read labels from: " + label_path + ". Error: " + e.what());
    }
    
    // Check size consistency
    if (cloud->size() != labels.size()) {
        throw std::runtime_error("Point cloud size (" + std::to_string(cloud->size()) + 
                               ") does not match label count (" + std::to_string(labels.size()) + ")");
    }
    
    return cloud;
}

inline std::vector<std::string> list_kitti_cloud_files(const std::string& velodyne_path) {
    namespace fs = std::filesystem;
    
    std::vector<std::string> files;
    
    if (!fs::exists(velodyne_path)) {
        return files;
    }
    
    for (const auto& entry : fs::directory_iterator(velodyne_path)) {
        if (entry.path().extension() == ".bin") {
            files.push_back(entry.path().string());
        }
    }
    
    // Sort by filename
    std::sort(files.begin(), files.end());
    
    return files;
}

inline std::vector<std::string> list_kitti_label_files(const std::string& labels_path) {
    namespace fs = std::filesystem;
    
    std::vector<std::string> files;
    
    if (!fs::exists(labels_path)) {
        return files;
    }
    
    for (const auto& entry : fs::directory_iterator(labels_path)) {
        if (entry.path().extension() == ".label") {
            files.push_back(entry.path().string());
        }
    }
    
    // Sort by filename
    std::sort(files.begin(), files.end());
    
    return files;
}

inline int parse_kitti_frame_index(const std::string& filename) {
    namespace fs = std::filesystem;
    
    fs::path path(filename);
    std::string stem = path.stem().string();
    
    try {
        return std::stoi(stem);
    } catch (const std::exception&) {
        return -1;
    }
}

inline std::string format_kitti_frame_index(std::size_t index, int digits) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(digits) << index;
    return oss.str();
}

inline kitti_sequence_info_t get_kitti_sequence_info(const std::string& sequence_path) {
    namespace fs = std::filesystem;
    
    kitti_sequence_info_t info;
    info.path = fs::path(sequence_path);
    info.sequence_name = info.path.filename().string();
    
    // Count velodyne files
    fs::path velodyne_path = info.path / "velodyne";
    if (fs::exists(velodyne_path)) {
        info.num_frames = 0;
        for (const auto& entry : fs::directory_iterator(velodyne_path)) {
            if (entry.path().extension() == ".bin") {
                info.num_frames++;
            }
        }
    }
    
    // Check for labels
    info.has_labels = fs::exists(info.path / "labels");
    
    // Check for calibration
    info.has_calibration = fs::exists(info.path / "calib.txt");
    
    return info;
}

inline bool validate_kitti_sequence_directory(const std::string& sequence_path) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(sequence_path) || !fs::is_directory(sequence_path)) {
        return false;
    }
    
    // Check for velodyne directory
    fs::path velodyne_path = fs::path(sequence_path) / "velodyne";
    if (!fs::exists(velodyne_path) || !fs::is_directory(velodyne_path)) {
        return false;
    }
    
    return true;
}

// ==================== Label Names Implementation ====================

namespace kitti_semantic_labels {

inline std::string get_label_name(uint16_t label) {
    static const std::map<uint16_t, std::string> label_names = {
        {0, "unlabeled"},
        {1, "outlier"},
        {10, "car"},
        {11, "bicycle"},
        {13, "bus"},
        {15, "motorcycle"},
        {18, "truck"},
        {20, "other-vehicle"},
        {30, "person"},
        {31, "bicyclist"},
        {32, "motorcyclist"},
        {40, "road"},
        {44, "parking"},
        {48, "sidewalk"},
        {49, "other-ground"},
        {50, "building"},
        {51, "fence"},
        {52, "other-structure"},
        {70, "vegetation"},
        {71, "trunk"},
        {72, "terrain"},
        {80, "pole"},
        {81, "traffic-sign"}
    };
    
    auto it = label_names.find(label);
    if (it != label_names.end()) {
        return it->second;
    }
    return "unknown";
}

inline std::map<uint16_t, std::string> get_label_map() {
    return {
        {0, "unlabeled"},
        {1, "outlier"},
        {10, "car"},
        {11, "bicycle"},
        {13, "bus"},
        {15, "motorcycle"},
        {18, "truck"},
        {20, "other-vehicle"},
        {30, "person"},
        {31, "bicyclist"},
        {32, "motorcyclist"},
        {40, "road"},
        {44, "parking"},
        {48, "sidewalk"},
        {49, "other-ground"},
        {50, "building"},
        {51, "fence"},
        {52, "other-structure"},
        {70, "vegetation"},
        {71, "trunk"},
        {72, "terrain"},
        {80, "pole"},
        {81, "traffic-sign"}
    };
}

} // namespace kitti_semantic_labels

} // namespace toolbox::io
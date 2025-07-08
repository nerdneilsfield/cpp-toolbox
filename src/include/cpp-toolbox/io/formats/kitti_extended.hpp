#pragma once

#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Dense>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace toolbox::io {

// ==================== Label I/O ====================

/**
 * @brief Read Semantic KITTI label file
 * @param file_path Path to the .label file
 * @return Vector of labels (one per point), empty on failure
 * @throws std::runtime_error if file cannot be read
 */
std::vector<uint32_t> read_kitti_labels(const std::string& file_path);

/**
 * @brief Write labels to Semantic KITTI format
 * @param file_path Output file path
 * @param labels Vector of labels to write
 * @return True if successful
 */
bool write_kitti_labels(const std::string& file_path,
                       const std::vector<uint32_t>& labels);

// ==================== Pose I/O ====================

/**
 * @brief Parse a single line from KITTI poses file
 * @tparam DataType The floating point type (float or double)
 * @param line Line containing 12 values (3x4 transformation matrix)
 * @return 4x4 transformation matrix (with [0,0,0,1] as last row)
 * @throws std::invalid_argument if line format is invalid
 */
template<typename DataType>
Eigen::Matrix<DataType, 4, 4> parse_kitti_pose_line(const std::string& line);

/**
 * @brief Read all poses from KITTI poses file
 * @tparam DataType The floating point type (float or double)
 * @param file_path Path to poses file (e.g., "00.txt")
 * @return Vector of 4x4 transformation matrices
 */
template<typename DataType>
std::vector<Eigen::Matrix<DataType, 4, 4>> read_kitti_poses(const std::string& file_path);

/**
 * @brief Format pose matrix as KITTI string
 * @tparam DataType The floating point type (float or double)
 * @param pose 4x4 transformation matrix
 * @return String with 12 values (first 3 rows)
 */
template<typename DataType>
std::string format_kitti_pose(const Eigen::Matrix<DataType, 4, 4>& pose);

/**
 * @brief Write poses to KITTI format file
 * @tparam DataType The floating point type (float or double)
 * @param file_path Output file path
 * @param poses Vector of 4x4 transformation matrices
 * @return True if successful
 */
template<typename DataType>
bool write_kitti_poses(const std::string& file_path,
                      const std::vector<Eigen::Matrix<DataType, 4, 4>>& poses);

// ==================== Calibration I/O ====================

/**
 * @brief KITTI calibration data structure
 */
template<typename DataType>
struct kitti_calibration_t {
    /// Transformation from Velodyne to camera 0 rectified coordinates
    Eigen::Matrix<DataType, 4, 4> Tr_velo_to_cam;
    
    /// Projection matrices for cameras 0-3 after rectification
    Eigen::Matrix<DataType, 3, 4> P0, P1, P2, P3;
    
    /// Rectification rotation matrix for camera 0
    Eigen::Matrix<DataType, 3, 3> R0_rect;
    
    /// Transformation from IMU to Velodyne
    std::optional<Eigen::Matrix<DataType, 4, 4>> Tr_imu_to_velo;
};

/**
 * @brief Read KITTI calibration file
 * @tparam DataType The floating point type (float or double)
 * @param file_path Path to calib.txt file
 * @return Calibration data
 * @throws std::runtime_error if file cannot be parsed
 */
template<typename DataType>
kitti_calibration_t<DataType> read_kitti_calibration(const std::string& file_path);

/**
 * @brief Write KITTI calibration file
 * @tparam DataType The floating point type (float or double)
 * @param file_path Output file path
 * @param calib Calibration data to write
 * @return True if successful
 */
template<typename DataType>
bool write_kitti_calibration(const std::string& file_path,
                            const kitti_calibration_t<DataType>& calib);

// ==================== Utility Functions ====================

/**
 * @brief Compute relative transformation between two poses
 * @tparam DataType The floating point type (float or double)
 * @param from_pose Source pose
 * @param to_pose Target pose
 * @return Relative transformation T_to_from such that to_pose = T_to_from * from_pose
 */
template<typename DataType>
Eigen::Matrix<DataType, 4, 4> compute_relative_transform(
    const Eigen::Matrix<DataType, 4, 4>& from_pose,
    const Eigen::Matrix<DataType, 4, 4>& to_pose);

/**
 * @brief Transform point cloud using transformation matrix
 * @tparam DataType The floating point type
 * @param cloud Input point cloud
 * @param transform Transformation matrix
 * @return Transformed point cloud
 */
template<typename DataType>
std::unique_ptr<point_cloud_t<DataType>> transform_point_cloud(
    const point_cloud_t<DataType>& cloud,
    const Eigen::Matrix<DataType, 4, 4>& transform);

/**
 * @brief List all KITTI point cloud files in a directory
 * @param velodyne_path Path to velodyne directory
 * @return Sorted list of .bin files with full paths
 */
std::vector<std::string> list_kitti_cloud_files(const std::string& velodyne_path);

/**
 * @brief List all KITTI label files in a directory
 * @param labels_path Path to labels directory
 * @return Sorted list of .label files with full paths
 */
std::vector<std::string> list_kitti_label_files(const std::string& labels_path);

/**
 * @brief Get frame index from KITTI filename (e.g., "000123.bin" -> 123)
 * @param filename Filename to parse
 * @return Frame index or -1 if invalid
 */
int parse_kitti_frame_index(const std::string& filename);

/**
 * @brief Format frame index as KITTI filename (e.g., 123 -> "000123")
 * @param index Frame index
 * @param digits Number of digits (default 6 for KITTI)
 * @return Formatted string
 */
std::string format_kitti_frame_index(std::size_t index, int digits = 6);

/**
 * @brief Information about a KITTI sequence
 */
struct kitti_sequence_info_t {
    std::size_t num_frames = 0;      ///< Number of frames in the sequence
    bool has_labels = false;         ///< Whether labels directory exists
    bool has_calibration = false;    ///< Whether calibration file exists
    std::string sequence_name;       ///< Sequence name (e.g., "00", "01")
    std::filesystem::path path;      ///< Full path to sequence directory
};

/**
 * @brief Get information about a KITTI sequence directory
 * @param sequence_path Path to sequence directory
 * @return Sequence information
 */
kitti_sequence_info_t get_kitti_sequence_info(const std::string& sequence_path);

/**
 * @brief Validate KITTI sequence directory structure
 * @param sequence_path Path to sequence directory
 * @return True if valid KITTI structure
 */
bool validate_kitti_sequence_directory(const std::string& sequence_path);

// ==================== Label Definitions ====================

namespace kitti_semantic_labels {
    // Static classes
    constexpr uint16_t UNLABELED = 0;
    constexpr uint16_t OUTLIER = 1;
    constexpr uint16_t CAR = 10;
    constexpr uint16_t BICYCLE = 11;
    constexpr uint16_t BUS = 13;
    constexpr uint16_t MOTORCYCLE = 15;
    constexpr uint16_t TRUCK = 18;
    constexpr uint16_t OTHER_VEHICLE = 20;
    constexpr uint16_t PERSON = 30;
    constexpr uint16_t BICYCLIST = 31;
    constexpr uint16_t MOTORCYCLIST = 32;
    constexpr uint16_t ROAD = 40;
    constexpr uint16_t PARKING = 44;
    constexpr uint16_t SIDEWALK = 48;
    constexpr uint16_t OTHER_GROUND = 49;
    constexpr uint16_t BUILDING = 50;
    constexpr uint16_t FENCE = 51;
    constexpr uint16_t OTHER_STRUCTURE = 52;
    constexpr uint16_t VEGETATION = 70;
    constexpr uint16_t TRUNK = 71;
    constexpr uint16_t TERRAIN = 72;
    constexpr uint16_t POLE = 80;
    constexpr uint16_t TRAFFIC_SIGN = 81;
    
    /**
     * @brief Check if a label is static (non-moving)
     */
    inline bool is_static(uint16_t label) {
        return label == ROAD || label == PARKING || label == SIDEWALK ||
               label == OTHER_GROUND || label == BUILDING || label == FENCE ||
               label == OTHER_STRUCTURE || label == VEGETATION || label == TRUNK ||
               label == TERRAIN || label == POLE || label == TRAFFIC_SIGN;
    }
    
    /**
     * @brief Check if a label is dynamic (moving)
     */
    inline bool is_dynamic(uint16_t label) {
        return label == CAR || label == BICYCLE || label == BUS ||
               label == MOTORCYCLE || label == TRUCK || label == OTHER_VEHICLE ||
               label == PERSON || label == BICYCLIST || label == MOTORCYCLIST;
    }
    
    /**
     * @brief Get human-readable label name
     */
    std::string get_label_name(uint16_t label);
    
    /**
     * @brief Get all label names mapping
     */
    std::map<uint16_t, std::string> get_label_map();
} // namespace kitti_semantic_labels

/**
 * @brief Extract label ID from full label (ignoring instance ID)
 */
inline uint16_t get_kitti_label_id(uint32_t full_label) {
    return static_cast<uint16_t>(full_label & 0xFFFF);
}

/**
 * @brief Extract instance ID from full label
 */
inline uint16_t get_kitti_instance_id(uint32_t full_label) {
    return static_cast<uint16_t>(full_label >> 16);
}

/**
 * @brief Combine label and instance IDs
 */
inline uint32_t make_kitti_full_label(uint16_t label_id, uint16_t instance_id) {
    return (static_cast<uint32_t>(instance_id) << 16) | label_id;
}

} // namespace toolbox::io

// Include implementation
#include <cpp-toolbox/io/formats/detail/kitti_extended_impl.hpp>
#pragma once

#include <stdexcept>
#include <string>

namespace toolbox::io {

/**
 * @brief Base exception for KITTI dataset operations
 */
class kitti_exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Exception thrown when a file is not found
 */
class kitti_file_not_found : public kitti_exception {
public:
    explicit kitti_file_not_found(const std::string& file)
        : kitti_exception("File not found: " + file), file_path_(file) {}
    
    const std::string& file_path() const noexcept { return file_path_; }
    
private:
    std::string file_path_;
};

/**
 * @brief Exception thrown when file format is invalid
 */
class kitti_invalid_format : public kitti_exception {
public:
    explicit kitti_invalid_format(const std::string& msg)
        : kitti_exception("Invalid format: " + msg) {}
};

/**
 * @brief Exception thrown when data is corrupted
 */
class kitti_corrupted_data : public kitti_exception {
public:
    explicit kitti_corrupted_data(const std::string& msg)
        : kitti_exception("Corrupted data: " + msg) {}
};

/**
 * @brief Exception thrown when sequence directory structure is invalid
 */
class kitti_invalid_sequence : public kitti_exception {
public:
    explicit kitti_invalid_sequence(const std::string& path)
        : kitti_exception("Invalid sequence directory: " + path), 
          sequence_path_(path) {}
    
    const std::string& sequence_path() const noexcept { return sequence_path_; }
    
private:
    std::string sequence_path_;
};

/**
 * @brief Exception thrown when accessing out of bounds frame
 */
class kitti_index_out_of_range : public kitti_exception {
public:
    kitti_index_out_of_range(std::size_t index, std::size_t size)
        : kitti_exception("Index " + std::to_string(index) + 
                         " out of range [0, " + std::to_string(size) + ")"),
          index_(index), size_(size) {}
    
    std::size_t index() const noexcept { return index_; }
    std::size_t size() const noexcept { return size_; }
    
private:
    std::size_t index_;
    std::size_t size_;
};

} // namespace toolbox::io
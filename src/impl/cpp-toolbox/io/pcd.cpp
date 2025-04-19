#include <algorithm>  // std::find_if_not
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cpp-toolbox/container/string.hpp>  // ends_with
#include <cpp-toolbox/file/file.hpp>  // Include the actual mmap utility
#include <cpp-toolbox/file/memory_mapped_file.hpp>
#include <cpp-toolbox/io/formats/pcd.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

namespace toolbox::io::formats
{

using toolbox::file::memory_mapped_file_t;  // Use the correct class name
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

// --- pcd_format_t::pcd_header_t Implementation ---

bool pcd_format_t::pcd_header_t::parse_line(const std::string& line)
{
  std::stringstream ss(line);
  std::string keyword;
  ss >> keyword;

  if (keyword == "VERSION") {
    ss >> version;
  } else if (keyword == "FIELDS") {
    fields.clear();
    std::string field;
    while (ss >> field) {
      fields.push_back(field);
    }
  } else if (keyword == "SIZE") {
    sizes.clear();
    size_t size = 0;
    while (ss >> size) {
      sizes.push_back(size);
    }
  } else if (keyword == "TYPE") {
    types.clear();
    char type = 0;
    while (ss >> type) {
      types.push_back(type);
    }
  } else if (keyword == "COUNT") {
    counts.clear();
    size_t count = 0;
    while (ss >> count) {
      counts.push_back(count);
    }
  } else if (keyword == "WIDTH") {
    ss >> width;
  } else if (keyword == "HEIGHT") {
    ss >> height;
  } else if (keyword == "VIEWPOINT") {
    for (auto& v : viewpoint) {
      ss >> v;
    }
  } else if (keyword == "POINTS") {
    ss >> points;
  } else if (keyword == "DATA") {
    std::string type_str;
    ss >> type_str;
    if (type_str == "ascii") {
      data_type = data_type_t::ASCII;
    } else if (type_str == "binary") {
      data_type = data_type_t::BINARY;
    } else if (type_str == "binary_compressed") {
      data_type = data_type_t::BINARY_COMPRESSED;
    } else {
      data_type = data_type_t::UNKNOWN;
    }
    return true;  // DATA is the last header line
  }
  // Ignore comments (#) and unknown keywords
  return false;  // Continue parsing header
}

void pcd_format_t::pcd_header_t::calculate_point_step_and_indices()
{
  point_step = 0;
  m_field_indices.clear();
  if (fields.size() != sizes.size() || fields.size() != types.size()
      || fields.size() != counts.size())
  {
    // Error condition already handled in validation usually, but prevent
    // crash here
    LOG_ERROR_S
        << "Header field/size/type/count mismatch during index calculation.";
    return;
  }
  for (size_t i = 0; i < fields.size(); ++i) {
    m_field_indices[fields[i]] = i;
    point_step += sizes[i] * counts[i];
  }
}

std::optional<pcd_format_t::pcd_header_t::field_info_t>
pcd_format_t::pcd_header_t::get_field_info(const std::string& field_name) const
{
  auto it_idx = m_field_indices.find(field_name);
  if (it_idx == m_field_indices.end()) {
    return std::nullopt;
  }

  size_t index = it_idx->second;
  size_t offset = 0;
  for (size_t i = 0; i < index; ++i) {
    offset += sizes[i] * counts[i];
  }
  return field_info_t {types[index], sizes[index], counts[index], offset};
}

bool pcd_format_t::pcd_header_t::validate(size_t file_size_bytes) const
{
  // (Implementation is the same as in the previous version's
  // detail_pcd::pcd_header_t::validate)
  // ... check required fields, consistency, binary size ...
  if (version.empty() || fields.empty() || sizes.empty() || types.empty()
      || counts.empty() || width == 0 || height == 0 || points == 0
      || data_type == data_type_t::UNKNOWN)
  {
    LOG_ERROR_S << "PCD header missing required fields.";
    return false;
  }
  if (fields.size() != sizes.size() || fields.size() != types.size()
      || fields.size() != counts.size())
  {
    LOG_ERROR_S << "PCD header FIELDS/SIZE/TYPE/COUNT mismatch.";
    return false;
  }
  if (points != width * height) {
    LOG_ERROR_S << "PCD header POINTS != WIDTH * HEIGHT.";
    return false;
  }
  if (point_step == 0) {
    LOG_ERROR_S
        << "PCD header validation failed: calculated point_step is zero.";
    return false;
  }

  // Validate field types/sizes
  for (size_t i = 0; i < fields.size(); ++i) {
    if ((types[i] != 'F' || (sizes[i] != 4 && sizes[i] != 8))
        && (types[i] != 'I'
            || (sizes[i] != 1 && sizes[i] != 2 && sizes[i] != 4
                && sizes[i] != 8))
        && (types[i] != 'U'
            || (sizes[i] != 1 && sizes[i] != 2 && sizes[i] != 4
                && sizes[i] != 8)))
    {
      LOG_ERROR_S << "PCD header invalid SIZE/TYPE combination for field "
                  << fields[i] << ": TYPE=" << types[i] << " SIZE=" << sizes[i];
      return false;
    }
    if (counts[i] == 0) {
      LOG_ERROR_S << "PCD header COUNT cannot be zero for field " << fields[i];
      return false;
    }
  }

  if (data_type == data_type_t::BINARY
      || data_type == data_type_t::BINARY_COMPRESSED)
  {
    size_t expected_data_size = points * point_step;
    if (header_length == 0) {
      LOG_ERROR_S << "PCD header validation failed: Header length unknown for "
                     "binary size check.";
      return false;  // Need header length to check data size
    }
    size_t actual_data_size = file_size_bytes - header_length;
    if (actual_data_size < expected_data_size) {  // Allow slightly larger files
      LOG_ERROR_S << "PCD binary data size mismatch. Expected >= "
                  << expected_data_size << " bytes, found " << actual_data_size
                  << " bytes (File size: " << file_size_bytes
                  << ", Header size: " << header_length
                  << ", Points: " << points << ", Point Step: " << point_step
                  << ")";
      return false;
    } else if (actual_data_size > expected_data_size * 1.1
               && actual_data_size > expected_data_size + 1024)
    {  // Warn if much larger
      LOG_WARN_S << "PCD binary data size (" << actual_data_size
                 << " bytes) is significantly larger than expected ("
                 << expected_data_size << " bytes). Ignoring extra data.";
    }
  }

  if (data_type == data_type_t::BINARY_COMPRESSED) {
    LOG_WARN_S << "PCD binary_compressed format is not yet fully supported for "
                  "reading.";
    // return false; // Decide if this is a hard error
  }
  return true;
}

// --- pcd_format_t Public Member Functions Implementation ---

bool pcd_format_t::can_read(const std::string& path) const
{
  // Basic check: just the extension
  return toolbox::container::string::ends_with(path, ".pcd");
  // Could add more checks here later (e.g., read first few bytes/lines) if
  // needed
}

std::vector<std::string> pcd_format_t::get_supported_extensions() const
{
  return {".pcd"};
}

bool pcd_format_t::read(const std::string& path,
                        std::unique_ptr<base_file_data_t>& data)
{
  LOG_INFO_S << "pcd_format_t: Attempting to read PCD file: " << path;

  // Default to reading as float
  auto cloud = std::make_unique<point_cloud_t<float>>();
  cloud->clear();  // Ensure it's empty

  std::ifstream file(
      path,
      std::ios::binary);  // Open in binary for consistent offset reading
  if (!file.is_open()) {
    LOG_ERROR_S << "pcd_format_t: Failed to open file: " << path;
    return false;
  }

  pcd_header_t header;
  size_t header_end_pos = 0;
  if (!parse_header_stream(file, header, header_end_pos)) {
    LOG_ERROR_S << "pcd_format_t: Failed to parse header in: " << path;
    return false;
  }
  header.header_length = header_end_pos;
  header.calculate_point_step_and_indices();  // Calculate step and indices map

  // Get file size for validation
  std::error_code ec;
  size_t file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    LOG_ERROR_S << "pcd_format_t: Could not get file size for " << path << ": "
                << ec.message();
    return false;
  }

  if (!header.validate(file_size)) {
    LOG_ERROR_S << "pcd_format_t: Invalid PCD header in file: " << path;
    return false;
  }

  LOG_INFO_S << "pcd_format_t: Header parsed successfully. Points: "
             << header.points << ", Format: "
             << (header.data_type == pcd_header_t::data_type_t::ASCII
                     ? "ASCII"
                     : (header.data_type == pcd_header_t::data_type_t::BINARY
                            ? "BINARY"
                            : "COMPRESSED/UNKNOWN"));

  bool success = false;
  if (header.data_type == pcd_header_t::data_type_t::ASCII) {
    file.seekg(header.header_length);  // Position stream after header
    if (!file) {
      LOG_ERROR_S
          << "pcd_format_t: Failed to seek past header for ASCII reading in: "
          << path;
      return false;
    }
    LOG_DEBUG_S << "pcd_format_t: Reading ASCII data...";
    success = read_ascii_data(file, header, *cloud);
  } else if (header.data_type == pcd_header_t::data_type_t::BINARY) {
    file.close();  // Close ifstream, use mmap
    LOG_DEBUG_S << "pcd_format_t: Reading BINARY data using memory mapping...";
    success = read_binary_data(path, header, *cloud);
  } else if (header.data_type == pcd_header_t::data_type_t::BINARY_COMPRESSED) {
    LOG_ERROR_S << "pcd_format_t: Reading binary_compressed PCD format is not "
                   "supported.";
    success = false;
  } else {
    LOG_ERROR_S << "pcd_format_t: Unknown PCD data type.";
    success = false;
  }

  if (success) {
    LOG_INFO_S << "pcd_format_t: Finished reading PCD file: " << path
               << ", read " << cloud->size() << " points.";
    data = std::move(cloud);  // Transfer ownership to output parameter
    return true;
  }
  LOG_ERROR_S << "pcd_format_t: Failed to read data section from: " << path;
  return false;
}

bool pcd_format_t::write(const std::string& path,
                         const std::unique_ptr<base_file_data_t>& data,
                         bool binary) const
{
  if (!data) {
    LOG_ERROR_S << "pcd_format_t: Cannot write null data.";
    return false;
  }

  // Try casting to supported point cloud types
  if (const auto* cloud_f =
          dynamic_cast<const point_cloud_t<float>*>(data.get()))
  {
    LOG_INFO_S << "pcd_format_t: Writing point_cloud_t<float> to " << path
               << (binary ? " (BINARY)" : " (ASCII)");
    return write_internal(path, *cloud_f, binary);
  }

  if (const auto* cloud_d =
          dynamic_cast<const point_cloud_t<double>*>(data.get()))
  {
    LOG_INFO_S << "pcd_format_t: Writing point_cloud_t<double> to " << path
               << (binary ? " (BINARY)" : " (ASCII)");
    return write_internal(path, *cloud_d, binary);
  }
  LOG_ERROR_S << "pcd_format_t: Input data is not a supported point_cloud_t "
                 "type (float or double).";
  return false;
}

// --- pcd_format_t Private Non-Template Helper Functions Implementation ---

bool pcd_format_t::parse_header_stream(std::istream& stream,
                                       pcd_header_t& header,
                                       size_t& header_end_pos)
{
  std::string line;
  bool data_keyword_found = false;
  header_end_pos = 0;
  size_t current_pos = 0;

  while (std::getline(stream, line)) {
    current_pos =
        static_cast<size_t>(stream.tellg());  // Position after reading the line
    // Handle CRLF before calculating length or parsing
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    if (line.empty() || line[0] == '#') {
      if (current_pos != static_cast<size_t>(-1)) {
        header_end_pos = current_pos;  // Update pos even for comments/empty
      }
      continue;  // Skip empty lines and comments
    }

    if (current_pos != static_cast<size_t>(-1)) {
      header_end_pos = current_pos;
    } else {  // If tellg failed, try calculating manually (less reliable)
      header_end_pos += line.length() + 1;  // +1 for newline
      LOG_WARN_S << "pcd_format_t: tellg failed during header parse, "
                    "estimating position.";
    }

    if (header.parse_line(line))
    {  // parse_line returns true when DATA is found
      data_keyword_found = true;
      break;
    }
  }
  return data_keyword_found;
}

template bool pcd_format_t::read_ascii_data(std::istream& stream,
                                            const pcd_header_t& header,
                                            point_cloud_t<float>& cloud);

template bool pcd_format_t::read_binary_data(const std::string& path,
                                             const pcd_header_t& header,
                                             point_cloud_t<float>& cloud);

template bool pcd_format_t::read_ascii_data(std::istream& stream,
                                            const pcd_header_t& header,
                                            point_cloud_t<double>& cloud);

template bool pcd_format_t::read_binary_data(const std::string& path,
                                             const pcd_header_t& header,
                                             point_cloud_t<double>& cloud);

template bool pcd_format_t::write_internal(const std::string& path,
                                           const point_cloud_t<float>& cloud,
                                           bool binary) const;

template bool pcd_format_t::write_internal(const std::string& path,
                                           const point_cloud_t<double>& cloud,
                                           bool binary) const;

template bool pcd_format_t::parse_ascii_point_line(const std::string& line,
                                                   const pcd_header_t& header,
                                                   point_t<float>& point,
                                                   point_t<float>* normal,
                                                   point_t<float>* color);

template bool pcd_format_t::parse_ascii_point_line(const std::string& line,
                                                   const pcd_header_t& header,
                                                   point_t<double>& point,
                                                   point_t<double>* normal,
                                                   point_t<double>* color);

// Explicit instantiations for standalone helper functions
// Temporarily comment out to check for duplicate instantiation issues
template CPP_TOOLBOX_EXPORT
    std::unique_ptr<toolbox::types::point_cloud_t<float>>
    read_pcd<float>(const std::string& path);
template CPP_TOOLBOX_EXPORT
    std::unique_ptr<toolbox::types::point_cloud_t<double>>
    read_pcd<double>(const std::string& path);

template CPP_TOOLBOX_EXPORT bool write_pcd<float>(
    const std::string& path,
    const toolbox::types::point_cloud_t<float>& cloud,
    bool binary);  // No default argument here
template CPP_TOOLBOX_EXPORT bool write_pcd<double>(
    const std::string& path,
    const toolbox::types::point_cloud_t<double>& cloud,
    bool binary);  // No default argument here

}  // namespace toolbox::io::formats
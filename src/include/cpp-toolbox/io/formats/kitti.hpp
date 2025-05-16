#pragma once

#include <array>
#include <cstdint>  // For fixed-width integers
#include <cstring>  // For memcpy
#include <iosfwd>  // Forward declare std::istream
#include <map>
#include <memory>  // unique_ptr
#include <optional>
#include <string>
#include <vector>

#include <cpp-toolbox/container/string.hpp>  // For string utilities like ends_with
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/file/file.hpp>  // Assuming MemoryMappedFile is here
#include <cpp-toolbox/file/memory_mapped_file.hpp>
#include <cpp-toolbox/io/formats/base.hpp>  // base_file_format_t, base_file_data_t
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/macro.hpp>  // For platform checks if needed directly
#include <cpp-toolbox/types/point.hpp>  // point_cloud_t (inherits base_file_data_t)

namespace toolbox::io
{

// Forward declare the concrete data type we'll work with
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

/**
 * @brief KITTI 点云数据（.bin）文件格式处理器。/File format handler for KITTI
 * Point Cloud Data (.bin) files.
 *
 * 实现了 PCD 文件的 base_file_format_t 接口。/Implements the base_file_format_t
 * interface for PCD files. 支持读取 BINARY 格式。/Supports reading BINARY
 * formats. 支持写入 BINARY
 *
 * @code{.cpp}
 * // 读取 PCD 文件/Reading a PCD file
 * toolbox::io::pcd_format_t kitti_reader;
 * std::unique_ptr<toolbox::io::base_file_data_t> data;
 *
 * if (kitti_reader.can_read("pointcloud.bin")) {
 *     if (kitti_reader.read("pointcloud.bin", data)) {
 *         auto* cloud =
 * dynamic_cast<toolbox::types::point_cloud_t<float>*>(data.get());
 *         if (cloud) {
 *             std::cout << "读取了 " << cloud->size() << " 个点/Read " <<
 * cloud->size() << " points" << std::endl;
 *         }
 *     }
 * }
 *
 * // 写入 PCD 文件/Writing a PCD file
 * toolbox::types::point_cloud_t<float> cloud;
 * // 填充点云数据/Fill point cloud data
 * cloud.push_back({1.0f, 2.0f, 3.0f});
 * cloud.push_back({4.0f, 5.0f, 6.0f});
 *
 * std::unique_ptr<toolbox::io::base_file_data_t> write_data =
 *     std::make_unique<toolbox::types::point_cloud_t<float>>(std::move(cloud));
 *
 * kitti_reader.write("output.bin", write_data, true); // 以二进制格式写入/Write
 * in binary format
 * @endcode
 */
class CPP_TOOLBOX_EXPORT kitti_format_t final : public base_file_format_t
{
public:
  /**
   * @brief 构造函数。/Constructor.
   */
  explicit kitti_format_t() = default;

  /**
   * @brief 析构函数。/Destructor.
   */
  ~kitti_format_t() override = default;

  // 禁用复制，继承移动操作/Disable copy, inherit move operations
  kitti_format_t(const kitti_format_t&) = delete;
  kitti_format_t& operator=(const kitti_format_t&) = delete;
  // kitti_format_t(kitti_format_t&&) = default; // 继承的足够了/Inherited is
  // fine kitti_format_t& operator=(kitti_format_t&&) = default; //
  // 继承的足够了/Inherited is fine

  /**
   * @brief 检查文件扩展名是否为".bin"。/Checks if the file extension is ".bin".
   *
   * @param path 文件路径/Path to the file
   * @return 如果扩展名匹配则为 true，否则为 false/True if the extension
   * matches, false otherwise
   *
   * @code{.cpp}
   * toolbox::io::kitti_format_t kitti_format;
   * if (kitti_format.can_read("cloud.bin")) {
   *     std::cout << "这是一个 KITTI 点云文件/This is a KITTI point cloud file"
   * << std::endl;
   * } else {
   *     std::cout << "这不是一个 KITTI 点云文件/This is not a KITTI point
   * cloud file" << std::endl;
   * }
   * @endcode
   */
  [[nodiscard]] auto can_read(const std::string& path) const -> bool override;

  /**
   * @brief 返回支持的文件扩展名。/Returns the supported file extension.
   *
   * @return 仅包含".bin"的向量/A vector containing only ".bin"
   *
   * @code{.cpp}
   * toolbox::io::kitti_format_t kitti_format;
   * auto extensions = kitti_format.get_supported_extensions();
   * for (const auto& ext : extensions) {
   *     std::cout << "支持的扩展名/Supported extension: " << ext << std::endl;
   * }
   * @endcode
   */
  [[nodiscard]] auto get_supported_extensions() const
      -> std::vector<std::string> override;

  /**
   * @brief 将 KITTI 点云文件（BINARY）读入 point_cloud_t<float>。/Reads a
   * KITTI point cloud file (BINARY) into a point_cloud_t<float>.
   *
   * 选择 T=float 作为常见默认值。/The specific type T=float is chosen as a
   * common default.
   *
   * @param path KITTI 点云文件的路径/Path to the KITTI point cloud file
   * @param data 输出 unique_ptr，成功时将持有创建的 point_cloud_t<float>/Output
   * unique_ptr that will hold the created point_cloud_t<float> on success
   * @return 读取成功则为 true，否则为 false/True on successful read, false
   * otherwise
   *
   * @code{.cpp}
   * toolbox::io::kitti_format_t kitti_format;
   * std::unique_ptr<toolbox::io::base_file_data_t> cloud_data;
   *
   * if (kitti_format.read("scan.bin", cloud_data)) {
   *     // 转换为具体类型/Convert to concrete type
   *     auto* point_cloud =
   * dynamic_cast<toolbox::types::point_cloud_t<float>*>(cloud_data.get()); if
   * (point_cloud) { std::cout << "读取了 " << point_cloud->size() << "
   * 个点/Read "
   *                   << point_cloud->size() << " points" << std::endl;
   *     }
   * } else {
   *     std::cerr << "读取失败/Read failed" << std::endl;
   * }
   * @endcode
   */
  auto read(const std::string& path, std::unique_ptr<base_file_data_t>& data)
      -> bool override;

  /**
   * @brief 将 point_cloud_t<float> 或 point_cloud_t<double> 中的数据写入 KITTI
   * 点云文件。/Writes data from a point_cloud_t<float> or point_cloud_t<double>
   * to a KITTI point cloud file.
   *
   * 格式（BINARY）由 binary 参数决定。/The format (BINARY) is determined by the
   * `binary` parameter.
   *
   * @param path 输出 KITTI 点云文件的路径/Path to the output KITTI point cloud
   * file
   * @param data 输入 unique_ptr，持有 point_cloud_t<float> 或
   * point_cloud_t<double>/Input unique_ptr holding a point_cloud_t<float> or
   * point_cloud_t<double>
   * @param binary 是否以二进制模式写入，总是 true，
   * 因为只支持二进制格式/Whether to write in binary mode(always true, because
   * only binary format is supported)
   * @return 写入成功则为
   * true；如果输入数据为空、不是支持的点云类型或发生文件错误则为 false/True on
   * successful write, false if input data is null, not a supported point cloud
   * type, or if a file error occurs
   *
   * @code{.cpp}
   * // 创建点云/Create a point cloud
   * auto cloud = std::make_unique<toolbox::types::point_cloud_t<float>>();
   *
   * // 添加一些点/Add some points
   * cloud->push_back({1.0f, 2.0f, 3.0f});
   * cloud->push_back({4.0f, 5.0f, 6.0f});
   * cloud->push_back({7.0f, 8.0f, 9.0f});
   *
   * // 转换为基类指针/Convert to base class pointer
   * std::unique_ptr<toolbox::io::base_file_data_t>
   * data_ptr(cloud.release());
   *
   * // 写入文件/Write to file
   * toolbox::io::kitti_format_t kitti_format;
   * bool binary_result = kitti_format.write("cloud_binary.bin", data_ptr,
   * true);
   * // ASCII 格式/ASCII format bool binary_result =
   * pcd_format.write("cloud_binary.pcd", data_ptr, true); // 二进制格式/Binary
   * format
   *
   * std::cout << "二进制写入" << (binary_result ? "成功" : "失败")
   *           << "/Binary write " << (binary_result ? "succeeded" : "failed")
   * << std::endl;
   * @endcode
   */
  [[nodiscard]] auto write(const std::string& path,
                           const std::unique_ptr<base_file_data_t>& data,
                           bool binary) const -> bool override;

private:
  // --- Internal Helper Functions ---

  /**
   * @brief 写入点云到文件的模板化核心逻辑。/Templated core logic for writing a
   * point cloud to a file.
   *
   * @tparam T 坐标类型（如 float, double）/The coordinate type (e.g., float,
   * double)
   * @param path 输出文件路径/Output file path
   * @param cloud 要写入的点云数据/The point cloud data to write
   * @param binary 是否以二进制模式写入/Whether to write in binary mode
   * @return 成功则为 true，失败则为 false/True on success, false on failure
   */
  template<typename T>
  bool write_internal(const std::string& path,
                      const point_cloud_t<T>& cloud,
                      bool binary) const;

  /**
   * @brief 使用内存映射将 BINARY 数据部分读入点云。/Reads BINARY data section
   * using memory mapping into the point cloud.
   *
   * @tparam T 点云坐标类型/Cloud coordinate type
   * @param path 文件路径（内存映射需要）/File path (needed for memory mapping)
   * @param cloud 输出点云对象/Output point cloud object
   * @return 成功则为 true，失败则为 false/True on success, false on failure
   */
  template<typename T>
  static bool read_binary_data(const std::string& path,
                               point_cloud_t<T>& cloud);

};  // class pcd_format_t

/**
 * @brief 从文件中读取 KITTI 点云数据的独立函数。/Standalone function to read
 * KITTI point cloud data from a file.
 *
 * @tparam T 点云数据的存储类型（如 float 或 double）/Storage type for the point
 * cloud data (e.g., float or double)
 * @param path KITTI 点云文件的路径/Path to the KITTI point cloud file
 * @return 包含读取点云的唯一指针，如果读取失败则返回 nullptr/Unique pointer
 * containing the read point cloud, or nullptr if reading fails
 *
 * @code
 * // 读取 float 类型的点云/Read a point cloud with float precision
 * auto cloud_float = toolbox::io::read_kitti_bin<float>("cloud.bin");
 * if (cloud_float) {
 *   std::cout << "读取了 " << cloud_float->size() << " 个点/Read " <<
 * cloud_float->size() << " points" << std::endl;
 * }
 *
 * // 读取 double 类型的点云/Read a point cloud with double precision
 * auto cloud_double =
 * toolbox::io::read_kitti_bin<double>("cloud.bin");
 * @endcode
 */
template<typename T>
CPP_TOOLBOX_EXPORT std::unique_ptr<toolbox::types::point_cloud_t<T>>
read_kitti_bin(const std::string& path);

/**
 * @brief 将点云数据写入 KITTI 点云文件的独立函数。/Standalone function to write
 * point cloud data to a KITTI point cloud file.
 *
 * @tparam T 点云数据的存储类型（如 float 或 double）/Storage type for the point
 * cloud data (e.g., float or double)
 * @param path 要写入的 KITTI 点云文件路径/Path to the KITTI point cloud file to
 * write
 * @param cloud 要保存的点云数据/The point cloud data to save
 * @return 写入成功返回 true，失败返回 false/Returns true if writing was
 * successful, false otherwise
 *
 * @code
 * // 创建一个点云并添加一些点/Create a point cloud and add some points
 * toolbox::types::point_cloud_t<float> cloud;
 * cloud.points = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
 *
 * // 以二进制格式保存/Save in binary format
 * bool binary_success =
 * toolbox::io::write_kitti_bin("cloud_binary.bin", cloud);
 * @endcode
 */
template<typename T>
CPP_TOOLBOX_EXPORT bool write_kitti_bin(
    const std::string& path, const toolbox::types::point_cloud_t<T>& cloud);

}  // namespace toolbox::io

#include <cpp-toolbox/io/formats/detail/kitti_impl.hpp>
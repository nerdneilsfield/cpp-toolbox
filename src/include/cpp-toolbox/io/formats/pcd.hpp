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

namespace toolbox::io::formats
{

// Forward declare the concrete data type we'll work with
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;  // Assuming point_t is also in types namespace

/**
 * @brief 点云数据（.pcd）文件格式处理器。/File format handler for Point Cloud
 * Data (.pcd) files.
 *
 * 实现了 PCD 文件的 base_file_format_t 接口。/Implements the base_file_format_t
 * interface for PCD files. 支持读取 ASCII 和 BINARY 格式。/Supports reading
 * ASCII and BINARY formats. 支持写入 ASCII 和 BINARY
 * 格式（可通过构造函数配置）。/Supports writing ASCII and BINARY formats
 * (configurable via constructor). 使用内存映射加速 BINARY 文件的读取。/Uses
 * memory mapping for accelerated reading of BINARY files. 默认将数据读入
 * point_cloud_t<float>。/Reads data into point_cloud_t<float> by default.
 * 可以写入 point_cloud_t<float> 或 point_cloud_t<double>。/Can write
 * point_cloud_t<float> or point_cloud_t<double>.
 *
 * @code{.cpp}
 * // 读取 PCD 文件/Reading a PCD file
 * toolbox::io::formats::pcd_format_t pcd_reader;
 * std::unique_ptr<toolbox::io::formats::base_file_data_t> data;
 *
 * if (pcd_reader.can_read("pointcloud.pcd")) {
 *     if (pcd_reader.read("pointcloud.pcd", data)) {
 *         auto* cloud =
 * dynamic_cast<toolbox::types::point_cloud_t<float>*>(data.get()); if (cloud) {
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
 * std::unique_ptr<toolbox::io::formats::base_file_data_t> write_data =
 *     std::make_unique<toolbox::types::point_cloud_t<float>>(std::move(cloud));
 *
 * pcd_reader.write("output.pcd", write_data, true); // 以二进制格式写入/Write
 * in binary format
 * @endcode
 */
class CPP_TOOLBOX_EXPORT pcd_format_t final : public base_file_format_t
{
private:
  // --- Internal Helper Structs/Enums ---

  /**
   * @brief 用于存储解析后的头信息的结构体。/Structure to hold parsed header
   * information.
   */
  struct pcd_header_t
  {
    std::string version; /**< PCD 版本号/PCD version number */
    std::vector<std::string> fields; /**< 字段名称列表（如 x, y, z, rgb）/List
                                        of field names (e.g., x, y, z, rgb) */
    std::vector<char>
        types; /**< 字段类型（'F'=浮点, 'I'=有符号整数, 'U'=无符号整数）/Field
                  types ('F'=float, 'I'=signed int, 'U'=unsigned int) */
    std::vector<size_t> sizes; /**< 每个字段的字节大小（1, 2, 4, 8）/Size in
                                  bytes for each field (1, 2, 4, 8) */
    std::vector<size_t> counts; /**< 每个字段的元素数量（通常为1）/Number of
                                   elements per field (usually 1) */
    size_t width =
        0; /**< 点云宽度（有序点云为列数，无序点云为点数）/Point cloud width
              (columns for organized clouds, total points for unorganized) */
    size_t height =
        0; /**< 点云高度（有序点云为行数，无序点云为1）/Point cloud height (rows
              for organized clouds, 1 for unorganized) */
    std::array<double, 7> viewpoint {
        /**< 视点参数 [tx ty tz qw qx qy qz]/Viewpoint parameters [tx ty tz qw
           qx qy qz] */
        0,
        0,
        0,
        1,
        0,
        0,
        0};
    size_t points = 0; /**< 点的总数（width * height）/Total number of points
                          (width * height) */
    /**
     * @brief 数据类型枚举。/Data type enumeration.
     */
    enum class data_type_t : uint8_t
    {
      ASCII, /**< ASCII 文本格式/ASCII text format */
      BINARY, /**< 二进制格式/Binary format */
      BINARY_COMPRESSED, /**< 压缩二进制格式/Compressed binary format */
      UNKNOWN /**< 未知格式/Unknown format */
    } data_type = data_type_t::UNKNOWN; /**< 数据存储类型/Data storage type */
    size_t point_step = 0; /**< 每个点的字节大小（计算得出）/Calculated size of
                              one point in bytes */
    size_t header_length = 0; /**< 头部的字节大小/Size of header in bytes */

    /**
     * @brief 辅助映射：字段名称 -> 在 fields/types/sizes/counts
     * 向量中的索引。/Helper map: field name -> index in
     * fields/types/sizes/counts vectors.
     */
    std::map<std::string, size_t> m_field_indices;

    /**
     * @brief 包含偏移量的字段详细信息结构体。/Helper struct for field details
     * including offset.
     */
    struct field_info_t
    {
      char type; /**< 字段类型（'F', 'I', 'U'）/Field type ('F', 'I', 'U') */
      size_t size; /**< 字段大小（字节）/Field size in bytes */
      size_t count; /**< 元素数量/Element count */
      size_t offset; /**< 在点数据中的字节偏移量/Byte offset in point data */
    };

    /**
     * @brief 解析头部的一行。如果找到 DATA 行则返回 true。/Parses one line of
     * the header. Returns true if DATA line is found.
     *
     * @param line 要解析的行/The line to parse
     * @return 如果找到 DATA 行则为 true，否则为 false/true if DATA line is
     * found, false otherwise
     */
    bool parse_line(const std::string& line);

    /**
     * @brief 计算 point_step 并根据解析的 fields/sizes/counts 填充
     * m_field_indices。/Calculates point_step and populates m_field_indices
     * based on parsed fields/sizes/counts.
     */
    void calculate_point_step_and_indices();

    /**
     * @brief 获取特定字段的详细信息。/Retrieves detailed info for a specific
     * field.
     *
     * @param field_name 字段名称/Field name
     * @return 如果找到字段则返回字段信息，否则返回空/Field info if found, empty
     * optional otherwise
     */
    [[nodiscard]] std::optional<field_info_t> get_field_info(
        const std::string& field_name) const;

    /**
     * @brief 验证头部一致性并检查二进制大小与文件大小。/Validates the header
     * consistency and checks binary size against file size.
     *
     * @param file_size_bytes 文件大小（字节）/File size in bytes
     * @return 如果头部有效则为 true，否则为 false/true if header is valid,
     * false otherwise
     */
    [[nodiscard]] bool validate(size_t file_size_bytes) const;
  };

public:
  /**
   * @brief 构造函数。/Constructor.
   */
  explicit pcd_format_t() = default;

  /**
   * @brief 析构函数。/Destructor.
   */
  ~pcd_format_t() override = default;

  // 禁用复制，继承移动操作/Disable copy, inherit move operations
  pcd_format_t(const pcd_format_t&) = delete;
  pcd_format_t& operator=(const pcd_format_t&) = delete;
  // pcd_format_t(pcd_format_t&&) = default; // 继承的足够了/Inherited is fine
  // pcd_format_t& operator=(pcd_format_t&&) = default; //
  // 继承的足够了/Inherited is fine

  /**
   * @brief 检查文件扩展名是否为".pcd"。/Checks if the file extension is ".pcd".
   *
   * @param path 文件路径/Path to the file
   * @return 如果扩展名匹配则为 true，否则为 false/True if the extension
   * matches, false otherwise
   *
   * @code{.cpp}
   * toolbox::io::formats::pcd_format_t pcd_format;
   * if (pcd_format.can_read("cloud.pcd")) {
   *     std::cout << "这是一个 PCD 文件/This is a PCD file" << std::endl;
   * } else {
   *     std::cout << "这不是一个 PCD 文件/This is not a PCD file" << std::endl;
   * }
   * @endcode
   */
  [[nodiscard]] auto can_read(const std::string& path) const -> bool override;

  /**
   * @brief 返回支持的文件扩展名。/Returns the supported file extension.
   *
   * @return 仅包含".pcd"的向量/A vector containing only ".pcd"
   *
   * @code{.cpp}
   * toolbox::io::formats::pcd_format_t pcd_format;
   * auto extensions = pcd_format.get_supported_extensions();
   * for (const auto& ext : extensions) {
   *     std::cout << "支持的扩展名/Supported extension: " << ext << std::endl;
   * }
   * @endcode
   */
  [[nodiscard]] auto get_supported_extensions() const
      -> std::vector<std::string> override;

  /**
   * @brief 将 PCD 文件（ASCII 或 BINARY）读入 point_cloud_t<float>。/Reads a
   * PCD file (ASCII or BINARY) into a point_cloud_t<float>.
   *
   * 选择 T=float 作为常见默认值。/The specific type T=float is chosen as a
   * common default.
   *
   * @param path PCD 文件的路径/Path to the PCD file
   * @param data 输出 unique_ptr，成功时将持有创建的 point_cloud_t<float>/Output
   * unique_ptr that will hold the created point_cloud_t<float> on success
   * @return 读取成功则为 true，否则为 false/True on successful read, false
   * otherwise
   *
   * @code{.cpp}
   * toolbox::io::formats::pcd_format_t pcd_format;
   * std::unique_ptr<toolbox::io::formats::base_file_data_t> cloud_data;
   *
   * if (pcd_format.read("scan.pcd", cloud_data)) {
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
   * @brief 将 point_cloud_t<float> 或 point_cloud_t<double> 中的数据写入 PCD
   * 文件。/Writes data from a point_cloud_t<float> or point_cloud_t<double> to
   * a PCD file.
   *
   * 格式（ASCII 或 BINARY）由 binary 参数决定。/The format (ASCII or BINARY) is
   * determined by the `binary` parameter.
   *
   * @param path 输出 PCD 文件的路径/Path to the output PCD file
   * @param data 输入 unique_ptr，持有 point_cloud_t<float> 或
   * point_cloud_t<double>/Input unique_ptr holding a point_cloud_t<float> or
   * point_cloud_t<double>
   * @param binary 是否以二进制模式写入/Whether to write in binary mode
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
   * std::unique_ptr<toolbox::io::formats::base_file_data_t>
   * data_ptr(cloud.release());
   *
   * // 写入文件/Write to file
   * toolbox::io::formats::pcd_format_t pcd_format;
   * bool ascii_result = pcd_format.write("cloud_ascii.pcd", data_ptr, false);
   * // ASCII 格式/ASCII format bool binary_result =
   * pcd_format.write("cloud_binary.pcd", data_ptr, true); // 二进制格式/Binary
   * format
   *
   * std::cout << "ASCII 写入" << (ascii_result ? "成功" : "失败")
   *           << "/ASCII write " << (ascii_result ? "succeeded" : "failed") <<
   * std::endl; std::cout << "二进制写入" << (binary_result ? "成功" : "失败")
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
   * @brief 解析 PCD 文件流的头部部分。/Parses the header section of a PCD file
   * stream.
   *
   * @param stream 输入文件流/Input file stream
   * @param header 要填充的 pcd_header_t 对象/Output pcd_header_t object to
   * populate
   * @param header_end_pos 输出头部之后的流位置（DATA 行之后）/Output stream
   * position after the header (after DATA line)
   * @return 如果头部解析成功直到 DATA 行则为 true，否则为 false/True if header
   * parsing is successful up to the DATA line, false otherwise
   */
  static bool parse_header_stream(std::istream& stream,
                                  pcd_header_t& header,
                                  size_t& header_end_pos);

  /**
   * @brief 将 ASCII 数据部分读入点云。/Reads ASCII data section into the point
   * cloud.
   *
   * @tparam T 点云坐标类型/Cloud coordinate type
   * @param stream 定位在 ASCII 数据开始处的输入流/Input stream positioned at
   * the start of ASCII data
   * @param header 解析的头部信息/Parsed header information
   * @param cloud 输出点云对象/Output point cloud object
   * @return 成功则为 true，失败则为 false/True on success, false on failure
   */
  template<typename T>
  static bool read_ascii_data(std::istream& stream,
                              const pcd_header_t& header,
                              point_cloud_t<T>& cloud);

  /**
   * @brief 使用内存映射将 BINARY 数据部分读入点云。/Reads BINARY data section
   * using memory mapping into the point cloud.
   *
   * @tparam T 点云坐标类型/Cloud coordinate type
   * @param path 文件路径（内存映射需要）/File path (needed for memory mapping)
   * @param header 解析的头部信息（包含 header_length）/Parsed header
   * information (contains header_length)
   * @param cloud 输出点云对象/Output point cloud object
   * @return 成功则为 true，失败则为 false/True on success, false on failure
   */
  template<typename T>
  static bool read_binary_data(const std::string& path,
                               const pcd_header_t& header,
                               point_cloud_t<T>& cloud);

  /**
   * @brief 解析单行 ASCII 点数据的辅助函数。/Helper to parse a single line of
   * ASCII point data.
   *
   * @tparam PointStorageT 点云中使用的类型（如 float）/The type used in
   * point_t/point_cloud_t (e.g., float)
   * @param line 包含点数据的字符串/The string containing the point data
   * @param header 解析的头部，用于了解字段顺序/类型/Parsed header to know field
   * order/types
   * @param point 输出点坐标/Output point coordinates
   * @param normal 可选的输出法线坐标/Optional output normal coordinates
   * @param color 可选的输出颜色坐标/Optional output color coordinates
   * @return 如果解析成功则为 true，否则为 false/True if parsing is successful,
   * false otherwise
   */
  template<typename PointStorageT>
  static bool parse_ascii_point_line(const std::string& line,
                                     const pcd_header_t& header,
                                     point_t<PointStorageT>& point,
                                     point_t<PointStorageT>* normal = nullptr,
                                     point_t<PointStorageT>* color = nullptr);

  /**
   * @brief
   * 从特定偏移量的二进制缓冲区读取类型化值的辅助函数。假设小端字节序。/Helper
   * to read a typed value from a binary buffer at a specific offset. Assumes
   * little-endian byte order.
   *
   * @tparam DestT C++ 代码中的目标类型（如 float, uint8_t）/The destination
   * type in the C++ code (e.g., float, uint8_t)
   * @tparam PCDT PCD 文件中表示的类型（如 float, double, int32_t, uint8_t）/The
   * type as represented in the PCD file (e.g. float, double, int32_t, uint8_t)
   * @param point_ptr 指向缓冲区中点数据开始的指针/Pointer to the start of the
   * point's data in the buffer
   * @param offset 点数据内字段的偏移量/Offset of the field within the point's
   * data
   * @param out_val 存储读取值的输出引用/Output reference to store the read
   * value
   * @return 成功则为 true，如果指针为空则为 false/True on success, false if
   * pointer is null
   */
  template<typename DestT, typename PCDT>
  static bool read_binary_field_value(const unsigned char* point_ptr,
                                      size_t offset,
                                      DestT& out_val);

};  // class pcd_format_t

}  // namespace toolbox::io::formats

#include <cpp-toolbox/io/formats/detail/pcd_impl.hpp>
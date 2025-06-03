#pragma once

#include <unordered_map>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>

namespace toolbox::pcl
{

/**
 * @brief Super4PCS 粗配准算法 / Super4PCS coarse registration algorithm
 *
 * Super4PCS是4PCS的改进版本，通过智能索引技术将时间复杂度从O(n²)降低到O(n)。
 * 主要改进包括：
 * - 基于网格的空间索引，实现快速点对查找
 * - 优化的匹配策略，减少不必要的计算
 * - 更好的可扩展性，能处理百万级点云
 * 
 * Super4PCS is an improved version of 4PCS that reduces time complexity from O(n²) to O(n)
 * through smart indexing. Key improvements include:
 * - Grid-based spatial indexing for fast pair extraction
 * - Optimized matching strategy to reduce unnecessary computations
 * - Better scalability for million-point clouds
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 *
 * @code
 * // 使用示例 / Usage example
 * super_four_pcs_registration_t<float> super4pcs;
 * super4pcs.set_source(source_cloud);
 * super4pcs.set_target(target_cloud);
 * super4pcs.set_delta(0.01f);           // 1cm精度
 * super4pcs.set_overlap(0.3f);          // 30%重叠（Super4PCS可处理更低重叠）
 * super4pcs.enable_smart_indexing(true); // 启用智能索引
 *
 * registration_result_t<float> result;
 * if (super4pcs.align(result)) {
 *     std::cout << "Super4PCS registration successful!" << std::endl;
 * }
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT super_four_pcs_registration_t
    : public four_pcs_registration_t<DataType>
{
  // Allow base classes to access protected members
  friend class base_coarse_registration_t<super_four_pcs_registration_t<DataType>, DataType>;
  friend class four_pcs_registration_t<DataType>;
public:
  using base_type = four_pcs_registration_t<DataType>;
  using typename base_type::base_4pcs_t;
  using typename base_type::candidate_t;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::result_type;
  using typename base_type::transformation_t;
  using typename base_type::vector3_t;
  using typename base_type::matrix3_t;

  /**
   * @brief 点对结构 / Point pair structure
   */
  struct point_pair_t
  {
    std::size_t idx1;      ///< 第一个点的索引 / First point index
    std::size_t idx2;      ///< 第二个点的索引 / Second point index
    DataType distance;     ///< 点对之间的距离 / Distance between points
  };

  /**
   * @brief 3D网格单元 / 3D grid cell
   */
  struct grid_cell_t
  {
    std::vector<std::size_t> point_indices;  ///< 单元内的点索引 / Point indices in cell
  };

  /**
   * @brief 智能索引结构 / Smart index structure
   */
  class smart_index_t
  {
  public:
    smart_index_t(DataType cell_size) : m_cell_size(cell_size) {}

    /**
     * @brief 构建索引 / Build index
     * @param cloud 点云 / Point cloud
     * @param indices 要索引的点 / Points to index
     */
    void build(const point_cloud_ptr& cloud, const std::vector<std::size_t>& indices);

    /**
     * @brief 查找指定距离范围内的所有点对 / Find all point pairs within distance range
     * @param distance 目标距离 / Target distance
     * @param epsilon 距离容差 / Distance tolerance
     * @param max_pairs 最大点对数量 / Maximum number of pairs
     * @return 点对列表 / List of point pairs
     */
    [[nodiscard]] std::vector<point_pair_t> find_pairs_in_range(
        DataType distance, DataType epsilon, std::size_t max_pairs = 1000) const;

    /**
     * @brief 获取网格统计信息 / Get grid statistics
     * @return 统计信息字符串 / Statistics string
     */
    [[nodiscard]] std::string get_statistics() const;

  private:
    /**
     * @brief 计算点的网格键 / Compute grid key for point
     * @param pt 点坐标 / Point coordinates
     * @return 网格键 / Grid key
     */
    [[nodiscard]] std::tuple<int, int, int> compute_grid_key(const vector3_t& pt) const;

    /**
     * @brief 获取邻域网格单元 / Get neighborhood grid cells
     * @param center_key 中心网格键 / Center grid key
     * @param radius 搜索半径（网格单元数） / Search radius (in grid cells)
     * @return 邻域网格键列表 / List of neighborhood grid keys
     */
    [[nodiscard]] std::vector<std::tuple<int, int, int>> get_neighbor_cells(
        const std::tuple<int, int, int>& center_key, int radius) const;

    DataType m_cell_size;  ///< 网格单元大小 / Grid cell size
    std::map<std::tuple<int, int, int>, grid_cell_t> m_grid;  ///< 3D网格 / 3D grid
    point_cloud_ptr m_cloud;  ///< 索引的点云 / Indexed point cloud
    vector3_t m_min_bound;    ///< 最小边界 / Minimum bound
    vector3_t m_max_bound;    ///< 最大边界 / Maximum bound
  };

  super_four_pcs_registration_t() = default;
  ~super_four_pcs_registration_t() = default;

  /**
   * @brief 设置网格分辨率 / Set grid resolution
   * @param resolution 网格单元大小（0表示自动计算） / Grid cell size (0 for auto)
   */
  void set_grid_resolution(DataType resolution) { m_grid_resolution = resolution; }

  /**
   * @brief 获取网格分辨率 / Get grid resolution
   * @return 网格分辨率 / Grid resolution
   */
  [[nodiscard]] DataType get_grid_resolution() const { return m_grid_resolution; }

  /**
   * @brief 启用/禁用智能索引 / Enable/disable smart indexing
   * @param enable true启用，false禁用 / true to enable, false to disable
   */
  void enable_smart_indexing(bool enable) { m_use_smart_indexing = enable; }

  /**
   * @brief 获取是否使用智能索引 / Get whether using smart indexing
   * @return 是否使用智能索引 / Whether using smart indexing
   */
  [[nodiscard]] bool is_smart_indexing_enabled() const { return m_use_smart_indexing; }

  /**
   * @brief 设置点对距离容差 / Set pair distance tolerance
   * @param epsilon 距离容差 / Distance tolerance
   */
  void set_pair_distance_epsilon(DataType epsilon) { m_pair_distance_epsilon = epsilon; }

  /**
   * @brief 获取点对距离容差 / Get pair distance tolerance
   * @return 距离容差 / Distance tolerance
   */
  [[nodiscard]] DataType get_pair_distance_epsilon() const { return m_pair_distance_epsilon; }

protected:
  /**
   * @brief 获取算法名称实现 / Get algorithm name implementation
   * @return 算法名称 / Algorithm name
   */
  [[nodiscard]] std::string get_algorithm_name_impl() const { return "Super4PCS"; }

  /**
   * @brief 派生类实现的配准算法 / Registration algorithm implementation
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align_impl(result_type& result);

  /**
   * @brief 寻找匹配的4点集（优化版本） / Find congruent sets (optimized version)
   * @param source_base 源基 / Source base
   * @return 匹配的基列表 / List of congruent bases
   */
  [[nodiscard]] std::vector<base_4pcs_t> find_congruent_sets_optimized(
      const base_4pcs_t& source_base) const;

private:
  /**
   * @brief 使用智能索引提取点对 / Extract point pairs using smart indexing
   * @param indices 点索引 / Point indices
   * @param cloud 点云 / Point cloud
   * @param distance 目标距离 / Target distance
   * @param epsilon 距离容差 / Distance tolerance
   * @return 点对列表 / List of point pairs
   */
  [[nodiscard]] std::vector<point_pair_t> extract_pairs_smart(
      const std::vector<std::size_t>& indices, const point_cloud_ptr& cloud,
      DataType distance, DataType epsilon) const;

  /**
   * @brief 从点对构建4点基 / Build 4-point base from point pairs
   * @param pair1 第一个点对 / First point pair
   * @param pair2 第二个点对 / Second point pair
   * @param cloud 点云 / Point cloud
   * @return 4点基（如果有效） / 4-point base (if valid)
   */
  [[nodiscard]] std::optional<base_4pcs_t> build_base_from_pairs(
      const point_pair_t& pair1, const point_pair_t& pair2,
      const point_cloud_ptr& cloud) const;

  /**
   * @brief 验证点对是否可以形成有效的基 / Verify if point pairs can form valid base
   * @param pair1 第一个点对 / First point pair
   * @param pair2 第二个点对 / Second point pair
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool verify_pair_compatibility(const point_pair_t& pair1,
                                                const point_pair_t& pair2) const;

  /**
   * @brief 计算自适应网格分辨率 / Compute adaptive grid resolution
   * @param cloud 点云 / Point cloud
   * @param indices 点索引 / Point indices
   * @return 网格分辨率 / Grid resolution
   */
  [[nodiscard]] DataType compute_adaptive_grid_resolution(
      const point_cloud_ptr& cloud, const std::vector<std::size_t>& indices) const;

  // Super4PCS特定参数 / Super4PCS-specific parameters
  DataType m_grid_resolution = 0;  ///< 网格分辨率（0表示自动） / Grid resolution (0 for auto)
  bool m_use_smart_indexing = true;  ///< 是否使用智能索引 / Whether to use smart indexing
  DataType m_pair_distance_epsilon = static_cast<DataType>(0.01);  ///< 点对距离容差 / Pair distance tolerance
  
  // 内部数据结构 / Internal data structures
  mutable std::unique_ptr<smart_index_t> m_source_index;  ///< 源点云索引 / Source cloud index
  mutable std::unique_ptr<smart_index_t> m_target_index;  ///< 目标点云索引 / Target cloud index
};

}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/super_four_pcs_registration_impl.hpp>
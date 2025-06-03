#pragma once

#include <array>
#include <unordered_map>
#include <unordered_set>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/registration/base_coarse_registration.hpp>

namespace toolbox::pcl
{

/**
 * @brief 4PCS（4-Point Congruent Sets）粗配准算法 / 4PCS coarse registration algorithm
 *
 * 4PCS是一种鲁棒的点云配准算法，通过寻找两个点云中的共面4点集合来估计刚体变换。
 * 该算法对噪声和部分重叠具有良好的鲁棒性。
 * 4PCS is a robust point cloud registration algorithm that estimates rigid transformation
 * by finding coplanar 4-point sets in two point clouds. It is robust to noise and partial overlap.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 *
 * @code
 * // 使用示例 / Usage example
 * four_pcs_registration_t<float> fourpcs;
 * fourpcs.set_source(source_cloud);
 * fourpcs.set_target(target_cloud);
 * fourpcs.set_delta(0.01f);          // 配准精度 1cm
 * fourpcs.set_overlap(0.4f);         // 40%重叠
 * fourpcs.set_sample_size(200);      // 采样200个点
 *
 * registration_result_t<float> result;
 * if (fourpcs.align(result)) {
 *     std::cout << "Registration successful!" << std::endl;
 * }
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT four_pcs_registration_t
    : public base_coarse_registration_t<four_pcs_registration_t<DataType>, DataType>
{
  // Allow base class to access protected members
  friend class base_coarse_registration_t<four_pcs_registration_t<DataType>, DataType>;
public:
  using base_type = base_coarse_registration_t<four_pcs_registration_t<DataType>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::result_type;
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;
  using vector3_t = Eigen::Matrix<DataType, 3, 1>;
  using matrix3_t = Eigen::Matrix<DataType, 3, 3>;
  using kdtree_t = toolbox::pcl::kdtree_t<DataType>;

  /**
   * @brief 4点基结构 / 4-point base structure
   */
  struct base_4pcs_t
  {
    std::array<std::size_t, 4> indices;  ///< 点索引 / Point indices
    std::array<vector3_t, 4> points;     ///< 点坐标 / Point coordinates
    DataType invariant1;  ///< 第一个仿射不变量 / First affine invariant
    DataType invariant2;  ///< 第二个仿射不变量 / Second affine invariant
    vector3_t normal;     ///< 平面法向量 / Plane normal
    DataType d;           ///< 平面方程参数 / Plane equation parameter
  };

  /**
   * @brief 候选匹配结构 / Candidate match structure
   */
  struct candidate_t
  {
    base_4pcs_t source_base;   ///< 源点云基 / Source base
    base_4pcs_t target_base;   ///< 目标点云基 / Target base
    transformation_t transform;  ///< 估计的变换 / Estimated transformation
    DataType lcp_score;        ///< LCP（最大公共点集）评分 / LCP score
    std::size_t num_inliers;   ///< 内点数量 / Number of inliers
  };

  four_pcs_registration_t() = default;
  ~four_pcs_registration_t() = default;

  /**
   * @brief 设置配准精度delta / Set registration accuracy delta
   * @param delta 期望的配准精度 / Desired registration accuracy
   */
  void set_delta(DataType delta) { m_delta = std::abs(delta); }

  /**
   * @brief 获取配准精度 / Get registration accuracy
   * @return 配准精度 / Registration accuracy
   */
  [[nodiscard]] DataType get_delta() const { return m_delta; }

  /**
   * @brief 设置重叠率估计 / Set overlap ratio estimate
   * @param overlap 重叠率（0到1之间） / Overlap ratio (between 0 and 1)
   */
  void set_overlap(DataType overlap)
  {
    m_overlap = std::clamp(overlap, static_cast<DataType>(0.0), static_cast<DataType>(1.0));
  }

  /**
   * @brief 获取重叠率 / Get overlap ratio
   * @return 重叠率 / Overlap ratio
   */
  [[nodiscard]] DataType get_overlap() const { return m_overlap; }

  /**
   * @brief 设置采样点数 / Set number of points to sample
   * @param size 采样点数 / Number of points to sample
   */
  void set_sample_size(std::size_t size) { m_sample_size = size; }

  /**
   * @brief 获取采样点数 / Get sample size
   * @return 采样点数 / Sample size
   */
  [[nodiscard]] std::size_t get_sample_size() const { return m_sample_size; }

  /**
   * @brief 设置是否使用法向量信息 / Set whether to use normal information
   * @param use_normals true使用法向量，false不使用 / true to use normals, false otherwise
   */
  void set_use_normals(bool use_normals) { m_use_normals = use_normals; }

  /**
   * @brief 获取是否使用法向量 / Get whether using normals
   * @return 是否使用法向量 / Whether using normals
   */
  [[nodiscard]] bool get_use_normals() const { return m_use_normals; }

  /**
   * @brief 设置最大法向量偏差角度 / Set maximum normal deviation angle
   * @param angle 最大角度（弧度） / Maximum angle (radians)
   */
  void set_max_normal_angle(DataType angle) { m_max_normal_angle = std::abs(angle); }

  /**
   * @brief 获取最大法向量偏差角度 / Get maximum normal deviation angle
   * @return 最大角度 / Maximum angle
   */
  [[nodiscard]] DataType get_max_normal_angle() const { return m_max_normal_angle; }

  /**
   * @brief 设置基的数量 / Set number of bases to try
   * @param num_bases 基的数量 / Number of bases
   */
  void set_num_bases(std::size_t num_bases) { m_num_bases = num_bases; }

  /**
   * @brief 获取基的数量 / Get number of bases
   * @return 基的数量 / Number of bases
   */
  [[nodiscard]] std::size_t get_num_bases() const { return m_num_bases; }

  // Methods needed by base class (must be accessible via friend)
protected:
  /**
   * @brief 派生类实现的配准算法 / Registration algorithm implementation
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align_impl(result_type& result);

  /**
   * @brief 获取算法名称实现 / Get algorithm name implementation
   * @return 算法名称 / Algorithm name
   */
  [[nodiscard]] std::string get_algorithm_name_impl() const { return "4PCS"; }

  /**
   * @brief 额外的输入验证 / Additional input validation
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input_impl() const;

  /**
   * @brief 钩子函数：设置源点云时的处理 / Hook: processing when setting source cloud
   */
  void set_source_impl(const point_cloud_ptr& source);

  /**
   * @brief 钩子函数：设置目标点云时的处理 / Hook: processing when setting target cloud
   */
  void set_target_impl(const point_cloud_ptr& target);

  /**
   * @brief 获取受保护的成员 / Get protected members
   */
  [[nodiscard]] const std::shared_ptr<kdtree_t>& get_target_kdtree() const { return m_target_kdtree; }
  [[nodiscard]] const std::vector<std::size_t>& get_source_samples() const { return m_source_samples; }
  [[nodiscard]] const std::vector<std::size_t>& get_target_samples() const { return m_target_samples; }

protected:
  /**
   * @brief 采样点云 / Sample point cloud
   * @param cloud 输入点云 / Input point cloud
   * @param num_samples 采样数量 / Number of samples
   * @param indices [out] 采样的索引 / Sampled indices
   */
  void sample_points(const point_cloud_ptr& cloud, std::size_t num_samples,
                     std::vector<std::size_t>& indices);
  
  /**
   * @brief 检查4个点是否共面 / Check if 4 points are coplanar
   * @param points 4个点 / 4 points
   * @param tolerance 容差 / Tolerance
   * @return 是否共面 / Whether coplanar
   */
  [[nodiscard]] bool are_coplanar(const std::array<vector3_t, 4>& points,
                                   DataType tolerance) const;

  /**
   * @brief 计算4点基的仿射不变量 / Compute affine invariants for 4-point base
   * @param base [in/out] 4点基 / 4-point base
   */
  void compute_invariants(base_4pcs_t& base) const;

  /**
   * @brief 估计两个4点基之间的变换 / Estimate transformation between two 4-point bases
   * @param source_base 源基 / Source base
   * @param target_base 目标基 / Target base
   * @return 变换矩阵 / Transformation matrix
   */
  [[nodiscard]] transformation_t estimate_transformation(const base_4pcs_t& source_base,
                                                         const base_4pcs_t& target_base) const;

  /**
   * @brief 计算LCP（最大公共点集）评分 / Compute LCP (Largest Common Pointset) score
   * @param transform 变换矩阵 / Transformation matrix
   * @param inliers [out] 内点索引 / Inlier indices
   * @return LCP评分 / LCP score
   */
  [[nodiscard]] DataType compute_lcp_score(const transformation_t& transform,
                                            std::vector<std::size_t>& inliers) const;

  /**
   * @brief 精炼候选变换 / Refine candidate transformation
   * @param candidate [in/out] 候选匹配 / Candidate match
   */
  void refine_candidate(candidate_t& candidate) const;

  /**
   * @brief 提取共面4点基 / Extract coplanar 4-point bases
   * @param indices 点索引 / Point indices
   * @param cloud 点云 / Point cloud
   * @param num_bases 要提取的基数量 / Number of bases to extract
   * @return 4点基列表 / List of 4-point bases
   */
  [[nodiscard]] std::vector<base_4pcs_t> extract_coplanar_bases(
      const std::vector<std::size_t>& indices, const point_cloud_ptr& cloud,
      std::size_t num_bases) const;

  /**
   * @brief 寻找匹配的4点集 / Find congruent 4-point sets
   * @param source_base 源基 / Source base
   * @param target_indices 目标点索引 / Target point indices
   * @param target_cloud 目标点云 / Target point cloud
   * @return 匹配的基列表 / List of congruent bases
   */
  [[nodiscard]] std::vector<base_4pcs_t> find_congruent_sets(
      const base_4pcs_t& source_base, const std::vector<std::size_t>& target_indices,
      const point_cloud_ptr& target_cloud) const;

  /**
   * @brief 验证变换的有效性 / Verify transformation validity
   * @param transform 变换矩阵 / Transformation matrix
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool is_valid_transformation(const transformation_t& transform) const;

private:
  // 4PCS特定参数 / 4PCS-specific parameters
  DataType m_delta = static_cast<DataType>(0.01);     ///< 配准精度 / Registration accuracy
  DataType m_overlap = static_cast<DataType>(0.5);    ///< 重叠率估计 / Overlap ratio estimate
  std::size_t m_sample_size = 200;                    ///< 采样点数 / Sample size
  bool m_use_normals = false;                         ///< 是否使用法向量 / Whether to use normals
  DataType m_max_normal_angle = static_cast<DataType>(0.349);  ///< 最大法向量偏差（20度） / Max normal deviation (20 degrees)
  std::size_t m_num_bases = 100;                      ///< 尝试的基数量 / Number of bases to try

  // 内部数据结构 / Internal data structures
  std::shared_ptr<kdtree_t> m_target_kdtree;          ///< 目标点云的KD树 / KD-tree for target cloud
  std::vector<std::size_t> m_source_samples;          ///< 源点云采样索引 / Source cloud sample indices
  std::vector<std::size_t> m_target_samples;          ///< 目标点云采样索引 / Target cloud sample indices
};

}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/four_pcs_registration_impl.hpp>
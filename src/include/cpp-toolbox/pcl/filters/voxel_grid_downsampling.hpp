#pragma once

#include <cpp-toolbox/pcl/filters/filters.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/utils/random.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT voxel_grid_downsampling_t
    : public filter_t<voxel_grid_downsampling_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = filter_t<voxel_grid_downsampling_t<DataType>, DataType>;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr =
      std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;
  // 体素坐标键类型 - 使用整数代替元组以提高性能
  using voxel_key_t = std::uint64_t;

  // 体素数据的SoA (Structure of Arrays)结构
  struct voxel_data_soa_t
  {
    // 点坐标和累加值
    std::vector<DataType> sum_x;
    std::vector<DataType> sum_y;
    std::vector<DataType> sum_z;

    // 法线累加值
    std::vector<DataType> sum_nx;
    std::vector<DataType> sum_ny;
    std::vector<DataType> sum_nz;

    // 颜色累加值
    std::vector<DataType> sum_r;
    std::vector<DataType> sum_g;
    std::vector<DataType> sum_b;

    // 计数和索引映射
    std::vector<std::size_t> counts;
    std::vector<std::size_t> voxel_indices;  // 用于从体素索引映射到输出点云索引

    // 添加新体素
    std::size_t add_voxel();

    // 获取体素数量
    [[nodiscard]] std::size_t size() const;

    // 预分配内存
    void reserve(std::size_t reserve_size);

    // 清空数据
    void clear();
  };

  // 哈希函数定义
  struct key_hash
  {
    std::size_t operator()(const voxel_key_t& key) const noexcept;
  };

  explicit voxel_grid_downsampling_t(float voxel_size)
      : m_voxel_size(voxel_size)
  {
  }
  ~voxel_grid_downsampling_t() = default;

  // 禁用拷贝和移动
  voxel_grid_downsampling_t(const voxel_grid_downsampling_t&) = delete;
  voxel_grid_downsampling_t& operator=(const voxel_grid_downsampling_t&) =
      delete;
  voxel_grid_downsampling_t(voxel_grid_downsampling_t&&) = delete;
  voxel_grid_downsampling_t& operator=(voxel_grid_downsampling_t&&) = delete;

  // 实现接口方法
  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  void enable_parallel_impl(bool enable);
  point_cloud filter_impl();
  void filter_impl(point_cloud_ptr output);

private:
  // 处理点云数据，将点添加到体素中
  void process_point(
      std::size_t idx,
      std::unordered_map<voxel_key_t, std::size_t, key_hash>& voxel_map,
      voxel_data_soa_t& voxel_data);

  // 合并多个线程的体素数据
  void merge_thread_data(
      const std::vector<std::unordered_map<voxel_key_t, std::size_t, key_hash>>&
          thread_maps,
      const std::vector<voxel_data_soa_t>& thread_data,
      std::unordered_map<voxel_key_t, std::size_t, key_hash>& merged_map,
      voxel_data_soa_t& merged_data);

  // 计算点云边界，用于优化体素索引计算
  void compute_point_cloud_bounds();

  // 计算体素键值
  [[nodiscard]] voxel_key_t compute_voxel_key(int ix, int iy, int iz) const;

  // 估计体素数量
  [[nodiscard]] std::size_t estimate_voxel_count() const;

  // 成员变量
  float m_voxel_size = 1.0F;
  bool m_enable_parallel = false;
  point_cloud_ptr m_cloud;

  // 点云边界相关变量
  bool m_bounds_computed = false;
  int m_min_ix = 0, m_min_iy = 0, m_min_iz = 0;
  int m_max_ix = 0, m_max_iy = 0, m_max_iz = 0;
  int m_span_x = 0, m_span_y = 0;  // 用于快速计算键值
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/filters/impl/voxel_grid_downsampling_impl.hpp>
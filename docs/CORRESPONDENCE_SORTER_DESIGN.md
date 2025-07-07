# 对应关系排序器设计文档

## 概述

对应关系排序器（Correspondence Sorter）是一个独立的模块，用于根据不同的质量度量对点云对应关系进行排序。这个设计将排序逻辑从 PROSAC 等算法中解耦，使得排序策略可以灵活配置和扩展。

## 设计理念

1. **单一职责**：排序器只负责对对应关系进行质量评估和排序
2. **策略模式**：不同的排序策略实现同一接口
3. **可组合性**：多个排序器可以组合使用
4. **高效性**：支持延迟计算和缓存

## 基类设计

### 基础接口

```cpp
namespace toolbox::pcl {

/**
 * @brief 对应关系排序器的基类（CRTP模式）
 * 
 * 该类定义了所有对应关系排序算法的通用接口，用于评估和排序对应关系的质量。
 * 
 * @tparam Derived 派生类类型
 * @tparam DataType 数据类型（如float或double）
 */
template<typename Derived, typename DataType>
class base_correspondence_sorter_t
{
public:
    using point_cloud = toolbox::types::point_cloud_t<DataType>;
    using point_cloud_ptr = std::shared_ptr<point_cloud>;
    using correspondence_t = toolbox::pcl::correspondence_t;
    using correspondences_ptr = std::shared_ptr<std::vector<correspondence_t>>;
    using quality_scores_t = std::vector<DataType>;
    using sorted_indices_t = std::vector<std::size_t>;
    
    base_correspondence_sorter_t() = default;
    virtual ~base_correspondence_sorter_t() = default;
    
    // 删除拷贝，允许移动
    base_correspondence_sorter_t(const base_correspondence_sorter_t&) = delete;
    base_correspondence_sorter_t& operator=(const base_correspondence_sorter_t&) = delete;
    base_correspondence_sorter_t(base_correspondence_sorter_t&&) = default;
    base_correspondence_sorter_t& operator=(base_correspondence_sorter_t&&) = default;
    
    /**
     * @brief 设置源点云和目标点云
     */
    void set_point_clouds(const point_cloud_ptr& source,
                         const point_cloud_ptr& target)
    {
        m_source_cloud = source;
        m_target_cloud = target;
        m_cached = false;
    }
    
    /**
     * @brief 设置对应关系
     */
    void set_correspondences(const correspondences_ptr& correspondences)
    {
        m_correspondences = correspondences;
        m_cached = false;
    }
    
    /**
     * @brief 计算质量分数并返回排序后的索引
     * @param[out] scores 每个对应关系的质量分数
     * @return 按质量降序排序的索引
     */
    sorted_indices_t compute_sorted_indices(quality_scores_t& scores)
    {
        if (!validate_input()) {
            return sorted_indices_t{};
        }
        
        // 如果已缓存且输入未改变，直接返回
        if (m_cached) {
            scores = m_cached_scores;
            return m_cached_indices;
        }
        
        // 调用派生类实现计算质量分数
        scores.resize(m_correspondences->size());
        static_cast<Derived*>(this)->compute_quality_scores_impl(scores);
        
        // 创建索引数组并按质量分数排序
        sorted_indices_t indices(scores.size());
        std::iota(indices.begin(), indices.end(), 0);
        
        // 降序排序（质量高的在前）
        std::sort(indices.begin(), indices.end(),
                  [&scores](std::size_t i, std::size_t j) {
                      return scores[i] > scores[j];
                  });
        
        // 缓存结果
        m_cached_scores = scores;
        m_cached_indices = indices;
        m_cached = true;
        
        return indices;
    }
    
    /**
     * @brief 仅计算质量分数，不排序
     */
    quality_scores_t compute_quality_scores()
    {
        quality_scores_t scores;
        compute_sorted_indices(scores);
        return scores;
    }
    
    /**
     * @brief 获取排序器名称
     */
    [[nodiscard]] std::string get_sorter_name() const
    {
        return static_cast<const Derived*>(this)->get_sorter_name_impl();
    }
    
    /**
     * @brief 清除缓存
     */
    void clear_cache()
    {
        m_cached = false;
        m_cached_scores.clear();
        m_cached_indices.clear();
    }
    
protected:
    /**
     * @brief 验证输入数据
     */
    [[nodiscard]] bool validate_input() const
    {
        if (!m_correspondences || m_correspondences->empty()) {
            LOG_ERROR_S << "错误：对应关系为空";
            return false;
        }
        
        if (!m_source_cloud || !m_target_cloud) {
            LOG_WARN_S << "警告：点云未设置，某些排序器可能需要点云数据";
        }
        
        return static_cast<const Derived*>(this)->validate_input_impl();
    }
    
    /**
     * @brief 派生类可选的额外验证
     */
    [[nodiscard]] bool validate_input_impl() const { return true; }
    
    // 数据成员
    point_cloud_ptr m_source_cloud;
    point_cloud_ptr m_target_cloud;
    correspondences_ptr m_correspondences;
    
    // 缓存
    bool m_cached = false;
    quality_scores_t m_cached_scores;
    sorted_indices_t m_cached_indices;
};

} // namespace toolbox::pcl
```

## 具体排序器实现

### 1. 描述子距离排序器

```cpp
template<typename DataType>
class descriptor_distance_sorter_t 
    : public base_correspondence_sorter_t<descriptor_distance_sorter_t<DataType>, DataType>
{
public:
    using base_type = base_correspondence_sorter_t<descriptor_distance_sorter_t<DataType>, DataType>;
    using typename base_type::quality_scores_t;
    
    /**
     * @brief 设置是否反转分数（距离越小质量越高）
     */
    void set_invert_score(bool invert) { m_invert = invert; }
    
protected:
    void compute_quality_scores_impl(quality_scores_t& scores)
    {
        const auto& corrs = *this->m_correspondences;
        
        // 找到最大距离用于归一化
        DataType max_distance = 0;
        for (const auto& corr : corrs) {
            max_distance = std::max(max_distance, static_cast<DataType>(corr.distance));
        }
        
        // 计算质量分数
        for (std::size_t i = 0; i < corrs.size(); ++i) {
            if (m_invert && max_distance > 0) {
                // 距离越小，质量越高
                scores[i] = 1.0 - (corrs[i].distance / max_distance);
            } else {
                // 直接使用距离作为分数
                scores[i] = corrs[i].distance;
            }
        }
    }
    
    [[nodiscard]] std::string get_sorter_name_impl() const 
    { 
        return "DescriptorDistance"; 
    }
    
private:
    bool m_invert = true;  // 默认：距离越小质量越高
};
```

### 2. 几何一致性排序器

```cpp
template<typename DataType>
class geometric_consistency_sorter_t 
    : public base_correspondence_sorter_t<geometric_consistency_sorter_t<DataType>, DataType>
{
public:
    using base_type = base_correspondence_sorter_t<geometric_consistency_sorter_t<DataType>, DataType>;
    using typename base_type::quality_scores_t;
    
    /**
     * @brief 设置邻域大小
     */
    void set_neighborhood_size(std::size_t size) { m_neighborhood_size = size; }
    
    /**
     * @brief 设置距离比率阈值
     */
    void set_distance_ratio_threshold(DataType threshold) 
    { 
        m_distance_ratio_threshold = threshold; 
    }
    
protected:
    bool validate_input_impl() const override
    {
        if (!this->m_source_cloud || !this->m_target_cloud) {
            LOG_ERROR_S << "错误：几何一致性排序器需要点云数据";
            return false;
        }
        return true;
    }
    
    void compute_quality_scores_impl(quality_scores_t& scores)
    {
        const auto& corrs = *this->m_correspondences;
        const auto& src_cloud = *this->m_source_cloud;
        const auto& tgt_cloud = *this->m_target_cloud;
        
        // 对每个对应关系计算几何一致性
        #pragma omp parallel for if(this->m_parallel_enabled)
        for (std::size_t i = 0; i < corrs.size(); ++i) {
            scores[i] = compute_single_consistency(i, corrs, src_cloud, tgt_cloud);
        }
    }
    
    [[nodiscard]] std::string get_sorter_name_impl() const 
    { 
        return "GeometricConsistency"; 
    }
    
private:
    DataType compute_single_consistency(
        std::size_t idx,
        const std::vector<correspondence_t>& corrs,
        const point_cloud& src_cloud,
        const point_cloud& tgt_cloud) const
    {
        const auto& corr = corrs[idx];
        DataType consistency_score = 0;
        std::size_t valid_pairs = 0;
        
        // 随机选择一些其他对应关系进行比较
        std::size_t num_samples = std::min(m_neighborhood_size, corrs.size() - 1);
        
        for (std::size_t j = 0; j < corrs.size() && valid_pairs < num_samples; ++j) {
            if (j == idx) continue;
            
            // 计算源点对之间的距离
            const auto& src_p1 = src_cloud.points[corr.src_idx];
            const auto& src_p2 = src_cloud.points[corrs[j].src_idx];
            DataType src_dist = compute_distance(src_p1, src_p2);
            
            // 计算目标点对之间的距离
            const auto& tgt_p1 = tgt_cloud.points[corr.dst_idx];
            const auto& tgt_p2 = tgt_cloud.points[corrs[j].dst_idx];
            DataType tgt_dist = compute_distance(tgt_p1, tgt_p2);
            
            // 检查距离比率
            if (src_dist > 0.001 && tgt_dist > 0.001) {
                DataType ratio = src_dist / tgt_dist;
                if (ratio > (1.0 - m_distance_ratio_threshold) && 
                    ratio < (1.0 + m_distance_ratio_threshold)) {
                    consistency_score += 1.0;
                }
                valid_pairs++;
            }
        }
        
        return valid_pairs > 0 ? consistency_score / valid_pairs : 0;
    }
    
    template<typename PointT>
    DataType compute_distance(const PointT& p1, const PointT& p2) const
    {
        return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                        (p1.y - p2.y) * (p1.y - p2.y) +
                        (p1.z - p2.z) * (p1.z - p2.z));
    }
    
    std::size_t m_neighborhood_size = 10;
    DataType m_distance_ratio_threshold = 0.2;  // 20% 的距离变化容忍度
    bool m_parallel_enabled = true;
};
```

### 3. 组合排序器

```cpp
template<typename DataType>
class combined_sorter_t 
    : public base_correspondence_sorter_t<combined_sorter_t<DataType>, DataType>
{
public:
    using base_type = base_correspondence_sorter_t<combined_sorter_t<DataType>, DataType>;
    using typename base_type::quality_scores_t;
    using sorter_ptr = std::shared_ptr<base_correspondence_sorter_t<void, DataType>>;
    
    /**
     * @brief 添加子排序器及其权重
     */
    void add_sorter(sorter_ptr sorter, DataType weight)
    {
        m_sorters.push_back({sorter, weight});
        this->m_cached = false;
    }
    
    /**
     * @brief 归一化权重
     */
    void normalize_weights()
    {
        DataType sum = 0;
        for (const auto& [sorter, weight] : m_sorters) {
            sum += weight;
        }
        
        if (sum > 0) {
            for (auto& [sorter, weight] : m_sorters) {
                weight /= sum;
            }
        }
    }
    
protected:
    void compute_quality_scores_impl(quality_scores_t& scores)
    {
        if (m_sorters.empty()) {
            LOG_WARN_S << "警告：组合排序器没有子排序器";
            std::fill(scores.begin(), scores.end(), 0);
            return;
        }
        
        // 初始化分数为0
        std::fill(scores.begin(), scores.end(), 0);
        
        // 计算加权组合分数
        for (const auto& [sorter, weight] : m_sorters) {
            // 设置数据
            sorter->set_point_clouds(this->m_source_cloud, this->m_target_cloud);
            sorter->set_correspondences(this->m_correspondences);
            
            // 计算子排序器的分数
            quality_scores_t sub_scores = sorter->compute_quality_scores();
            
            // 加权累加
            for (std::size_t i = 0; i < scores.size(); ++i) {
                scores[i] += weight * sub_scores[i];
            }
        }
    }
    
    [[nodiscard]] std::string get_sorter_name_impl() const 
    { 
        return "Combined"; 
    }
    
private:
    std::vector<std::pair<sorter_ptr, DataType>> m_sorters;
};
```

### 4. 自定义排序器

```cpp
template<typename DataType>
class custom_function_sorter_t 
    : public base_correspondence_sorter_t<custom_function_sorter_t<DataType>, DataType>
{
public:
    using base_type = base_correspondence_sorter_t<custom_function_sorter_t<DataType>, DataType>;
    using typename base_type::quality_scores_t;
    using quality_function_t = std::function<DataType(
        const correspondence_t&,
        const point_cloud&,
        const point_cloud&)>;
    
    /**
     * @brief 设置自定义质量函数
     */
    void set_quality_function(quality_function_t func)
    {
        m_quality_function = func;
        this->m_cached = false;
    }
    
protected:
    void compute_quality_scores_impl(quality_scores_t& scores)
    {
        if (!m_quality_function) {
            LOG_ERROR_S << "错误：未设置质量函数";
            std::fill(scores.begin(), scores.end(), 0);
            return;
        }
        
        const auto& corrs = *this->m_correspondences;
        
        for (std::size_t i = 0; i < corrs.size(); ++i) {
            scores[i] = m_quality_function(
                corrs[i], 
                *this->m_source_cloud, 
                *this->m_target_cloud
            );
        }
    }
    
    [[nodiscard]] std::string get_sorter_name_impl() const 
    { 
        return "CustomFunction"; 
    }
    
private:
    quality_function_t m_quality_function;
};
```

## 更新后的 PROSAC 接口

PROSAC 现在接受已排序的对应关系：

```cpp
template<typename DataType>
class prosac_registration_t 
    : public base_coarse_registration_t<prosac_registration_t<DataType>, DataType>
{
public:
    // ... 其他成员 ...
    
    /**
     * @brief 设置已排序的对应关系
     * @param correspondences 对应关系
     * @param sorted_indices 按质量降序排序的索引（可选）
     * @param quality_scores 质量分数（可选）
     */
    void set_sorted_correspondences(
        const correspondences_ptr& correspondences,
        const std::vector<std::size_t>& sorted_indices = {},
        const std::vector<DataType>& quality_scores = {})
    {
        this->m_correspondences = correspondences;
        
        if (!sorted_indices.empty()) {
            m_sorted_indices = sorted_indices;
        } else {
            // 如果未提供排序索引，假设对应关系已按顺序排序
            m_sorted_indices.resize(correspondences->size());
            std::iota(m_sorted_indices.begin(), m_sorted_indices.end(), 0);
        }
        
        m_quality_scores = quality_scores;
    }
    
    // ... 其他成员 ...
};
```

## 使用示例

### 基本使用

```cpp
// 创建描述子距离排序器
auto distance_sorter = std::make_shared<descriptor_distance_sorter_t<float>>();
distance_sorter->set_point_clouds(source_cloud, target_cloud);
distance_sorter->set_correspondences(correspondences);

// 计算排序后的索引
std::vector<float> quality_scores;
auto sorted_indices = distance_sorter->compute_sorted_indices(quality_scores);

// 使用排序后的对应关系进行 PROSAC 配准
prosac_registration_t<float> prosac;
prosac.set_source(source_cloud);
prosac.set_target(target_cloud);
prosac.set_sorted_correspondences(correspondences, sorted_indices, quality_scores);

registration_result_t<float> result;
prosac.align(result);
```

### 组合排序器使用

```cpp
// 创建组合排序器
auto combined_sorter = std::make_shared<combined_sorter_t<float>>();

// 添加描述子距离排序器（权重 0.7）
auto dist_sorter = std::make_shared<descriptor_distance_sorter_t<float>>();
combined_sorter->add_sorter(dist_sorter, 0.7f);

// 添加几何一致性排序器（权重 0.3）
auto geom_sorter = std::make_shared<geometric_consistency_sorter_t<float>>();
geom_sorter->set_neighborhood_size(20);
combined_sorter->add_sorter(geom_sorter, 0.3f);

// 归一化权重
combined_sorter->normalize_weights();

// 使用组合排序器
combined_sorter->set_point_clouds(source_cloud, target_cloud);
combined_sorter->set_correspondences(correspondences);

std::vector<float> scores;
auto indices = combined_sorter->compute_sorted_indices(scores);
```

### 自定义排序器使用

```cpp
// 创建自定义排序器
auto custom_sorter = std::make_shared<custom_function_sorter_t<float>>();

// 设置自定义质量函数
custom_sorter->set_quality_function(
    [](const correspondence_t& corr,
       const point_cloud<float>& src,
       const point_cloud<float>& tgt) -> float {
        // 自定义质量计算逻辑
        // 例如：结合描述子距离和法向量夹角
        float desc_quality = 1.0f - (corr.distance / 100.0f);
        
        // 假设点云有法向量
        const auto& src_normal = src.points[corr.src_idx].normal;
        const auto& tgt_normal = tgt.points[corr.dst_idx].normal;
        float normal_dot = src_normal.dot(tgt_normal);
        float normal_quality = (normal_dot + 1.0f) / 2.0f;  // 归一化到 [0,1]
        
        return 0.6f * desc_quality + 0.4f * normal_quality;
    });

custom_sorter->set_point_clouds(source_cloud, target_cloud);
custom_sorter->set_correspondences(correspondences);

auto sorted_indices = custom_sorter->compute_sorted_indices(scores);
```

## 优势

1. **解耦性**：排序逻辑与配准算法分离
2. **可扩展性**：易于添加新的排序策略
3. **可组合性**：不同排序器可以灵活组合
4. **复用性**：排序器可用于多种算法（PROSAC、RANSAC 变种等）
5. **性能优化**：支持缓存和并行计算

## 文件组织

```
src/include/cpp-toolbox/pcl/correspondence/
├── base_correspondence_sorter.hpp
├── descriptor_distance_sorter.hpp
├── geometric_consistency_sorter.hpp
├── combined_sorter.hpp
├── custom_function_sorter.hpp
└── impl/
    ├── descriptor_distance_sorter_impl.hpp
    ├── geometric_consistency_sorter_impl.hpp
    ├── combined_sorter_impl.hpp
    └── custom_function_sorter_impl.hpp
```
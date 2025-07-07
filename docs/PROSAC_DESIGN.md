# PROSAC (渐进式采样一致性) 粗配准设计文档

## 算法概述

PROSAC (Progressive Sample Consensus，渐进式采样一致性) 是 RANSAC 的改进版本，通过利用对应关系的质量排序来实现更快的收敛。与随机均匀采样不同，PROSAC 从逐渐增大的高质量对应关系集合中进行渐进式采样。

### 核心优势

- **更快收敛**：在高外点率情况下，通常比 RANSAC 快 10-100 倍
- **质量感知采样**：利用对应关系质量分数
- **统计保证**：保持与 RANSAC 相同的理论保证
- **优雅降级**：在最坏情况下收敛到 RANSAC 行为

## 算法详解

### 1. 核心算法流程

```
1. 按质量对对应关系进行排序（降序）
2. 初始化 n = m（最小样本大小，3D 刚体变换通常为 3）
3. 对于 t = 1 到 max_iterations：
   a. 如果 t == T_n，递增 n（扩展采样池）
   b. 抽取样本：
      - 选择第 n 个对应关系
      - 从 {1, ..., n-1} 中选择 m-1 个对应关系
   c. 从样本估计变换
   d. 计算内点并在改进时更新最佳模型
   e. 检查停止准则（非随机性和最大性）
   f. 如果满足准则，提前终止
4. 可选：使用所有内点精炼变换
5. 返回最佳变换和内点集
```

### 2. 数学公式

#### 渐进采样调度

PROSAC 的关键是渐进采样调度 T_n，它决定何时包含第 n 个对应关系：

```
T_n = T_{n-1} + ceil(T_m * (n - m) / (m * C(n, m)))

其中：
- T_m = N * (1 - eta_0)^m（前 m 个对应关系的初始迭代次数）
- N = 对应关系总数
- eta_0 = 内点率的初始估计（例如 0.1）
- C(n, m) = 二项式系数 "n 选 m"
```

#### 停止准则

**非随机性准则**：如果偶然获得观察到的内点数量的概率低于阈值（例如 5%），则认为解是非随机的：

```
P_inlier = 1 - prod_{j=m}^{I_star}(1 - beta(j, m, n))

其中：
- beta(i, m, n) = C(i-1, m-1) * C(n-i, 1) / C(n, m)
- I_star = 迄今为止的最佳内点数
```

**最大性准则**：当找到更好模型的概率低于阈值时停止：

```
k_max = log(eta) / log(1 - (I_star/n)^m)

其中 eta = 0.05（5% 阈值）
```

### 3. 变换估计

对于 3D 刚体变换（最小样本大小 = 3）：

1. **样本验证**：确保点不共线
2. **基于 SVD 的估计**（Kabsch 算法）：

   ```
   - 计算源点和目标点的质心
   - 将两个点集中心化
   - 计算协方差矩阵 H = sum(p_i * q_i^T)
   - SVD 分解：H = U * S * V^T
   - 旋转：R = V * U^T（如果 det(R) < 0 则处理反射）
   - 平移：t = centroid_target - R * centroid_source
   ```

## API 设计

### 类接口

```cpp
namespace toolbox::pcl {

template<typename DataType>
class prosac_registration_t 
    : public base_coarse_registration_t<prosac_registration_t<DataType>, DataType>
{
public:
    // 类型别名
    using base_type = base_coarse_registration_t<prosac_registration_t<DataType>, DataType>;
    using typename base_type::point_cloud;
    using typename base_type::point_cloud_ptr;
    using typename base_type::correspondences_ptr;
    using typename base_type::result_type;
    using transformation_t = Eigen::Matrix<DataType, 4, 4>;
    
    // 构造函数
    prosac_registration_t();
    ~prosac_registration_t() = default;
    
    // 配置方法
    void set_confidence(DataType confidence);
    void set_sample_size(std::size_t size);
    void set_refine_result(bool refine);
    void set_early_stop_ratio(DataType ratio);
    
    // PROSAC 特定方法
    void set_initial_inlier_ratio(DataType ratio);
    void set_non_randomness_threshold(DataType threshold);
    
    /**
     * @brief 设置已排序的对应关系
     * @param correspondences 对应关系
     * @param sorted_indices 按质量降序排序的索引（可选）
     * @param quality_scores 质量分数（可选，用于调试和分析）
     * 
     * @note 如果不提供 sorted_indices，则假设 correspondences 已经按质量降序排序
     */
    void set_sorted_correspondences(
        const correspondences_ptr& correspondences,
        const std::vector<std::size_t>& sorted_indices = {},
        const std::vector<DataType>& quality_scores = {});
    
    // 获取器
    [[nodiscard]] DataType get_confidence() const;
    [[nodiscard]] std::size_t get_sample_size() const;
    [[nodiscard]] bool get_refine_result() const;
    [[nodiscard]] DataType get_early_stop_ratio() const;
    [[nodiscard]] DataType get_initial_inlier_ratio() const;
    [[nodiscard]] const std::vector<std::size_t>& get_sorted_indices() const;
    
protected:
    // 实现方法（基类要求）
    bool align_impl(result_type& result);
    [[nodiscard]] std::string get_algorithm_name_impl() const { return "PROSAC"; }
    [[nodiscard]] bool validate_input_impl() const;
    void set_correspondences_impl(const correspondences_ptr& correspondences);
    
private:
    // 核心算法方法
    void precompute_sampling_schedule(std::size_t n_correspondences);
    void progressive_sample(std::vector<correspondence_t>& sample, 
                           std::size_t n, std::size_t t,
                           std::mt19937& generator) const;
    [[nodiscard]] transformation_t estimate_transformation(
        const std::vector<correspondence_t>& sample) const;
    [[nodiscard]] std::size_t count_inliers(const transformation_t& transform,
                                            std::vector<std::size_t>& inliers) const;
    [[nodiscard]] bool check_non_randomness(std::size_t inlier_count, 
                                            std::size_t n) const;
    [[nodiscard]] bool check_maximality(std::size_t inlier_count, 
                                        std::size_t n, std::size_t t) const;
    [[nodiscard]] transformation_t refine_transformation(
        const std::vector<std::size_t>& inlier_indices) const;
    
    // 辅助方法
    [[nodiscard]] DataType compute_beta(std::size_t i, std::size_t m, 
                                        std::size_t n) const;
    [[nodiscard]] std::size_t compute_binomial_coefficient(std::size_t n, 
                                                           std::size_t k) const;
    [[nodiscard]] bool is_sample_valid(
        const std::vector<correspondence_t>& sample) const;
    
    // 参数
    DataType m_confidence = 0.99;
    std::size_t m_sample_size = 3;
    bool m_refine_result = true;
    DataType m_early_stop_ratio = 0.9;
    DataType m_initial_inlier_ratio = 0.1;
    DataType m_non_randomness_threshold = 0.05;
    
    // 排序信息
    std::vector<std::size_t> m_sorted_indices;    // 排序后的索引
    std::vector<DataType> m_quality_scores;       // 质量分数（可选）
    
    // 预计算的采样调度
    std::vector<std::size_t> m_T_n;
    
    // 统计信息
    mutable std::size_t m_total_samples = 0;
    mutable std::size_t m_best_inlier_count = 0;
};

} // namespace toolbox::pcl
```

## 与对应关系排序器的集成

PROSAC 现在不再内部处理对应关系的排序，而是接受已经排序好的对应关系。质量度量和排序功能由独立的对应关系排序器模块提供。

详细的对应关系排序器设计请参见：[对应关系排序器设计文档](./CORRESPONDENCE_SORTER_DESIGN.md)

### 排序器类型

1. **描述子距离排序器**：基于描述子匹配距离
2. **几何一致性排序器**：基于空间几何关系的保持程度
3. **组合排序器**：多种排序策略的加权组合
4. **自定义函数排序器**：用户定义的质量评估函数

## 实施计划

### 第一阶段：核心算法（第 1 周）

1. 创建 `prosac_registration.hpp` 头文件
2. 实现基本的 PROSAC 采样循环
3. 添加基于 SVD 的变换估计
4. 实现渐进采样调度
5. 添加基本的内点计数

### 第二阶段：停止准则（第 2 周）

1. 实现非随机性准则
2. 实现最大性准则
3. 添加高效的概率计算
4. 优化二项式系数计算

### 第三阶段：质量度量和优化（第 3 周）

1. 实现默认质量函数
2. 添加质量函数接口
3. 预计算 T_n 值以提高效率
4. 使用线程池添加并行内点计数
5. 实现样本缓存以避免冗余计算

### 第四阶段：集成和测试（第 4 周）

1. 添加到 `registration.hpp` 模块头文件
2. 创建全面的单元测试
3. 添加与 RANSAC 比较的基准测试
4. 编写使用示例和文档
5. 性能分析和优化

## 使用示例

### 基本使用（使用描述子距离排序）

```cpp
// 1. 创建排序器并排序对应关系
auto distance_sorter = std::make_shared<descriptor_distance_sorter_t<float>>();
distance_sorter->set_point_clouds(source_cloud, target_cloud);
distance_sorter->set_correspondences(correspondences);

std::vector<float> quality_scores;
auto sorted_indices = distance_sorter->compute_sorted_indices(quality_scores);

// 2. 创建 PROSAC 配准对象
prosac_registration_t<float> prosac;
prosac.set_source(source_cloud);
prosac.set_target(target_cloud);
prosac.set_max_iterations(10000);
prosac.set_inlier_threshold(0.05f);
prosac.set_confidence(0.99f);

// 3. 设置已排序的对应关系
prosac.set_sorted_correspondences(correspondences, sorted_indices, quality_scores);

// 4. 执行配准
registration_result_t<float> result;
if (prosac.align(result)) {
    std::cout << "PROSAC 配准成功！" << std::endl;
    std::cout << "内点数：" << result.inliers.size() << "/" 
              << correspondences->size() << std::endl;
    std::cout << "适应度分数：" << result.fitness_score << std::endl;
    std::cout << "变换矩阵：\n" << result.transformation << std::endl;
}
```

### 高级使用（组合排序策略）

```cpp
// 1. 创建组合排序器
auto combined_sorter = std::make_shared<combined_sorter_t<float>>();

// 添加描述子距离排序器（权重 0.6）
auto dist_sorter = std::make_shared<descriptor_distance_sorter_t<float>>();
combined_sorter->add_sorter(dist_sorter, 0.6f);

// 添加几何一致性排序器（权重 0.4）
auto geom_sorter = std::make_shared<geometric_consistency_sorter_t<float>>();
geom_sorter->set_neighborhood_size(15);
combined_sorter->add_sorter(geom_sorter, 0.4f);

// 归一化权重
combined_sorter->normalize_weights();

// 2. 排序对应关系
combined_sorter->set_point_clouds(source_cloud, target_cloud);
combined_sorter->set_correspondences(correspondences);

std::vector<float> quality_scores;
auto sorted_indices = combined_sorter->compute_sorted_indices(quality_scores);

// 3. 使用 PROSAC 进行配准
prosac_registration_t<float> prosac;
prosac.set_source(source_cloud);
prosac.set_target(target_cloud);
prosac.set_sorted_correspondences(correspondences, sorted_indices, quality_scores);
prosac.set_max_iterations(5000);  // 组合排序通常能更快收敛
prosac.set_inlier_threshold(0.05f);

registration_result_t<float> result;
prosac.align(result);
```

## 性能预期

- **加速比**：在外点率 > 50% 时比 RANSAC 快 10-100 倍
- **内存开销**：最小（预计算的 T_n 数组和排序索引）
- **准确性**：与 RANSAC 相同或更好
- **鲁棒性**：优雅处理边缘情况

## 边缘情况和错误处理

1. **对应关系不足**：返回单位变换并设置失败标志
2. **均匀质量分数**：退化为标准 RANSAC 行为
3. **退化配置**：检测并重新采样
4. **数值问题**：使用鲁棒的 SVD 实现，处理边缘情况
5. **未找到内点**：返回最佳模型并给出适当警告

## 测试策略

1. **单元测试**：
   - 测试渐进采样调度
   - 验证停止准则
   - 测试变换估计
   - 边缘情况处理

2. **集成测试**：
   - 在合成数据上与 RANSAC 比较
   - 测试各种外点率
   - 验证质量函数集成

3. **基准测试**：
   - 与 RANSAC 的速度比较
   - 内存使用分析
   - 对应关系数量的可扩展性

## 未来增强

1. **GPU 加速**：在 GPU 上并行计算内点
2. **自适应质量度量**：从数据学习质量函数
3. **多线程 PROSAC**：使用不同种子运行多个实例
4. **混合方法**：与其他鲁棒估计器结合

## 算法优化策略

### 1. 计算优化

- **预计算优化**：预计算所有 T_n 值和二项式系数
- **缓存策略**：缓存已计算的样本和变换
- **SIMD 优化**：使用 SIMD 指令加速内点计数

### 2. 并行化策略

- **内点并行计数**：使用线程池并行计算内点
- **多实例并行**：运行多个 PROSAC 实例，选择最佳结果
- **GPU 加速**：将内点计数和变换应用移至 GPU

### 3. 内存优化

- **索引操作**：使用索引而非复制点数据
- **延迟计算**：仅在需要时计算变换矩阵
- **内存池**：使用内存池减少分配开销

## 与现有系统集成

1. **遵循 cpp-toolbox 模式**：
   - 继承自 `base_coarse_registration_t`
   - 使用 CRTP 实现静态多态
   - 遵循命名约定和代码风格

2. **复用现有组件**：
   - 使用 `RegistrationResult` 结构
   - 复用 `Correspondence` 结构
   - 利用 `ThreadPoolSingleton` 进行并行处理

3. **模块化设计**：
   - 清晰的接口定义
   - 可扩展的质量函数
   - 灵活的参数配置

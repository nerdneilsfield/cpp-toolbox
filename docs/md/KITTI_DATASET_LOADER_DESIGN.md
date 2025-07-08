# KITTI Odometry Dataset Loader 设计文档

## 1. 概述

本文档描述了cpp-toolbox中KITTI Odometry数据集加载器的设计方案。该加载器基于现有的dataloader架构，支持：

1. **KITTI Odometry数据集**：读取点云和位姿数据
2. **点云对数据集**：支持可配置skip的点云对加载，计算相对变换
3. **Semantic KITTI数据集**：额外读取每个点的语义标签
4. **语义点云对数据集**：结合语义标签的点云对加载

## 2. 数据集格式

### 2.1 KITTI Odometry格式
```
sequences/
├── 00/
│   ├── velodyne/         # 点云数据
│   │   ├── 000000.bin
│   │   ├── 000001.bin
│   │   └── ...
│   └── calib.txt         # 标定文件（可选）
├── 01/
├── ...
└── poses/
    ├── 00.txt            # 序列00的位姿
    ├── 01.txt
    └── ...
```

### 2.2 Semantic KITTI格式
```
sequences/
├── 00/
│   ├── velodyne/         # 点云数据
│   │   └── *.bin
│   ├── labels/           # 语义标签
│   │   └── *.label
│   └── calib.txt
└── poses/
    └── *.txt
```

### 2.3 数据格式说明

- **点云文件(.bin)**：每个点4个float值(x, y, z, intensity)
- **标签文件(.label)**：每个点1个uint32值（低16位为标签ID）
- **位姿文件(.txt)**：每行12个值，表示3x4变换矩阵

## 3. 架构设计

### 3.1 核心数据结构

```cpp
// 单帧数据
template<typename DataType>
struct kitti_odometry_frame_t {
    std::unique_ptr<point_cloud_t<DataType>> cloud;  // 点云数据
    Eigen::Matrix<DataType, 4, 4> pose;              // 全局位姿
    std::size_t frame_index;                         // 帧索引
    std::string timestamp;                           // 时间戳（可选）
};

// 帧对数据
template<typename DataType>
struct kitti_odometry_frame_pair_t {
    std::unique_ptr<point_cloud_t<DataType>> source_cloud;
    std::unique_ptr<point_cloud_t<DataType>> target_cloud;
    Eigen::Matrix<DataType, 4, 4> source_pose;
    Eigen::Matrix<DataType, 4, 4> target_pose;
    Eigen::Matrix<DataType, 4, 4> relative_transform;  // T_target_source
    std::size_t source_index;
    std::size_t target_index;
};

// 语义单帧数据
template<typename DataType>
struct semantic_kitti_frame_t {
    std::unique_ptr<point_cloud_t<DataType>> cloud;
    std::vector<uint32_t> labels;                    // 每个点的标签
    Eigen::Matrix<DataType, 4, 4> pose;
    std::size_t frame_index;
    
    // 获取特定标签的点云
    std::unique_ptr<point_cloud_t<DataType>> get_labeled_cloud(uint32_t label) const;
    // 获取多个标签的点云
    std::unique_ptr<point_cloud_t<DataType>> get_labeled_cloud(
        const std::vector<uint32_t>& labels) const;
};

// 语义帧对数据
template<typename DataType>
struct semantic_kitti_frame_pair_t {
    std::unique_ptr<point_cloud_t<DataType>> source_cloud;
    std::unique_ptr<point_cloud_t<DataType>> target_cloud;
    std::vector<uint32_t> source_labels;
    std::vector<uint32_t> target_labels;
    Eigen::Matrix<DataType, 4, 4> source_pose;
    Eigen::Matrix<DataType, 4, 4> target_pose;
    Eigen::Matrix<DataType, 4, 4> relative_transform;
    std::size_t source_index;
    std::size_t target_index;
};
```

### 3.2 辅助组件

```cpp
// 位姿读取器
template<typename DataType>
class kitti_pose_reader_t {
public:
    // 从文件加载位姿
    bool load(const std::string& poses_file);
    
    // 获取帧数
    std::size_t size() const;
    
    // 获取指定帧的全局位姿
    Eigen::Matrix<DataType, 4, 4> get_pose(std::size_t index) const;
    
    // 计算相对变换
    Eigen::Matrix<DataType, 4, 4> get_relative_transform(
        std::size_t from_index, std::size_t to_index) const;
    
    // 检查索引是否有效
    bool is_valid_index(std::size_t index) const;

private:
    std::vector<Eigen::Matrix<DataType, 4, 4>> poses_;
};

// 标签读取器
class kitti_label_reader_t {
public:
    // 读取标签文件
    static std::vector<uint32_t> read_labels(const std::string& label_file);
    
    // 写入标签文件
    static bool write_labels(const std::string& label_file, 
                           const std::vector<uint32_t>& labels);
    
    // 提取标签ID（忽略实例信息）
    static uint16_t get_label_id(uint32_t label);
    
    // 获取标签名称映射
    static std::map<uint16_t, std::string> get_label_map();
};

// 标定信息
template<typename DataType>
struct kitti_calibration_t {
    Eigen::Matrix<DataType, 4, 4> Tr_velo_to_cam;  // Velodyne到相机
    Eigen::Matrix<DataType, 3, 4> P0, P1, P2, P3;  // 投影矩阵
    
    // 从文件加载
    bool load(const std::string& calib_file);
};
```

### 3.3 数据集类

```cpp
// 基础KITTI Odometry数据集
template<typename DataType = float>
class kitti_odometry_dataset_t : 
    public dataset_t<kitti_odometry_dataset_t<DataType>, 
                     kitti_odometry_frame_t<DataType>> {
public:
    // 构造函数
    explicit kitti_odometry_dataset_t(const std::string& sequence_path);
    
    // 配置选项
    void set_cache_size(std::size_t max_cached_frames);
    void enable_intensity(bool enable);
    
    // CRTP接口实现
    std::size_t size_impl() const;
    std::optional<kitti_odometry_frame_t<DataType>> at_impl(std::size_t index) const;
    
    // 便利方法
    std::string get_cloud_file(std::size_t index) const;
    Eigen::Matrix<DataType, 4, 4> get_pose(std::size_t index) const;
    bool has_calibration() const;
    const kitti_calibration_t<DataType>& get_calibration() const;

private:
    std::string sequence_path_;
    std::string velodyne_path_;
    kitti_pose_reader_t<DataType> pose_reader_;
    std::optional<kitti_calibration_t<DataType>> calibration_;
    std::vector<std::string> cloud_files_;
    
    // LRU缓存
    mutable std::map<std::size_t, kitti_odometry_frame_t<DataType>> cache_;
    mutable std::list<std::size_t> lru_list_;
    std::size_t max_cache_size_;
    
    // 配置选项
    bool load_intensity_ = true;
    
    // 内部方法
    void scan_cloud_files();
    std::unique_ptr<point_cloud_t<DataType>> load_cloud(std::size_t index) const;
    void update_cache(std::size_t index, kitti_odometry_frame_t<DataType>&& frame) const;
};

// 点云对数据集
template<typename DataType = float>
class kitti_odometry_pair_dataset_t : 
    public dataset_t<kitti_odometry_pair_dataset_t<DataType>, 
                     kitti_odometry_frame_pair_t<DataType>> {
public:
    // 构造函数
    kitti_odometry_pair_dataset_t(const std::string& sequence_path, 
                                  std::size_t skip = 1);
    
    // 配置
    void set_skip(std::size_t skip);
    std::size_t get_skip() const;
    
    // CRTP接口
    std::size_t size_impl() const;
    std::optional<kitti_odometry_frame_pair_t<DataType>> at_impl(std::size_t index) const;

private:
    kitti_odometry_dataset_t<DataType> base_dataset_;
    std::size_t skip_;
};

// Semantic KITTI数据集
template<typename DataType = float>
class semantic_kitti_dataset_t : 
    public dataset_t<semantic_kitti_dataset_t<DataType>, 
                     semantic_kitti_frame_t<DataType>> {
public:
    explicit semantic_kitti_dataset_t(const std::string& sequence_path);
    
    // CRTP接口
    std::size_t size_impl() const;
    std::optional<semantic_kitti_frame_t<DataType>> at_impl(std::size_t index) const;
    
    // 语义相关方法
    std::set<uint32_t> get_unique_labels() const;
    std::map<uint16_t, std::string> get_label_names() const;
    std::map<uint16_t, std::size_t> compute_label_statistics() const;

private:
    kitti_odometry_dataset_t<DataType> base_dataset_;
    std::string labels_path_;
    std::vector<std::string> label_files_;
    bool has_labels_;
    
    void scan_label_files();
    std::vector<uint32_t> load_labels(std::size_t index) const;
};

// 语义点云对数据集
template<typename DataType = float>
class semantic_kitti_pair_dataset_t : 
    public dataset_t<semantic_kitti_pair_dataset_t<DataType>, 
                     semantic_kitti_frame_pair_t<DataType>> {
public:
    semantic_kitti_pair_dataset_t(const std::string& sequence_path, 
                                  std::size_t skip = 1);
    
    // 配置
    void set_skip(std::size_t skip);
    
    // CRTP接口
    std::size_t size_impl() const;
    std::optional<semantic_kitti_frame_pair_t<DataType>> at_impl(std::size_t index) const;

private:
    semantic_kitti_dataset_t<DataType> base_dataset_;
    std::size_t skip_;
};
```

### 3.4 工具函数

```cpp
namespace kitti_utils {
    // 读取KITTI二进制点云
    template<typename DataType>
    std::unique_ptr<point_cloud_t<DataType>> read_kitti_cloud(
        const std::string& file_path,
        bool load_intensity = true);
    
    // 写入KITTI二进制点云
    template<typename DataType>
    bool write_kitti_cloud(const std::string& file_path,
                          const point_cloud_t<DataType>& cloud);
    
    // 解析位姿行
    template<typename DataType>
    Eigen::Matrix<DataType, 4, 4> parse_pose_line(const std::string& line);
    
    // 格式化位姿为字符串
    template<typename DataType>
    std::string format_pose(const Eigen::Matrix<DataType, 4, 4>& pose);
    
    // 计算相对变换
    template<typename DataType>
    Eigen::Matrix<DataType, 4, 4> compute_relative_transform(
        const Eigen::Matrix<DataType, 4, 4>& from_pose,
        const Eigen::Matrix<DataType, 4, 4>& to_pose);
    
    // 验证序列目录结构
    bool validate_sequence_directory(const std::string& sequence_path);
    
    // 获取序列信息
    struct sequence_info_t {
        std::size_t num_frames;
        bool has_labels;
        bool has_calibration;
        std::string sequence_name;
    };
    
    sequence_info_t get_sequence_info(const std::string& sequence_path);
}
```

## 4. 使用示例

### 4.1 基础使用

```cpp
// 1. KITTI Odometry数据集
kitti_odometry_dataset_t<float> dataset("/path/to/kitti/sequences/02");
std::cout << "序列包含 " << dataset.size() << " 帧" << std::endl;

// 遍历所有帧
for (std::size_t i = 0; i < dataset.size(); ++i) {
    auto frame = dataset[i];
    if (frame) {
        std::cout << "帧 " << i << ": " 
                  << frame->cloud->size() << " 个点" << std::endl;
        // 处理点云和位姿...
    }
}

// 2. 点云对数据集
kitti_odometry_pair_dataset_t<float> pair_dataset("/path/to/kitti/sequences/02", 5);

for (const auto& pair : pair_dataset) {
    std::cout << "处理帧对: " << pair.source_index 
              << " -> " << pair.target_index << std::endl;
    
    // 使用相对变换进行配准
    registration_result_t<float> result;
    icp.set_initial_transform(pair.relative_transform);
    icp.align(pair.source_cloud, pair.target_cloud, result);
}
```

### 4.2 Semantic KITTI使用

```cpp
// 3. Semantic KITTI数据集
semantic_kitti_dataset_t<float> sem_dataset("/path/to/semantic_kitti/sequences/02");

// 获取标签统计
auto label_stats = sem_dataset.compute_label_statistics();
for (const auto& [label, count] : label_stats) {
    std::cout << "标签 " << label << ": " << count << " 个点" << std::endl;
}

// 处理特定类别
auto frame = sem_dataset[0];
if (frame) {
    // 提取道路点云（标签40）
    auto road_cloud = frame->get_labeled_cloud(40);
    
    // 提取多个类别（建筑物、植被）
    auto static_cloud = frame->get_labeled_cloud({50, 70, 71, 72});
}

// 4. 语义点云对
semantic_kitti_pair_dataset_t<float> sem_pair_dataset(
    "/path/to/semantic_kitti/sequences/02", 10);

for (const auto& pair : sem_pair_dataset) {
    // 只对静态物体进行配准
    auto source_static = extract_static_points(pair.source_cloud, pair.source_labels);
    auto target_static = extract_static_points(pair.target_cloud, pair.target_labels);
    
    // 执行配准...
}
```

### 4.3 与DataLoader集成

```cpp
// 创建数据集
semantic_kitti_dataset_t<float> dataset(sequence_path);

// 配置采样器（随机打乱）
shuffle_policy_t policy(42);
sampler_t<shuffle_policy_t> sampler(dataset.size(), policy);

// 创建线程池进行预取
thread_pool_t pool(8);

// 创建数据加载器
dataloader_t loader(dataset,          // 数据集
                   sampler,           // 采样器
                   4,                 // batch_size
                   2,                 // prefetch_batches
                   &pool,             // 线程池
                   false);            // drop_last

// 批量处理
for (const auto& batch : loader) {
    // batch是vector<semantic_kitti_frame_t<float>>
    for (const auto& frame : batch) {
        process_semantic_frame(frame);
    }
}
```

### 4.4 高级功能

```cpp
// 配置缓存大小
dataset.set_cache_size(100);  // 缓存最多100帧

// 禁用强度信息加载（节省内存）
dataset.enable_intensity(false);

// 获取标定信息
if (dataset.has_calibration()) {
    auto calib = dataset.get_calibration();
    // 将点云投影到相机坐标系
    auto cam_cloud = transform_cloud(*frame->cloud, calib.Tr_velo_to_cam);
}

// 自定义数据增强
class augmented_kitti_dataset_t : public kitti_odometry_dataset_t<float> {
public:
    std::optional<kitti_odometry_frame_t<float>> at_impl(std::size_t index) const override {
        auto frame = kitti_odometry_dataset_t<float>::at_impl(index);
        if (frame) {
            // 应用数据增强
            augment_cloud(*frame->cloud);
        }
        return frame;
    }
    
private:
    void augment_cloud(point_cloud_t<float>& cloud) const {
        // 随机降采样、添加噪声等
    }
};
```

## 5. 性能优化

### 5.1 缓存策略

- **LRU缓存**：避免重复读取相同帧
- **预取机制**：利用dataloader的多线程预取功能
- **延迟加载**：只在需要时读取数据

### 5.2 内存优化

- **选择性加载**：可选择是否加载强度信息
- **智能指针**：使用unique_ptr避免内存泄漏
- **移动语义**：减少数据拷贝

### 5.3 并行处理

```cpp
// 并行处理多个序列
std::vector<std::string> sequences = {"00", "01", "02", "03"};
std::vector<std::future<void>> futures;

thread_pool_t pool(std::thread::hardware_concurrency());

for (const auto& seq : sequences) {
    futures.push_back(pool.submit([seq]() {
        kitti_odometry_dataset_t<float> dataset(
            "/path/to/kitti/sequences/" + seq);
        process_sequence(dataset);
    }));
}

// 等待所有任务完成
for (auto& f : futures) {
    f.get();
}
```

## 6. 错误处理

### 6.1 异常类型

```cpp
// KITTI特定异常
class kitti_exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class kitti_file_not_found : public kitti_exception {
public:
    kitti_file_not_found(const std::string& file) 
        : kitti_exception("File not found: " + file) {}
};

class kitti_invalid_format : public kitti_exception {
public:
    kitti_invalid_format(const std::string& msg) 
        : kitti_exception("Invalid format: " + msg) {}
};
```

### 6.2 错误处理示例

```cpp
try {
    kitti_odometry_dataset_t<float> dataset(sequence_path);
    auto frame = dataset[0];
    if (!frame) {
        std::cerr << "无法加载帧" << std::endl;
        return;
    }
} catch (const kitti_file_not_found& e) {
    std::cerr << "文件未找到: " << e.what() << std::endl;
} catch (const kitti_invalid_format& e) {
    std::cerr << "格式错误: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "未知错误: " << e.what() << std::endl;
}
```

## 7. 测试计划

### 7.1 单元测试

- 数据结构测试
- 文件读取测试
- 位姿计算测试
- 标签处理测试
- 缓存机制测试

### 7.2 集成测试

- 与dataloader集成测试
- 多线程加载测试
- 大规模数据测试

### 7.3 性能测试

- 加载速度基准测试
- 内存使用测试
- 缓存效率测试

## 8. 实施计划

| 阶段 | 任务 | 时间 |
|------|------|------|
| Phase 1 | 基础设施（数据结构、工具函数） | 2天 |
| Phase 2 | 核心数据集实现 | 3天 |
| Phase 3 | Semantic KITTI支持 | 2天 |
| Phase 4 | 测试和优化 | 2天 |
| Phase 5 | 文档和示例 | 1天 |

总计：10个工作日

## 9. 未来扩展

1. **支持更多数据集格式**
   - KITTI Raw
   - KITTI 360
   - nuScenes格式转换

2. **高级功能**
   - 在线数据增强
   - 自动数据统计
   - 可视化工具集成

3. **性能优化**
   - GPU加速点云处理
   - 分布式数据加载
   - 压缩存储支持
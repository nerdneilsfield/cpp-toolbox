# KITTI Dataset Loader 测试策略

## 1. 测试挑战

- KITTI数据集体积庞大（单个序列可能几GB）
- 不能在代码仓库中包含真实数据
- 需要测试各种边界情况和错误处理
- 需要验证数据格式的正确性

## 2. 测试策略概述

### 2.1 三层测试架构

```
┌─────────────────────────────────────┐
│      集成测试（可选，真实数据）        │
├─────────────────────────────────────┤
│      功能测试（小型模拟数据）          │
├─────────────────────────────────────┤
│      单元测试（Mock数据）             │
└─────────────────────────────────────┘
```

## 3. 具体测试方案

### 3.1 Mock数据生成器

创建一个测试辅助类，生成最小化的合法KITTI格式数据：

```cpp
// test/pcl/kitti_test_utils.hpp
namespace kitti_test_utils {
    
// 创建临时测试目录结构
class kitti_test_env_t {
private:
    std::filesystem::path temp_dir_;
    std::string sequence_name_;
    
public:
    kitti_test_env_t(const std::string& sequence_name = "00");
    ~kitti_test_env_t();  // 自动清理
    
    // 获取路径
    std::string get_sequence_path() const;
    std::string get_velodyne_path() const;
    std::string get_labels_path() const;
    std::string get_poses_file() const;
    
    // 创建测试数据
    void create_test_point_cloud(std::size_t frame_idx, 
                                std::size_t num_points = 100);
    void create_test_labels(std::size_t frame_idx, 
                           std::size_t num_points = 100);
    void create_test_poses(std::size_t num_frames);
    void create_test_calibration();
};

// 生成测试点云数据
template<typename DataType>
void generate_test_point_cloud(const std::string& file_path,
                              std::size_t num_points,
                              bool with_pattern = true) {
    std::ofstream file(file_path, std::ios::binary);
    
    std::random_device rd;
    std::mt19937 gen(42);  // 固定种子for可重复性
    std::uniform_real_distribution<DataType> dist(-10.0, 10.0);
    std::uniform_real_distribution<DataType> intensity_dist(0.0, 1.0);
    
    for (std::size_t i = 0; i < num_points; ++i) {
        DataType x, y, z, intensity;
        
        if (with_pattern) {
            // 创建可识别的模式（如平面、球面等）
            DataType angle = 2 * M_PI * i / num_points;
            x = 5.0 * std::cos(angle);
            y = 5.0 * std::sin(angle);
            z = 0.1 * i;
            intensity = static_cast<DataType>(i) / num_points;
        } else {
            x = dist(gen);
            y = dist(gen);
            z = dist(gen);
            intensity = intensity_dist(gen);
        }
        
        file.write(reinterpret_cast<const char*>(&x), sizeof(DataType));
        file.write(reinterpret_cast<const char*>(&y), sizeof(DataType));
        file.write(reinterpret_cast<const char*>(&z), sizeof(DataType));
        file.write(reinterpret_cast<const char*>(&intensity), sizeof(DataType));
    }
}

// 生成测试标签数据
void generate_test_labels(const std::string& file_path,
                         std::size_t num_points,
                         const std::vector<uint32_t>& label_ids = {40, 44, 48, 49, 50, 70});

// 生成测试位姿数据
template<typename DataType>
void generate_test_poses(const std::string& file_path,
                        std::size_t num_frames,
                        bool with_motion = true) {
    std::ofstream file(file_path);
    
    for (std::size_t i = 0; i < num_frames; ++i) {
        Eigen::Matrix<DataType, 4, 4> pose = Eigen::Matrix<DataType, 4, 4>::Identity();
        
        if (with_motion) {
            // 模拟简单的前进运动
            pose(0, 3) = 0.1 * i;  // x方向前进
            pose(1, 3) = 0.05 * std::sin(0.1 * i);  // 轻微横向移动
            
            // 添加小幅旋转
            DataType angle = 0.01 * i;
            pose(0, 0) = std::cos(angle);
            pose(0, 1) = -std::sin(angle);
            pose(1, 0) = std::sin(angle);
            pose(1, 1) = std::cos(angle);
        }
        
        // 写入KITTI格式（前3行，每行4个值）
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (row > 0 || col > 0) file << " ";
                file << std::fixed << std::setprecision(9) << pose(row, col);
            }
        }
        file << "\n";
    }
}

// 验证辅助函数
template<typename DataType>
bool verify_point_cloud_format(const std::string& file_path, 
                              std::size_t expected_points);

bool verify_label_format(const std::string& file_path, 
                        std::size_t expected_points);

template<typename DataType>
bool verify_pose_format(const std::string& file_path, 
                       std::size_t expected_frames);

} // namespace kitti_test_utils
```

### 3.2 单元测试示例

```cpp
// test/pcl/kitti_dataset_test.cpp
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>
#include "kitti_test_utils.hpp"

using namespace toolbox::io;
using namespace kitti_test_utils;

TEST_CASE("KITTI Odometry Dataset Basic Tests", "[kitti][dataset]") {
    // 创建临时测试环境
    kitti_test_env_t test_env("test_seq");
    
    SECTION("空数据集测试") {
        kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
        REQUIRE(dataset.size() == 0);
    }
    
    SECTION("单帧数据测试") {
        // 创建一帧测试数据
        test_env.create_test_point_cloud(0, 1000);
        test_env.create_test_poses(1);
        
        kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
        REQUIRE(dataset.size() == 1);
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        REQUIRE(frame->cloud->size() == 1000);
        REQUIRE(frame->frame_index == 0);
        
        // 验证位姿
        auto expected_pose = Eigen::Matrix4f::Identity();
        REQUIRE(frame->pose.isApprox(expected_pose, 1e-6f));
    }
    
    SECTION("多帧数据测试") {
        const std::size_t num_frames = 10;
        for (std::size_t i = 0; i < num_frames; ++i) {
            test_env.create_test_point_cloud(i, 500 + i * 100);
        }
        test_env.create_test_poses(num_frames);
        
        kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
        REQUIRE(dataset.size() == num_frames);
        
        // 测试随机访问
        auto frame5 = dataset[5];
        REQUIRE(frame5.has_value());
        REQUIRE(frame5->cloud->size() == 500 + 5 * 100);
        
        // 测试迭代器
        std::size_t count = 0;
        for (const auto& frame : dataset) {
            REQUIRE(frame.cloud != nullptr);
            count++;
        }
        REQUIRE(count == num_frames);
    }
    
    SECTION("缓存测试") {
        test_env.create_test_point_cloud(0, 1000);
        test_env.create_test_poses(1);
        
        kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
        dataset.set_cache_size(1);
        
        // 第一次访问
        auto frame1 = dataset[0];
        auto ptr1 = frame1->cloud.get();
        
        // 第二次访问（应该从缓存返回）
        auto frame2 = dataset[0];
        auto ptr2 = frame2->cloud.get();
        
        // 由于缓存，指针应该不同（因为返回的是副本）
        REQUIRE(frame1->cloud->size() == frame2->cloud->size());
    }
}

TEST_CASE("KITTI Frame Pair Dataset Tests", "[kitti][dataset][pair]") {
    kitti_test_env_t test_env("test_seq");
    
    SECTION("基本点云对测试") {
        const std::size_t num_frames = 10;
        for (std::size_t i = 0; i < num_frames; ++i) {
            test_env.create_test_point_cloud(i, 1000);
        }
        test_env.create_test_poses(num_frames);
        
        kitti_odometry_pair_dataset_t<float> pair_dataset(
            test_env.get_sequence_path(), 1);
        
        REQUIRE(pair_dataset.size() == num_frames - 1);
        
        auto pair = pair_dataset[0];
        REQUIRE(pair.has_value());
        REQUIRE(pair->source_index == 0);
        REQUIRE(pair->target_index == 1);
        
        // 验证相对变换
        auto expected_transform = pair->target_pose * pair->source_pose.inverse();
        REQUIRE(pair->relative_transform.isApprox(expected_transform, 1e-6f));
    }
    
    SECTION("Skip参数测试") {
        const std::size_t num_frames = 20;
        for (std::size_t i = 0; i < num_frames; ++i) {
            test_env.create_test_point_cloud(i, 500);
        }
        test_env.create_test_poses(num_frames);
        
        std::size_t skip = 5;
        kitti_odometry_pair_dataset_t<float> pair_dataset(
            test_env.get_sequence_path(), skip);
        
        REQUIRE(pair_dataset.size() == num_frames - skip);
        
        auto first_pair = pair_dataset[0];
        REQUIRE(first_pair->source_index == 0);
        REQUIRE(first_pair->target_index == skip);
    }
}

TEST_CASE("Semantic KITTI Dataset Tests", "[kitti][dataset][semantic]") {
    kitti_test_env_t test_env("test_seq");
    
    SECTION("带标签的数据测试") {
        const std::size_t num_points = 1000;
        test_env.create_test_point_cloud(0, num_points);
        test_env.create_test_labels(0, num_points);
        test_env.create_test_poses(1);
        
        semantic_kitti_dataset_t<float> dataset(test_env.get_sequence_path());
        REQUIRE(dataset.size() == 1);
        
        auto frame = dataset[0];
        REQUIRE(frame.has_value());
        REQUIRE(frame->cloud->size() == num_points);
        REQUIRE(frame->labels.size() == num_points);
    }
    
    SECTION("标签过滤测试") {
        const std::size_t num_points = 1000;
        test_env.create_test_point_cloud(0, num_points);
        
        // 创建具有特定模式的标签
        std::vector<uint32_t> labels;
        for (std::size_t i = 0; i < num_points; ++i) {
            labels.push_back(i % 2 == 0 ? 40 : 44);  // 一半道路，一半停车场
        }
        // 这里需要自定义标签写入...
        
        semantic_kitti_dataset_t<float> dataset(test_env.get_sequence_path());
        auto frame = dataset[0];
        
        // 提取道路点
        auto road_cloud = frame->get_labeled_cloud(40);
        REQUIRE(road_cloud->size() == num_points / 2);
    }
}

TEST_CASE("Error Handling Tests", "[kitti][dataset][error]") {
    SECTION("不存在的路径") {
        REQUIRE_THROWS_AS(
            kitti_odometry_dataset_t<float>("/non/existent/path"),
            kitti_file_not_found
        );
    }
    
    SECTION("损坏的点云文件") {
        kitti_test_env_t test_env("test_seq");
        
        // 创建一个损坏的文件（大小不对）
        std::string cloud_file = test_env.get_velodyne_path() + "/000000.bin";
        std::ofstream file(cloud_file, std::ios::binary);
        file << "corrupted";
        file.close();
        
        test_env.create_test_poses(1);
        
        kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
        auto frame = dataset[0];
        
        // 应该返回空或抛出异常
        REQUIRE((!frame.has_value() || frame->cloud->size() == 0));
    }
    
    SECTION("位姿文件格式错误") {
        kitti_test_env_t test_env("test_seq");
        test_env.create_test_point_cloud(0, 100);
        
        // 创建格式错误的位姿文件
        std::ofstream pose_file(test_env.get_poses_file());
        pose_file << "invalid pose format\n";
        pose_file.close();
        
        REQUIRE_THROWS_AS(
            kitti_odometry_dataset_t<float>(test_env.get_sequence_path()),
            kitti_invalid_format
        );
    }
}
```

### 3.3 性能基准测试

```cpp
// benchmark/kitti_loader_bench.cpp
#include <catch2/benchmark/catch_benchmark.hpp>
#include <cpp-toolbox/io/dataloader/kitti_odometry_dataset.hpp>
#include "kitti_test_utils.hpp"

TEST_CASE("KITTI Loader Performance", "[kitti][benchmark]") {
    kitti_test_env_t test_env("bench_seq");
    
    // 创建不同规模的测试数据
    std::vector<std::size_t> frame_counts = {10, 50, 100};
    std::vector<std::size_t> point_counts = {1000, 10000, 100000};
    
    for (auto num_frames : frame_counts) {
        for (auto num_points : point_counts) {
            // 准备测试数据
            for (std::size_t i = 0; i < num_frames; ++i) {
                test_env.create_test_point_cloud(i, num_points);
            }
            test_env.create_test_poses(num_frames);
            
            std::string bench_name = "Load " + std::to_string(num_frames) + 
                                   " frames with " + std::to_string(num_points) + 
                                   " points";
            
            BENCHMARK(bench_name) {
                kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
                
                // 加载所有帧
                for (std::size_t i = 0; i < dataset.size(); ++i) {
                    auto frame = dataset[i];
                    return frame->cloud->size();
                }
            };
        }
    }
    
    SECTION("缓存性能测试") {
        const std::size_t num_frames = 100;
        const std::size_t num_points = 10000;
        
        for (std::size_t i = 0; i < num_frames; ++i) {
            test_env.create_test_point_cloud(i, num_points);
        }
        test_env.create_test_poses(num_frames);
        
        BENCHMARK("无缓存随机访问") {
            kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
            dataset.set_cache_size(0);  // 禁用缓存
            
            std::size_t total = 0;
            for (int i = 0; i < 10; ++i) {
                auto frame = dataset[i % num_frames];
                total += frame->cloud->size();
            }
            return total;
        };
        
        BENCHMARK("有缓存随机访问") {
            kitti_odometry_dataset_t<float> dataset(test_env.get_sequence_path());
            dataset.set_cache_size(10);  // 缓存10帧
            
            std::size_t total = 0;
            for (int i = 0; i < 10; ++i) {
                auto frame = dataset[i % num_frames];
                total += frame->cloud->size();
            }
            return total;
        };
    }
}
```

### 3.4 集成测试（可选）

创建一个可选的集成测试，只在有真实数据时运行：

```cpp
// test/pcl/kitti_integration_test.cpp
#ifdef KITTI_TEST_DATA_PATH

TEST_CASE("Real KITTI Data Integration Test", "[kitti][integration][.]") {
    // 默认跳过，除非明确运行
    const std::string kitti_path = KITTI_TEST_DATA_PATH;
    
    SECTION("加载真实序列") {
        kitti_odometry_dataset_t<float> dataset(kitti_path + "/sequences/00");
        
        REQUIRE(dataset.size() > 0);
        
        auto first_frame = dataset[0];
        REQUIRE(first_frame.has_value());
        REQUIRE(first_frame->cloud->size() > 100000);  // KITTI通常有10万+点
        
        // 验证位姿维度
        REQUIRE(first_frame->pose.rows() == 4);
        REQUIRE(first_frame->pose.cols() == 4);
    }
}

#endif
```

### 3.5 测试数据最小化策略

为了在CI/CD中运行测试，可以创建极小的参考数据：

```cpp
// test/data/kitti_mini/
namespace kitti_test_data {
    
// 嵌入最小测试数据
struct mini_kitti_data_t {
    // 10个点的二进制数据（40字节）
    static constexpr uint8_t mini_cloud_000000[] = {
        // x, y, z, intensity (每个4字节float)
        0x00, 0x00, 0x80, 0x3F,  // 1.0f
        0x00, 0x00, 0x00, 0x40,  // 2.0f
        0x00, 0x00, 0x40, 0x40,  // 3.0f
        0x00, 0x00, 0x80, 0x3F,  // 1.0f
        // ... 更多点
    };
    
    // 对应的标签数据（10个uint32）
    static constexpr uint32_t mini_labels_000000[] = {
        40, 40, 44, 48, 50, 70, 71, 72, 40, 44
    };
    
    // 一行位姿数据
    static constexpr const char* mini_pose_line = 
        "1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0";
    
    // 创建迷你测试环境
    static void create_mini_dataset(const std::string& output_path) {
        // 创建目录结构
        std::filesystem::create_directories(output_path + "/velodyne");
        std::filesystem::create_directories(output_path + "/labels");
        
        // 写入迷你点云
        std::ofstream cloud_file(output_path + "/velodyne/000000.bin", 
                                std::ios::binary);
        cloud_file.write(reinterpret_cast<const char*>(mini_cloud_000000), 
                        sizeof(mini_cloud_000000));
        
        // 写入标签
        std::ofstream label_file(output_path + "/labels/000000.label", 
                                std::ios::binary);
        label_file.write(reinterpret_cast<const char*>(mini_labels_000000), 
                        sizeof(mini_labels_000000));
        
        // 写入位姿
        std::ofstream pose_file(output_path + "/../poses/00.txt");
        pose_file << mini_pose_line << "\n";
    }
};

} // namespace kitti_test_data
```

## 4. CMake测试配置

```cmake
# test/CMakeLists.txt 添加
option(KITTI_ENABLE_INTEGRATION_TESTS "Enable KITTI integration tests" OFF)
set(KITTI_TEST_DATA_PATH "" CACHE PATH "Path to KITTI test data")

# 编译测试辅助库
add_library(kitti_test_utils INTERFACE)
target_include_directories(kitti_test_utils INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/pcl)

# 添加单元测试
set(KITTI_TEST_FILES
    pcl/kitti_dataset_test.cpp
    pcl/kitti_utils_test.cpp
    pcl/kitti_pose_reader_test.cpp
    pcl/kitti_label_reader_test.cpp
)

# 如果启用集成测试
if(KITTI_ENABLE_INTEGRATION_TESTS AND KITTI_TEST_DATA_PATH)
    list(APPEND KITTI_TEST_FILES pcl/kitti_integration_test.cpp)
    target_compile_definitions(cpp-toolbox_test PRIVATE 
        KITTI_TEST_DATA_PATH="${KITTI_TEST_DATA_PATH}")
endif()

# 添加性能测试
list(APPEND BENCHMARK_FILES
    pcl/kitti_loader_bench.cpp
)
```

## 5. 持续集成配置

```yaml
# .github/workflows/test.yml
- name: Run KITTI unit tests
  run: |
    ./build/test/cpp-toolbox_test "[kitti]"
    
- name: Run KITTI benchmarks
  run: |
    ./build/benchmark/cpp-toolbox_benchmark "[kitti]" --benchmark-samples 10
```

## 6. 测试覆盖清单

### 必须测试的功能
- [x] 基本数据加载
- [x] 空数据集处理
- [x] 单帧/多帧访问
- [x] 迭代器功能
- [x] 缓存机制
- [x] Skip参数
- [x] 相对变换计算
- [x] 标签读取
- [x] 标签过滤
- [x] 错误处理
- [x] 文件格式验证
- [x] 内存管理
- [x] 线程安全（如果支持）

### 边界情况
- [x] 不存在的文件
- [x] 损坏的文件
- [x] 空文件
- [x] 格式错误
- [x] 索引越界
- [x] 内存不足
- [x] 权限问题

### 性能测试
- [x] 加载速度
- [x] 缓存效率
- [x] 内存使用
- [x] 并发访问
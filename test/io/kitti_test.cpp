#include <filesystem>
#include <limits>  // numeric_limits
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <cpp-toolbox/io/formats/kitti.hpp>
#include <cpp-toolbox/types/minmax.hpp>  // For minmax_t
#include <cpp-toolbox/types/point.hpp>  // point_cloud_t, point_t
#include <cpp-toolbox/types/point_utils.hpp>  // For generate_random_points_parallel

#include "test_data_dir.hpp"

using namespace toolbox::io::formats;
using Catch::Matchers::Equals;
using Catch::Matchers::WithinAbs;
using toolbox::io::base_file_data_t;
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;

// Helper function to compare two point_t with tolerance
template<typename T>
struct PointMatcher : Catch::Matchers::MatcherBase<point_t<T>>
{
  point_t<T> m_expected;
  double m_tolerance;

  PointMatcher(const point_t<T>& expected, double tolerance)
      : m_expected(expected)
      , m_tolerance(tolerance)
  {
  }

  bool match(const point_t<T>& actual) const override
  {
    return std::abs(actual.x - m_expected.x) < m_tolerance
        && std::abs(actual.y - m_expected.y) < m_tolerance
        && std::abs(actual.z - m_expected.z) < m_tolerance;
  }

  std::string describe() const override
  {
    std::ostringstream ss;
    ss << "is within tolerance " << m_tolerance << " of { " << m_expected.x
       << ", " << m_expected.y << ", " << m_expected.z << " }";
    return ss.str();
  }
};

// Helper to use the matcher
template<typename T>
inline PointMatcher<T> IsPointClose(const point_t<T>& expected,
                                    double tolerance)
{
  return PointMatcher<T>(expected, tolerance);
}

TEST_CASE("KITTI Format Reading", "[io][kitti][read]")
{
  kitti_format_t kitti_reader;
  std::unique_ptr<base_file_data_t> data;

  SECTION("Supported Extensions")
  {
    REQUIRE(kitti_reader.get_supported_extensions()
            == std::vector<std::string> {".bin"});
  }

  SECTION("Can Read Check")
  {
    REQUIRE(kitti_reader.can_read("some_file.bin"));
    REQUIRE_FALSE(kitti_reader.can_read("some_file.txt"));
    REQUIRE_FALSE(kitti_reader.can_read("another_file.pcd"));
  }

  SECTION("Reading non-existent file")
  {
    REQUIRE_FALSE(kitti_reader.read("non_existent_file.bin", data));
    REQUIRE(data == nullptr);
  }

#if !defined(CPP_TOOLBOX_COMPILER_ZIG)
  // 测试文件/Test file
  const std::string test_file = "000000.bin";

  SECTION("Reading valid file: " + test_file)
  {
    std::filesystem::path file_path =
        std::filesystem::path(test_data_dir) / test_file;
    LOG_INFO_F("Reading file: {}", file_path.string());
    REQUIRE(std::filesystem::exists(file_path));  // 预检查/Pre-check

    data = nullptr;  // 重置数据指针/Reset data ptr
    bool success = kitti_reader.read(file_path.string(), data);

    INFO("File: " << file_path.string());
    REQUIRE(success);
    REQUIRE(data != nullptr);

    // 尝试转换为预期类型（默认为float）/Attempt to cast to the expected type (float by default)
    auto* cloud_ptr = dynamic_cast<point_cloud_t<float>*>(data.get());
    REQUIRE(cloud_ptr != nullptr);

    // 基本验证：检查是否读取了任何点/Basic validation: check if any points were read
    CHECK_FALSE(cloud_ptr->points.empty());
    CHECK(cloud_ptr->size() > 0);

    // 打印点云大小/Print point cloud size
    LOG_INFO_F("Read {} points from KITTI file", cloud_ptr->size());
  }
#endif
}

TEST_CASE("KITTI Format Writing and Reading Back", "[io][kitti][write]")
{
  kitti_format_t kitti_handler;
  const std::string temp_path = "temp_test_kitti.bin";
  const double tolerance = 1e-6;

  // 清理可能的上次运行残留/Cleanup potential leftovers from previous runs
  std::filesystem::remove(temp_path);

  SECTION("Writing and Reading Back <float>")
  {
    auto original_cloud_f = std::make_unique<point_cloud_t<float>>();
    original_cloud_f->points = {{1.1f, 2.2f, 3.3f}, {4.4f, 5.5f, 6.6f}};

    // KITTI格式不支持法线和颜色，只有点坐标和强度
    // KITTI format doesn't support normals and colors, only point coordinates and intensity

    std::unique_ptr<base_file_data_t> original_data_f =
        std::move(original_cloud_f);

    SECTION("Binary Mode (KITTI is always binary)")
    {
      REQUIRE(kitti_handler.write(
          temp_path, original_data_f, true));  // 写入二进制/Write Binary

      std::unique_ptr<base_file_data_t> read_data;
      REQUIRE(kitti_handler.read(temp_path, read_data));
      REQUIRE(read_data != nullptr);

      auto* read_cloud = dynamic_cast<point_cloud_t<float>*>(read_data.get());
      REQUIRE(read_cloud != nullptr);
      auto* original_cloud_ptr = dynamic_cast<point_cloud_t<float>*>(
          original_data_f.get());  // 获取指针/Get ptr back

      REQUIRE(read_cloud->size() == original_cloud_ptr->size());
      REQUIRE(read_cloud->points.size() == original_cloud_ptr->points.size());

      for (size_t i = 0; i < read_cloud->size(); ++i) {
        CHECK_THAT(read_cloud->points[i],
                   IsPointClose(original_cloud_ptr->points[i], tolerance));
      }
      std::filesystem::remove(temp_path);
    }
  }

  SECTION("Writing and Reading Back <double>")
  {
    auto original_cloud_d = std::make_unique<point_cloud_t<double>>();
    original_cloud_d->points = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};

    std::unique_ptr<base_file_data_t> original_data_d =
        std::move(original_cloud_d);

    SECTION("Binary Mode (KITTI is always binary)")
    {
      REQUIRE(kitti_handler.write(temp_path, original_data_d, true));

      std::unique_ptr<base_file_data_t> read_data;
      REQUIRE(kitti_handler.read(temp_path, read_data));
      REQUIRE(read_data != nullptr);

      auto* read_cloud = dynamic_cast<point_cloud_t<float>*>(read_data.get());
      REQUIRE(read_cloud != nullptr);
      auto* original_cloud_ptr =
          dynamic_cast<point_cloud_t<double>*>(original_data_d.get());

      REQUIRE(read_cloud->size() == original_cloud_ptr->size());

      for (size_t i = 0; i < read_cloud->size(); ++i) {
        // 比较读取的float值与原始double值
        // Compare read float values to original double values
        CHECK_THAT(read_cloud->points[i].x,
                   WithinAbs(original_cloud_ptr->points[i].x, tolerance));
        CHECK_THAT(read_cloud->points[i].y,
                   WithinAbs(original_cloud_ptr->points[i].y, tolerance));
        CHECK_THAT(read_cloud->points[i].z,
                   WithinAbs(original_cloud_ptr->points[i].z, tolerance));
      }
      std::filesystem::remove(temp_path);
    }
  }

  SECTION("Writing null data")
  {
    std::unique_ptr<base_file_data_t> null_data = nullptr;
    REQUIRE_FALSE(kitti_handler.write("null_test.bin", null_data, true));
    REQUIRE_FALSE(std::filesystem::exists(
        "null_test.bin"));  // 确保没有创建文件/Ensure no file was created
  }

  // 最终清理，以防某些部分在删除前失败
  // Final cleanup in case some sections failed before remove
  std::filesystem::remove(temp_path);
  std::filesystem::remove("null_test.bin");
}

TEST_CASE("KITTI Format Large Point Cloud Handling", "[io][kitti][large]")
{
  kitti_format_t kitti_handler;
  const std::string temp_large_path = "temp_large_kitti.bin";
  const size_t num_large_points =
      100000;  // 定义大型测试的点数/Define number of points for large test

  // 清理潜在的残留/Cleanup potential leftovers
  std::filesystem::remove(temp_large_path);

  // 生成大型点云数据/Generate large point cloud data
  auto large_cloud = std::make_unique<point_cloud_t<float>>();
  toolbox::types::minmax_t<point_t<float>> bounds({-10.0f, -10.0f, -10.0f},
                                                {10.0f, 10.0f, 10.0f});
  // 使用并行生成提高效率/Use parallel generation for efficiency
  large_cloud->points = toolbox::types::generate_random_points_parallel<float>(
      num_large_points, bounds);

  LOG_INFO_F("Test on large cloud size: {}", large_cloud->size());

  REQUIRE(large_cloud->size() == num_large_points);
  std::unique_ptr<base_file_data_t> large_data = std::move(large_cloud);

  SECTION("Binary Large File - Check Count Only")
  {
    REQUIRE(kitti_handler.write(
        temp_large_path, large_data, true));  // 写入二进制/Write Binary

    LOG_INFO_F("Wrote large file to: {}", temp_large_path);

    const auto file_size = toolbox::file::get_file_size(temp_large_path);
    const auto file_size_mb = static_cast<double>(file_size) / 1024.0 / 1024.0;
    LOG_INFO_F("Binary File size: {} MB", file_size_mb);

    std::unique_ptr<base_file_data_t> read_data;
    REQUIRE(kitti_handler.read(temp_large_path, read_data));
    REQUIRE(read_data != nullptr);

    auto* read_cloud = dynamic_cast<point_cloud_t<float>*>(read_data.get());
    REQUIRE(read_cloud != nullptr);

    // 只检查读回的点数/Only check the number of points read back
    REQUIRE(read_cloud->size() == num_large_points);

    std::filesystem::remove(temp_large_path);
  }

  // 最终清理/Final cleanup
  std::filesystem::remove(temp_large_path);
}

TEST_CASE("KITTI Standalone Helper Functions", "[io][kitti][standalone]")
{
  const std::string temp_standalone_float_path = "temp_standalone_float.bin";
  const std::string temp_standalone_double_path = "temp_standalone_double.bin";
  const double tolerance = 1e-6;

  // 清理潜在的残留/Cleanup potential leftovers
  std::filesystem::remove(temp_standalone_float_path);
  std::filesystem::remove(temp_standalone_double_path);

  SECTION("Testing standalone float functions")
  {
    point_cloud_t<float> original_cloud;
    original_cloud.points = {{10.1f, 10.2f, 10.3f}, {-1.1f, -2.2f, -3.3f}};

    // 测试写入/Test write
    REQUIRE(toolbox::io::formats::write_kitti_bin(
        temp_standalone_float_path, original_cloud));

    // 测试读取/Test read
    std::unique_ptr<point_cloud_t<float>> read_cloud_ptr =
        toolbox::io::formats::read_kitti_bin<float>(temp_standalone_float_path);

    REQUIRE(read_cloud_ptr != nullptr);
    REQUIRE(read_cloud_ptr->size() == original_cloud.size());

    // 验证内容/Verify content
    REQUIRE(read_cloud_ptr->points.size() == original_cloud.points.size());
    for (size_t i = 0; i < original_cloud.size(); ++i) {
      CHECK_THAT(read_cloud_ptr->points[i],
                 IsPointClose(original_cloud.points[i], tolerance));
    }

    std::filesystem::remove(temp_standalone_float_path);
  }

  SECTION("Testing standalone double functions")
  {
    point_cloud_t<double> original_cloud;
    original_cloud.points = {{20.1, 20.2, 20.3}, {-4.4, -5.5, -6.6}};

    // 测试写入/Test write
    REQUIRE(toolbox::io::formats::write_kitti_bin(
        temp_standalone_double_path, original_cloud));

    // 测试读取/Test read
    std::unique_ptr<point_cloud_t<float>> read_cloud_float_ptr =
        toolbox::io::formats::read_kitti_bin<float>(temp_standalone_double_path);
    REQUIRE(read_cloud_float_ptr != nullptr);
    REQUIRE(read_cloud_float_ptr->size() == original_cloud.size());

    // 验证内容（比较float读取数据与原始double数据）
    // Verify content (comparing float read data to original double data)
    REQUIRE(read_cloud_float_ptr->points.size()
            == original_cloud.points.size());
    for (size_t i = 0; i < original_cloud.size(); ++i) {
      CHECK_THAT(read_cloud_float_ptr->points[i].x,
                 WithinAbs(original_cloud.points[i].x, tolerance));
      CHECK_THAT(read_cloud_float_ptr->points[i].y,
                 WithinAbs(original_cloud.points[i].y, tolerance));
      CHECK_THAT(read_cloud_float_ptr->points[i].z,
                 WithinAbs(original_cloud.points[i].z, tolerance));
    }

    std::filesystem::remove(temp_standalone_double_path);
  }

  // 最终清理/Final cleanup
  std::filesystem::remove(temp_standalone_float_path);
  std::filesystem::remove(temp_standalone_double_path);
}
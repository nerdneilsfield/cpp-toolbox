#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <cpp-toolbox/math/statistics.hpp>

using namespace toolbox::math;
using Catch::Matchers::Equals;
using Catch::Matchers::WithinAbs;

// 辅助函数：比较两个向量是否相等（考虑浮点误差）/ Compare two vectors for
// equality within a tolerance
template<typename T>
bool vectors_equal(const std::vector<T>& v1,
                   const std::vector<T>& v2,
                   double tolerance = 1e-6)
{
  if (v1.size() != v2.size())
    return false;
  for (size_t i = 0; i < v1.size(); ++i) {
    if (std::abs(static_cast<double>(v1[i]) - static_cast<double>(v2[i]))
        > tolerance)
    {
      return false;
    }
  }
  return true;
}

// 辅助宏：测试函数在空容器上的行为 / Test that a function throws when given an
// empty container
#define TEST_EMPTY_CONTAINER_THROWS(func_name) \
  SECTION("Empty container throws") \
  { \
    std::vector<int> empty_vec; \
    REQUIRE_THROWS_AS(func_name(empty_vec), std::invalid_argument); \
  }

TEST_CASE("Statistics - Mean", "[math][statistics][mean]")
{
  SECTION("Integer vector")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    REQUIRE_THAT(mean(data), WithinAbs(3.0, 1e-6));
  }

  SECTION("Double vector")
  {
    std::vector<double> data = {1.5, 2.5, 3.5, 4.5, 5.5};
    REQUIRE_THAT(mean(data), WithinAbs(3.5, 1e-6));
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE_THAT(mean(data), WithinAbs(42.0, 1e-6));
  }

  SECTION("Negative values")
  {
    std::vector<int> data = {-5, -3, -1, 1, 3, 5};
    REQUIRE_THAT(mean(data), WithinAbs(0.0, 1e-6));
  }

  TEST_EMPTY_CONTAINER_THROWS(mean);
}

TEST_CASE("Statistics - Median", "[math][statistics][median]")
{
  SECTION("Odd number of elements")
  {
    std::vector<int> data = {5, 2, 8, 1, 3};
    REQUIRE_THAT(median(data), WithinAbs(3.0, 1e-6));
  }

  SECTION("Even number of elements")
  {
    std::vector<int> data = {5, 2, 8, 1, 3, 9};
    REQUIRE_THAT(median(data), WithinAbs(4.0, 1e-6));  // (3+5)/2 = 4
  }

  SECTION("Already sorted")
  {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    REQUIRE_THAT(median(data), WithinAbs(3.0, 1e-6));
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE_THAT(median(data), WithinAbs(42.0, 1e-6));
  }

  SECTION("Two elements")
  {
    std::vector<int> data = {1, 3};
    REQUIRE_THAT(median(data), WithinAbs(2.0, 1e-6));
  }

  TEST_EMPTY_CONTAINER_THROWS(median);
}

TEST_CASE("Statistics - Mode", "[math][statistics][mode]")
{
  SECTION("Single mode")
  {
    std::vector<int> data = {1, 2, 2, 3, 4};
    auto result = mode(data);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 2);
  }

  SECTION("Multiple modes")
  {
    std::vector<int> data = {1, 2, 2, 3, 3, 4};
    auto result = mode(data);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 2);
    REQUIRE(result[1] == 3);
  }

  SECTION("All elements unique")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto result = mode(data);
    REQUIRE(result.size() == 5);
    // 所有元素都是众数，应该按顺序返回
    REQUIRE(vectors_equal(result, std::vector<int> {1, 2, 3, 4, 5}));
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    auto result = mode(data);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 42);
  }

  SECTION("Empty container")
  {
    std::vector<int> empty_vec;
    auto result = mode(empty_vec);
    REQUIRE(result.empty());
  }
}

TEST_CASE("Statistics - Variance", "[math][statistics][variance]")
{
  SECTION("Sample variance")
  {
    std::vector<double> data = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    // 样本方差 = sum((x_i - mean)^2) / (n-1)
    // mean = 5.0, sum((x_i - 5)^2) = 32, n-1 = 7
    // 样本方差 = 32/7 ≈ 4.57142857143
    REQUIRE_THAT(variance(data), WithinAbs(4.57142857143, 1e-6));
  }

  SECTION("Population variance")
  {
    std::vector<double> data = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    // 总体方差 = sum((x_i - mean)^2) / n
    // mean = 5.0, sum((x_i - 5)^2) = 32, n = 8
    // 总体方差 = 32/8 = 4.0
    REQUIRE_THAT(variance(data, false), WithinAbs(4.0, 1e-6));
  }

  SECTION("Single element - population variance")
  {
    std::vector<int> data = {42};
    // 单个元素的总体方差为0
    REQUIRE_THAT(variance(data, false), WithinAbs(0.0, 1e-6));
  }

  SECTION("Single element - sample variance throws")
  {
    std::vector<int> data = {42};
    REQUIRE_THROWS_AS(variance(data), std::invalid_argument);
  }

  SECTION("Two elements - sample variance")
  {
    std::vector<int> data = {2, 4};
    // 样本方差 = ((2-3)^2 + (4-3)^2) / (2-1) = 2
    REQUIRE_THAT(variance(data), WithinAbs(2.0, 1e-6));
  }

  SECTION("Empty container - both types throw")
  {
    std::vector<int> empty_vec;
    REQUIRE_THROWS_AS(variance(empty_vec), std::invalid_argument);
    REQUIRE_THROWS_AS(variance(empty_vec, false), std::invalid_argument);
  }
}

TEST_CASE("Statistics - Standard Deviation", "[math][statistics][stdev]")
{
  SECTION("Sample standard deviation")
  {
    std::vector<double> data = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    // 样本标准差 = sqrt(样本方差) = sqrt(4.57142857143) ≈ 2.13809
    REQUIRE_THAT(stdev(data), WithinAbs(2.13809, 1e-5));
  }

  SECTION("Population standard deviation")
  {
    std::vector<double> data = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    // 总体标准差 = sqrt(总体方差) = sqrt(4.0) = 2.0
    REQUIRE_THAT(stdev(data, false), WithinAbs(2.0, 1e-5));
  }

  SECTION("Single element - population stdev")
  {
    std::vector<int> data = {42};
    REQUIRE_THAT(stdev(data, false), WithinAbs(0.0, 1e-6));
  }

  SECTION("Single element - sample stdev throws")
  {
    std::vector<int> data = {42};
    REQUIRE_THROWS_AS(stdev(data), std::invalid_argument);
  }

  // 空容器测试由 variance 函数处理
}

TEST_CASE("Statistics - Sum", "[math][statistics][sum]")
{
  SECTION("Integer sum")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    REQUIRE(sum(data) == 15);
  }

  SECTION("Double sum")
  {
    std::vector<double> data = {1.1, 2.2, 3.3, 4.4, 5.5};
    REQUIRE_THAT(sum(data), WithinAbs(16.5, 1e-6));
  }

  SECTION("Negative values")
  {
    std::vector<int> data = {-5, -3, -1, 1, 3, 5};
    REQUIRE(sum(data) == 0);
  }

  SECTION("Empty container")
  {
    std::vector<int> empty_vec;
    REQUIRE(sum(empty_vec) == 0);
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE(sum(data) == 42);
  }
}

TEST_CASE("Statistics - Sum Double", "[math][statistics][sum_d]")
{
  SECTION("Integer sum as double")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    REQUIRE_THAT(sum_d(data), WithinAbs(15.0, 1e-6));
  }

  SECTION("Double sum")
  {
    std::vector<double> data = {1.1, 2.2, 3.3, 4.4, 5.5};
    REQUIRE_THAT(sum_d(data), WithinAbs(16.5, 1e-6));
  }

  SECTION("Large integers")
  {
    // 测试可能导致整数溢出的情况
    std::vector<int> data(1000, 1000000);  // 1000个1,000,000
    REQUIRE_THAT(sum_d(data), WithinAbs(1000.0 * 1000000.0, 1e-6));
  }

  SECTION("Empty container")
  {
    std::vector<int> empty_vec;
    REQUIRE_THAT(sum_d(empty_vec), WithinAbs(0.0, 1e-6));
  }
}

TEST_CASE("Statistics - Min", "[math][statistics][min]")
{
  SECTION("Integer min")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    REQUIRE(min(data) == 1);
  }

  SECTION("Double min")
  {
    std::vector<double> data = {5.5, 3.3, 8.8, 1.1, 9.9, 2.2};
    REQUIRE_THAT(min(data), WithinAbs(1.1, 1e-6));
  }

  SECTION("Negative values")
  {
    std::vector<int> data = {-5, -10, -3, -8, -1};
    REQUIRE(min(data) == -10);
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE(min(data) == 42);
  }

  TEST_EMPTY_CONTAINER_THROWS(min);
}

TEST_CASE("Statistics - Max", "[math][statistics][max]")
{
  SECTION("Integer max")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    REQUIRE(max(data) == 9);
  }

  SECTION("Double max")
  {
    std::vector<double> data = {5.5, 3.3, 8.8, 1.1, 9.9, 2.2};
    REQUIRE_THAT(max(data), WithinAbs(9.9, 1e-6));
  }

  SECTION("Negative values")
  {
    std::vector<int> data = {-5, -10, -3, -8, -1};
    REQUIRE(max(data) == -1);
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE(max(data) == 42);
  }

  TEST_EMPTY_CONTAINER_THROWS(max);
}

TEST_CASE("Statistics - Range", "[math][statistics][range]")
{
  SECTION("Integer range")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    REQUIRE(range(data) == 8);  // 9 - 1 = 8
  }

  SECTION("Double range")
  {
    std::vector<double> data = {5.5, 3.3, 8.8, 1.1, 9.9, 2.2};
    REQUIRE_THAT(range(data), WithinAbs(8.8, 1e-6));  // 9.9 - 1.1 = 8.8
  }

  SECTION("Negative values")
  {
    std::vector<int> data = {-5, -10, -3, -8, -1};
    REQUIRE(range(data) == 9);  // -1 - (-10) = 9
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE(range(data) == 0);  // max - min = 42 - 42 = 0
  }

  TEST_EMPTY_CONTAINER_THROWS(range);
}

TEST_CASE("Statistics - Percentile", "[math][statistics][percentile]")
{
  SECTION("Median (50th percentile)")
  {
    std::vector<int> data = {15, 20, 35, 40, 50};
    REQUIRE_THAT(percentile(data, 0.5), WithinAbs(35.0, 1e-6));
  }

  SECTION("First quartile (25th percentile)")
  {
    std::vector<int> data = {15, 20, 35, 40, 50};
    REQUIRE_THAT(percentile(data, 0.25), WithinAbs(20.0, 1e-6));
  }

  SECTION("Third quartile (75th percentile)")
  {
    std::vector<int> data = {15, 20, 35, 40, 50};
    REQUIRE_THAT(percentile(data, 0.75), WithinAbs(40.0, 1e-6));
  }

  SECTION("Minimum (0th percentile)")
  {
    std::vector<int> data = {15, 20, 35, 40, 50};
    REQUIRE_THAT(percentile(data, 0.0), WithinAbs(15.0, 1e-6));
  }

  SECTION("Maximum (100th percentile)")
  {
    std::vector<int> data = {15, 20, 35, 40, 50};
    REQUIRE_THAT(percentile(data, 1.0), WithinAbs(50.0, 1e-6));
  }

  SECTION("Interpolation")
  {
    std::vector<int> data = {10, 20, 30, 40, 50};
    // 30% 应该在 10 和 20 之间插值
    // index = 0.3 * 4 = 1.2
    // 10 + (1.2 - 1) * (20 - 10) = 10 + 0.2 * 10 = 12
    // 但实际实现是：
    // index = 0.3 * 4 = 1.2
    // lower_index = floor(1.2) = 1, upper_index = ceil(1.2) = 2
    // lower_val = data[1] = 20, upper_val = data[2] = 30
    // 20 + (1.2 - 1) * (30 - 20) = 20 + 0.2 * 10 = 22
    REQUIRE_THAT(percentile(data, 0.3), WithinAbs(22.0, 1e-6));
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    REQUIRE_THAT(percentile(data, 0.5), WithinAbs(42.0, 1e-6));
  }

  SECTION("Invalid percentile values")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    REQUIRE_THROWS_AS(percentile(data, -0.1), std::out_of_range);
    REQUIRE_THROWS_AS(percentile(data, 1.1), std::out_of_range);
  }

  SECTION("Empty container throws")
  {
    std::vector<int> empty_vec;
    REQUIRE_THROWS_AS(percentile(empty_vec, 0.5), std::invalid_argument);
  }
}

TEST_CASE("Statistics - Min K Elements", "[math][statistics][min_k]")
{
  SECTION("Basic min_k")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = min_k(data, 3);
    REQUIRE(result.size() == 3);
    REQUIRE(vectors_equal(result, std::vector<int> {1, 2, 3}));
  }

  SECTION("k equals container size")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = min_k(data, 6);
    REQUIRE(result.size() == 6);
    REQUIRE(vectors_equal(result, std::vector<int> {1, 2, 3, 5, 8, 9}));
  }

  SECTION("k greater than container size")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = min_k(data, 10);
    REQUIRE(result.size() == 6);
    REQUIRE(vectors_equal(result, std::vector<int> {1, 2, 3, 5, 8, 9}));
  }

  SECTION("k = 0")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = min_k(data, 0);
    REQUIRE(result.empty());
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    auto result = min_k(data, 1);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 42);
  }

  SECTION("Empty container with k > 0")
  {
    std::vector<int> empty_vec;
    REQUIRE_THROWS_AS(min_k(empty_vec, 1), std::invalid_argument);
  }
}

TEST_CASE("Statistics - Max K Elements", "[math][statistics][max_k]")
{
  SECTION("Basic max_k")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = max_k(data, 3);
    REQUIRE(result.size() == 3);
    REQUIRE(vectors_equal(result, std::vector<int> {9, 8, 5}));
  }

  SECTION("k equals container size")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = max_k(data, 6);
    REQUIRE(result.size() == 6);
    REQUIRE(vectors_equal(result, std::vector<int> {9, 8, 5, 3, 2, 1}));
  }

  SECTION("k greater than container size")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = max_k(data, 10);
    REQUIRE(result.size() == 6);
    REQUIRE(vectors_equal(result, std::vector<int> {9, 8, 5, 3, 2, 1}));
  }

  SECTION("k = 0")
  {
    std::vector<int> data = {5, 3, 8, 1, 9, 2};
    auto result = max_k(data, 0);
    REQUIRE(result.empty());
  }

  SECTION("Single element")
  {
    std::vector<int> data = {42};
    auto result = max_k(data, 1);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 42);
  }

  SECTION("Empty container with k > 0")
  {
    std::vector<int> empty_vec;
    REQUIRE_THROWS_AS(max_k(empty_vec, 1), std::invalid_argument);
  }
}

TEST_CASE("Statistics - Edge Cases", "[math][statistics][edge]")
{
  SECTION("Extreme values")
  {
    std::vector<double> data = {std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::min(),
                                std::numeric_limits<double>::lowest()};

    // 这些测试主要是确保函数不会崩溃
    REQUIRE_NOTHROW(mean(data));
    REQUIRE_NOTHROW(median(data));
    REQUIRE_NOTHROW(mode(data));
    REQUIRE_NOTHROW(variance(data, false));  // 总体方差
    REQUIRE_NOTHROW(stdev(data, false));  // 总体标准差
    REQUIRE_NOTHROW(sum_d(data));  // 使用 sum_d 而不是 sum 避免溢出
    REQUIRE_NOTHROW(min(data));
    REQUIRE_NOTHROW(max(data));
    REQUIRE_NOTHROW(range(data));
    REQUIRE_NOTHROW(percentile(data, 0.5));
    REQUIRE_NOTHROW(min_k(data, 2));
    REQUIRE_NOTHROW(max_k(data, 2));
  }
}

// 清理宏定义，避免影响其他文件
#undef TEST_EMPTY_CONTAINER_THROWS
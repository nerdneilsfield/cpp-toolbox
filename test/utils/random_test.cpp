#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <cpp-toolbox/utils/random.hpp>

using namespace toolbox::utils;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// 辅助函数：检查值是否在指定范围内
template<typename T>
bool is_in_range(T value, T min, T max)
{
  return value >= min && value <= max;
}

// 辅助函数：计算均值
template<typename Container>
auto mean(const Container& values)
{
  using ValueType = typename Container::value_type;
  return std::accumulate(values.begin(), values.end(), ValueType {})
      / static_cast<ValueType>(values.size());
}

// 辅助函数：计算标准差
template<typename Container>
auto standard_deviation(const Container& values,
                        typename Container::value_type mean_val)
{
  using ValueType = typename Container::value_type;
  ValueType sum_sq =
      std::accumulate(values.begin(),
                      values.end(),
                      ValueType {},
                      [mean_val](ValueType sum, ValueType val)
                      { return sum + (val - mean_val) * (val - mean_val); });
  return std::sqrt(sum_sq / static_cast<ValueType>(values.size()));
}

TEST_CASE("Random_t Basic Functionality", "[utils][random][basic]")
{
  SECTION("Constructor and Seeding")
  {
    // 默认构造函数
    random_t random1;

    // 使用特定种子构造
    random_t random2(42);
    random_t random3(42);

    // 使用相同种子的两个实例应该生成相同的随机数序列
    REQUIRE(random2.randint(1, 1000) == random3.randint(1, 1000));

    // 重新设置种子
    random3.seed(100);
    REQUIRE_FALSE(random2.randint(1, 1000) == random3.randint(1, 1000));
  }

  SECTION("Singleton Instance")
  {
    // 测试单例实例
    auto& instance1 = random_t::instance();
    auto& instance2 = random_t::instance();

    // 应该是同一个实例
    REQUIRE(&instance1 == &instance2);

    // 设置种子并测试
    instance1.seed(123);
    int val1 = instance1.randint(1, 1000);
    instance1.seed(123);
    int val2 = instance2.randint(1, 1000);

    REQUIRE(val1 == val2);
  }

  SECTION("randint Function")
  {
    random_t random(42);  // 固定种子以获得可重复的结果

    // 测试范围
    for (int i = 0; i < 100; ++i) {
      int val = random.randint(10, 20);
      REQUIRE(is_in_range(val, 10, 20));
    }

    // 测试边界情况
    for (int i = 0; i < 10; ++i) {
      int val = random.randint(5, 5);
      REQUIRE(val == 5);
    }

    // 测试参数顺序不重要
    random.seed(42);
    int val1 = random.randint(10, 20);
    random.seed(42);
    int val2 = random.randint(20, 10);
    REQUIRE(val1 == val2);
  }
}

TEST_CASE("Random_t Numeric Type Tests", "[utils][random][types]")
{
  random_t random(42);  // 固定种子

  SECTION("Integer Types")
  {
    // int
    for (int i = 0; i < 100; ++i) {
      int val = random.random<int>(10, 20);
      REQUIRE(is_in_range(val, 10, 20));
    }

    // long
    for (int i = 0; i < 100; ++i) {
      long val = random.random<long>(10L, 20L);
      REQUIRE(is_in_range(val, 10L, 20L));
    }

    // unsigned int
    for (int i = 0; i < 100; ++i) {
      unsigned int val = random.random<unsigned int>(10U, 20U);
      REQUIRE(is_in_range(val, 10U, 20U));
    }
  }

  SECTION("Floating Point Types")
  {
    // float
    for (int i = 0; i < 100; ++i) {
      float val = random.random<float>(10.0f, 20.0f);
      REQUIRE(is_in_range(val, 10.0f, 20.0f));
    }

    // double
    for (int i = 0; i < 100; ++i) {
      double val = random.random<double>(10.0, 20.0);
      REQUIRE(is_in_range(val, 10.0, 20.0));
    }
  }

  SECTION("No-argument random")
  {
    // int - 应该在[0, INT_MAX]范围内
    for (int i = 0; i < 100; ++i) {
      int val = random.random<int>();
      REQUIRE(is_in_range(val, 0, std::numeric_limits<int>::max()));
    }

    // float - 应该在[0, 1]范围内
    for (int i = 0; i < 100; ++i) {
      float val = random.random<float>();
      REQUIRE(is_in_range(val, 0.0f, 1.0f));
    }
  }
}

TEST_CASE("Random_t Distribution Tests", "[utils][random][distribution]")
{
  random_t random(42);  // 固定种子
  const size_t sample_size = 10000;

  SECTION("Uniform Distribution")
  {
    std::vector<double> samples;
    samples.reserve(sample_size);

    // 生成[0, 1]范围内的均匀分布样本
    for (size_t i = 0; i < sample_size; ++i) {
      samples.push_back(random.uniform<double>(0.0, 1.0));
    }

    // 计算均值和标准差
    double mean_val = mean(samples);
    double std_dev = standard_deviation(samples, mean_val);

    // 均匀分布在[0,1]上的理论均值为0.5，标准差约为0.289
    CHECK_THAT(mean_val, WithinAbs(0.5, 0.05));
    CHECK_THAT(std_dev, WithinAbs(0.289, 0.05));
  }

  SECTION("Gaussian Distribution")
  {
    std::vector<double> samples;
    samples.reserve(sample_size);

    // 生成均值为0，标准差为1的高斯分布样本
    for (size_t i = 0; i < sample_size; ++i) {
      samples.push_back(random.gauss<double>(0.0, 1.0));
    }

    // 计算均值和标准差
    double mean_val = mean(samples);
    double std_dev = standard_deviation(samples, mean_val);

    // 检查均值和标准差是否接近预期值
    CHECK_THAT(mean_val, WithinAbs(0.0, 0.1));
    CHECK_THAT(std_dev, WithinAbs(1.0, 0.1));
  }
}

TEST_CASE("Random_t Container Functions", "[utils][random][container]")
{
  random_t random(42);  // 固定种子

  SECTION("Choice Function")
  {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // 测试choice函数
    for (int i = 0; i < 100; ++i) {
      int val = random.choice(vec);
      REQUIRE(std::find(vec.begin(), vec.end(), val) != vec.end());
    }

    // 测试空容器
    std::vector<int> empty_vec;
    REQUIRE_THROWS_AS(random.choice(empty_vec), std::out_of_range);
  }

  SECTION("Sample Function")
  {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 测试sample函数
    auto sample1 = random.sample(vec, 5);
    REQUIRE(sample1.size() == 5);

    // 每个元素应该在原始容器中
    for (int val : sample1) {
      REQUIRE(std::find(vec.begin(), vec.end(), val) != vec.end());
    }

    // 测试k > 容器大小的情况
    REQUIRE_THROWS_AS(random.sample(vec, 20), std::out_of_range);

    // 测试k = 0的情况
    auto sample2 = random.sample(vec, 0);
    REQUIRE(sample2.empty());
  }

  SECTION("Shuffle Function")
  {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);  // 填充1到10
    std::vector<int> original = vec;

    // 洗牌
    random.shuffle(vec);

    // 长度应该保持不变
    REQUIRE(vec.size() == original.size());

    // 内容应该相同但顺序可能不同
    std::sort(vec.begin(), vec.end());
    REQUIRE(vec == original);
  }
}

TEST_CASE("Global Random Functions", "[utils][random][global]")
{
  // 设置单例实例的种子
  random_t::instance().seed(42);

  SECTION("Global randint")
  {
    for (int i = 0; i < 100; ++i) {
      int val = randint(10, 20);
      REQUIRE(is_in_range(val, 10, 20));
    }
  }

  SECTION("Global random")
  {
    // 整数类型
    for (int i = 0; i < 100; ++i) {
      int val = random<int>(10, 20);
      REQUIRE(is_in_range(val, 10, 20));
    }

    // 浮点类型
    for (int i = 0; i < 100; ++i) {
      double val = random<double>(10.0, 20.0);
      REQUIRE(is_in_range(val, 10.0, 20.0));
    }

    // 无参数版本
    for (int i = 0; i < 100; ++i) {
      float val = random<float>();
      REQUIRE(is_in_range(val, 0.0f, 1.0f));
    }
  }

  SECTION("Global uniform")
  {
    for (int i = 0; i < 100; ++i) {
      double val = uniform<double>(10.0, 20.0);
      REQUIRE(is_in_range(val, 10.0, 20.0));
    }
  }

  SECTION("Global gauss")
  {
    // 只测试函数是否运行，不检查分布
    double val = gauss<double>(0.0, 1.0);
    REQUIRE_NOTHROW(val);
  }

  SECTION("Global container functions")
  {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // choice
    int choice_val = choice(vec);
    REQUIRE(std::find(vec.begin(), vec.end(), choice_val) != vec.end());

    // sample
    auto sample_vec = sample(vec, 3);
    REQUIRE(sample_vec.size() == 3);

    // shuffle
    std::vector<int> shuffle_vec = {1, 2, 3, 4, 5};
    shuffle(shuffle_vec);
    REQUIRE(shuffle_vec.size() == 5);
  }
}
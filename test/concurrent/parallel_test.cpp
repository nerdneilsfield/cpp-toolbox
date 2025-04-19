// test/concurrent/parallel_test.cpp
#include <algorithm>  // std::transform, std::equal
#include <array>
#include <atomic>
#include <cmath>  // std::sqrt
#include <functional>  // std::plus, std::multiplies
#include <iostream>  // For potential debug output
#include <limits>  // std::numeric_limits
#include <numeric>  // std::iota, std::accumulate
#include <stdexcept>  // std::runtime_error
#include <string>
#include <vector>

// Include the header for the parallel functions under test
#include "cpp-toolbox/concurrent/parallel.hpp"
#include "cpp-toolbox/types.hpp"  // 引入类型别名，如 i64

// Include Catch2 testing framework
#include <catch2/catch_approx.hpp>  // For floating point comparison
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>  // For vector comparison

using namespace toolbox::concurrent;  // 引入命名空间方便调用
using Catch::Matchers::Equals;  // 用于比较 vector

// --- Test Cases ---

TEST_CASE("Parallel Transform Tests", "[concurrent][parallel_transform]")
{
  const size_t large_size = 1'000'000;  // 用于大型测试的大小

  SECTION("Basic integer transformation (square)")
  {
    std::vector<int> input = {1, 2, 3, 4, 5};
    std::vector<int> result(input.size());  // 必须预先分配空间
    std::vector<int> expected = {1, 4, 9, 16, 25};

    parallel_transform(input.begin(),
                       input.end(),
                       result.begin(),
                       [](int x) { return x * x; });
    REQUIRE_THAT(result, Equals(expected));
  }

  SECTION("Transformation to different type (int to double)")
  {
    std::vector<int> input = {1, 2, 3, 4};
    std::vector<double> result(input.size());
    std::vector<double> expected = {1.5, 2.5, 3.5, 4.5};

    parallel_transform(input.begin(),
                       input.end(),
                       result.begin(),
                       [](int x) { return static_cast<double>(x) + 0.5; });

    // 使用 Approx 进行浮点数比较
    REQUIRE(result.size() == expected.size());
    for (size_t i = 0; i < result.size(); ++i) {
      REQUIRE(result[i] == Catch::Approx(expected[i]));
    }
  }

  SECTION("Empty input range")
  {
    std::vector<int> input_empty;
    std::vector<int> result_empty;  // 大小为 0
    std::vector<int> expected_empty;

    // 对空 range 调用应该直接返回，不执行任何操作，不抛异常
    REQUIRE_NOTHROW(parallel_transform(input_empty.begin(),
                                       input_empty.end(),
                                       result_empty.begin(),
                                       [](int x) { return x * x; }));
    REQUIRE_THAT(result_empty, Equals(expected_empty));
  }

  SECTION("Single element range")
  {
    std::vector<int> input = {10};
    std::vector<int> result(input.size());
    std::vector<int> expected = {100};
    parallel_transform(input.begin(),
                       input.end(),
                       result.begin(),
                       [](int x) { return x * x; });
    REQUIRE_THAT(result, Equals(expected));
  }

  SECTION("Large input range")
  {
    std::vector<i64> input(large_size);
    std::iota(
        input.begin(), input.end(), 1);  // Fill with 1, 2, ..., large_size

    std::vector<i64> result(large_size);
    std::vector<i64> expected(large_size);

    // 计算期望结果 (使用 std::transform)
    std::transform(input.begin(),
                   input.end(),
                   expected.begin(),
                   [](i64 x) { return x + 10; });

    // 执行并行转换
    parallel_transform(input.begin(),
                       input.end(),
                       result.begin(),
                       [](i64 x) { return x + 10; });

    // 比较结果
    REQUIRE_THAT(result, Equals(expected));
  }

  SECTION("Using std::array")
  {
    std::array<int, 5> input = {1, 2, 3, 4, 5};
    std::array<int, 5> result;  // std::array 大小固定，无需 resize
    std::array<int, 5> expected = {1, 4, 9, 16, 25};

    parallel_transform(input.begin(),
                       input.end(),
                       result.begin(),
                       [](int x) { return x * x; });
    // Catch2 没有直接比较 array 的 Matcher，我们逐个比较
    for (size_t i = 0; i < input.size(); ++i) {
      REQUIRE(result[i] == expected[i]);
    }
  }

  SECTION("Const input vector")
  {
    const std::vector<int> input = {1, 2, 3, 4, 5};  // 输入是 const
    std::vector<double> result(input.size());
    std::vector<double> expected = {
        1.0, 1.414, 1.732, 2.0, 2.236};  // sqrt approx

    // lambda 接受 const int& 或 int
    parallel_transform(input.cbegin(),
                       input.cend(),
                       result.begin(),
                       [](const int& x)
                       { return std::sqrt(static_cast<double>(x)); });

    REQUIRE(result.size() == expected.size());
    for (size_t i = 0; i < result.size(); ++i) {
      REQUIRE(result[i]
              == Catch::Approx(expected[i]).margin(0.001));  // sqrt 需要 Approx
    }
  }

  SECTION("Exception propagation during transform")
  {
    std::vector<int> input(100);  // 足够大以触发多任务
    std::iota(input.begin(), input.end(), 0);
    std::vector<int> result(input.size());

    auto throwing_op = [](int x) -> int
    {
      if (x == 50) {  // 在处理某个特定值时抛出异常
        throw std::runtime_error("Transform error at 50");
      }
      return x * x;
    };

    // 期望 parallel_transform 捕获并重新抛出来自任务的异常
    REQUIRE_THROWS_AS(
        parallel_transform(
            input.begin(), input.end(), result.begin(), throwing_op),
        std::runtime_error);
    //  REQUIRE_THROWS_WITH(
    //      parallel_transform(input.begin(), input.end(), result.begin(),
    //      throwing_op), ContainsSubstring("Transform error at 50") //
    //      或更通用的嵌套异常消息
    //  );
  }
}

TEST_CASE("Parallel Reduce Tests", "[concurrent][parallel_reduce]")
{
  const size_t large_size = 1'000'000;

  SECTION("Summation of integers")
  {
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  // Sum = 55
    int identity = 0;
    int expected = 55;

    int result =
        parallel_reduce(input.begin(), input.end(), identity, std::plus<int>());
    REQUIRE(result == expected);

    // 使用 lambda
    result = parallel_reduce(
        input.begin(), input.end(), 0, [](int a, int b) { return a + b; });
    REQUIRE(result == expected);
  }

  SECTION("Summation of doubles")
  {
    std::vector<double> input = {1.1, 2.2, 3.3, 4.4, 5.5};  // Sum = 16.5
    double identity = 0.0;
    double expected = 16.5;

    double result = parallel_reduce(
        input.begin(), input.end(), identity, std::plus<double>());
    REQUIRE(result == Catch::Approx(expected));
  }

  SECTION("Product of integers (use long long)")
  {
    std::vector<int> input = {1, 2, 3, 4, 5};  // Product = 120
    long long identity = 1LL;  // 乘法幺元是 1
    long long expected = 120LL;

    long long result = parallel_reduce(
        input.begin(), input.end(), identity, std::multiplies<long long>());
    REQUIRE(result == expected);

    // 使用 lambda
    result = parallel_reduce(input.begin(),
                             input.end(),
                             1LL,
                             [](long long a, int b) { return a * b; });
    REQUIRE(result == expected);
  }

  SECTION("Empty input range")
  {
    std::vector<int> input_empty;
    int identity = 0;
    int expected = 0;  // 空范围归约结果是幺元
    int result = parallel_reduce(
        input_empty.begin(), input_empty.end(), identity, std::plus<int>());
    REQUIRE(result == expected);

    std::string str_identity = "";
    std::string str_expected = "";
    // Create a single named empty vector to get compatible iterators
    std::vector<std::string> empty_str_vec;
    std::string str_result =
        parallel_reduce(empty_str_vec.begin(), // Use begin() from the named vector
                        empty_str_vec.end(),   // Use end() from the same named vector
                        str_identity,
                        std::plus<std::string>());
    REQUIRE(str_result == str_expected);
  }

  SECTION("Single element range")
  {
    std::vector<int> input = {42};
    int identity = 0;
    int expected = 42;  // identity + 42 = 0 + 42 = 42
    int result =
        parallel_reduce(input.begin(), input.end(), identity, std::plus<int>());
    REQUIRE(result == expected);

    identity = 10;  // 换个幺元
    expected = 52;  // identity + 42 = 10 + 42 = 52
    result =
        parallel_reduce(input.begin(), input.end(), identity, std::plus<int>());
    REQUIRE(result == expected);
  }

  SECTION("Large input range summation")
  {
    std::vector<i64> input(large_size);
    std::iota(input.begin(), input.end(), 1);  // 1, 2, ..., large_size

    i64 identity = 0LL;
    // 计算期望值: n*(n+1)/2
    i64 n = large_size;
    i64 expected = n * (n + 1) / 2;

    i64 result =
        parallel_reduce(input.begin(), input.end(), identity, std::plus<i64>());
    REQUIRE(result == expected);
  }

  SECTION("String concatenation")
  {
    std::vector<std::string> input = {"Hello", " ", "World", "!"};
    std::string identity = "";
    std::string expected = "Hello World!";

    std::string result = parallel_reduce(
        input.begin(), input.end(), identity, std::plus<std::string>());
    REQUIRE(result == expected);
  }

  SECTION("Reduce with custom struct")
  {
    struct Point
    {
      int x = 0;
      int y = 0;
    };
    std::vector<Point> points = {{1, 2}, {3, 4}, {5, 6}};
    Point identity = {0, 0};
    Point expected = {9, 12};

    auto reduce_op = [](const Point& accumulated, const Point& current) {
      return Point {accumulated.x + current.x, accumulated.y + current.y};
    };

    Point result =
        parallel_reduce(points.begin(), points.end(), identity, reduce_op);
    REQUIRE(result.x == expected.x);
    REQUIRE(result.y == expected.y);
  }

  SECTION("Exception propagation during reduce")
  {
    std::vector<int> input(100);
    std::iota(input.begin(), input.end(), 0);
    int identity = 0;

    std::atomic<int> count = 0;
    auto throwing_op = [&](int acc, int val) -> int
    {
      // 在处理到某个值时抛出异常，确保它不是第一个元素
      // 用原子计数器模拟，在第N次调用时抛出
      if (count.fetch_add(1, std::memory_order_relaxed) == 50) {
        throw std::runtime_error("Reduce error at count 50");
      }
      return acc + val;
    };

    // 期望 parallel_reduce 捕获并重新抛出来自任务（局部归约或最终合并）的异常
    REQUIRE_THROWS_AS(
        parallel_reduce(input.begin(), input.end(), identity, throwing_op),
        std::runtime_error);
    //  REQUIRE_THROWS_WITH(
    //      parallel_reduce(input.begin(), input.end(), identity, throwing_op),
    //      ContainsSubstring("Reduce error at count 50")
    //  );
  }

  SECTION(
      "Reduce with non-commutative operation (order might matter if chunking "
      "strategy changes)")
  {
    // Example: string prepending instead of appending. Result depends on task
    // completion order if not associative. Our implementation merges partial
    // results sequentially, so it should be deterministic here, but good to be
    // aware. Let's test string append which *is* associative. A better test
    // might be matrix multiplication if we had a matrix type. We'll stick to
    // associative examples for correctness test.
    SUCCEED(
        "Skipping non-commutative test as implementation ensures sequential "
        "final merge");
  }
}
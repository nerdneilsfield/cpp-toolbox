// test/functional/functional_test.cpp
#include <array>
#include <atomic>
#include <chrono>  // For sleep simulation
#include <functional>
#include <future>
#include <iostream>
#include <map>  // For memoize test key
#include <map>  // For cache key comparison (if needed, though tuple handles it)
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>  // For concurrency testing
#include <unordered_map>
#include <variant>
#include <vector>

// Include the header for the functional tools under test
#include "cpp-toolbox/functional/functional.hpp"

// Include Catch2 testing framework
#include <catch2/catch_approx.hpp>  // For floating point comparison
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>  // For string matching
#include <catch2/matchers/catch_matchers_vector.hpp>  // For vector comparison

using namespace toolbox::functional;
using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::Equals;

/// @brief Adds two integers together
/// @param a First integer operand
/// @param b Second integer operand
/// @return Sum of a and b
static auto add(int a, int b) -> int
{
  return a + b;
}

/// @brief Multiplies two integers together
/// @param a First integer operand
/// @param b Second integer operand
/// @return Product of a and b
static auto multiply(int a, int b) -> int
{
  return a * b;
}

/// @brief Creates a greeting string for a given name
/// @param name The name to include in the greeting
/// @return A string containing "Hello, " followed by the name
static auto greet(const std::string& name) -> std::string
{
  return "Hello, " + name;
}

/// @brief Simple 2D point structure
struct Point
{
  int x, y;
};

/// @brief Equality comparison operator for Point
/// @param lhs Left-hand side Point operand
/// @param rhs Right-hand side Point operand
/// @return true if points have equal coordinates, false otherwise
static auto operator==(const Point& lhs, const Point& rhs) -> bool
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

/// @brief Global atomic counter for tracking function calls
static std::atomic<int> global_call_count = 0;

/// @brief Test function with side effects (increments counter)
/// @param x Integer input
/// @param y String input
/// @return Sum of x and y's length
static auto function_to_memoize(int x, const std::string& y) -> int
{
  global_call_count++;  // Increment on each actual execution
  return x + static_cast<int>(y.length());
}

/// @brief Another test function with different signature
/// @param a Double input value
/// @return Input multiplied by 1.5
static auto another_function(double a) -> double
{
  global_call_count++;
  return a * 1.5;
}

/// @brief Counter for tracking Fibonacci calculation calls
static std::atomic<int> fib_call_count = 0;

/// @brief Function object for memoized recursive Fibonacci implementation
/// Must be declared outside memoize call to allow lambda capture
static std::function<long long(int)> fib_rec;

/// @brief Sets up the memoized recursive Fibonacci calculator
static void setup_fib_rec()
{
  fib_call_count = 0;  // Reset counter
  fib_rec = toolbox::functional::memoize_explicit<long long, int>(
      [&](int n) -> long long
      {
        fib_call_count++;  // Track actual calculation count
        if (n <= 1)
          return n;
        return fib_rec(n - 1) + fib_rec(n - 2);
      });
}

/// @brief Test case for function composition functionality
TEST_CASE("Functional Compose Tests", "[functional][compose]")
{
  auto add5 = [](int x) { return x + 5; };
  auto mul2 = [](int x) { return x * 2; };
  auto to_str = [](int x) { return std::to_string(x); };

  SECTION("Compose two functions")
  {
    auto add5_then_mul2 = compose(mul2, add5);  // mul2(add5(x))
    REQUIRE(add5_then_mul2(10) == 30);  // mul2(10+5) = 30

    auto mul2_then_add5 = compose(add5, mul2);  // add5(mul2(x))
    REQUIRE(mul2_then_add5(10) == 25);  // add5(10*2) = 25
  }

  SECTION("Compose three functions")
  {
    auto mul2_add5_tostr =
        compose(to_str, add5, mul2);  // to_str(add5(mul2(x)))
    REQUIRE(mul2_add5_tostr(10) == "25");  // to_str(add5(10*2))

    auto add5_mul2_tostr =
        compose(to_str, mul2, add5);  // to_str(mul2(add5(x)))
    REQUIRE(add5_mul2_tostr(10) == "30");  // to_str(mul2(10+5))
  }

  SECTION("Compose with void function (not directly supported as intermediate)")
  {
    auto mul2_then_print = compose(
        [](int x)
        { std::cout << "Should not print in test: " << x << std::endl; },
        mul2);
    REQUIRE_NOTHROW(mul2_then_print(5));  // Just check it compiles and runs
  }

  SECTION("Compose single function")
  {
    auto just_add5 = compose(add5);
    REQUIRE(just_add5(10) == 15);
  }

  SECTION("Compose no functions")
  {
    REQUIRE_THROWS_AS(compose(), std::logic_error);
  }

  SECTION("Compose with stateful lambda")
  {
    int offset = 1;
    auto add_offset_then_mul2 = compose(mul2,
                                        [&offset](int x) mutable
                                        {
                                          offset++;
                                          return x + offset;
                                        });
    REQUIRE(add_offset_then_mul2(10)
            == 24);  // offset becomes 2, returns (10+2)*2 = 24
    REQUIRE(add_offset_then_mul2(10)
            == 26);  // offset becomes 3, returns (10+3)*2 = 26
    REQUIRE(offset == 3);
  }
}

/// @brief Test case for bind_first functionality
/// @details Tests binding the first argument of various functions
TEST_CASE("Functional Bind First Tests", "[functional][bind_first]")
{
  /// @brief Tests binding first argument of std::plus
  SECTION("Bind first argument of std::plus")
  {
    auto add10 = bind_first(std::plus<int>(), 10);
    REQUIRE(add10(5) == 15);
    REQUIRE(add10(-2) == 8);
  }

  /// @brief Tests binding first argument of custom functions
  SECTION("Bind first argument of custom function")
  {
    auto greet_alice = bind_first(greet, std::string("Alice"));
    REQUIRE(greet_alice() == "Hello, Alice");  // No more arguments needed

    auto multiply_by_5 = bind_first(multiply, 5);
    REQUIRE(multiply_by_5(6) == 30);
  }

  /// @brief Tests binding first argument with lambda functions
  SECTION("Bind first argument with lambda")
  {
    // Lambda arguments swapped: prefix first, then the string to check
    auto starts_with_hello =
        bind_first([](const std::string& prefix, const std::string& s)
                   { return s.rfind(prefix, 0) == 0; },
                   std::string("Hello"));

    REQUIRE(starts_with_hello("Hello World"));
    REQUIRE_FALSE(starts_with_hello("Goodbye World"));
  }
}

/// @brief Test case for optional extensions functionality
/// @details Tests map, flatMap, orElse, orElseGet and filter operations on
/// std::optional
TEST_CASE("Functional Optional Extensions", "[functional][optional]")
{
  std::optional<int> opt_num = 10;
  std::optional<int> opt_num_odd = 9;
  std::optional<int> opt_empty;
  std::optional<std::string> opt_str = "hello";

  auto int_to_str = [](int x) { return std::to_string(x); };
  auto str_len = [](const std::string& s) { return s.length(); };
  auto int_to_opt_str = [](int x)
  { return std::make_optional(std::to_string(x)); };
  auto int_to_opt_empty = [](int) -> std::optional<std::string>
  { return std::nullopt; };
  auto is_even = [](int x) { return x % 2 == 0; };
  std::atomic<int> default_func_calls = 0;
  auto get_default_val = [&]()
  {
    default_func_calls++;
    return -1;
  };

  /// @brief Tests map operation on optionals
  SECTION("map")
  {
    auto mapped1 = map(opt_num, int_to_str);
    REQUIRE(mapped1.has_value());
    REQUIRE(mapped1.value() == "10");

    auto mapped2 = map(opt_empty, int_to_str);
    REQUIRE_FALSE(mapped2.has_value());

    // Rvalue overload
    auto mapped3 = map(std::make_optional(std::string("world")), str_len);
    REQUIRE(mapped3.has_value());
    REQUIRE(mapped3.value() == 5);
  }

  /// @brief Tests flatMap operation on optionals
  SECTION("flatMap")
  {
    auto flatMapped1 =
        flatMap(opt_num, int_to_opt_str);  // 10 -> optional("10")
    REQUIRE(flatMapped1.has_value());
    REQUIRE(flatMapped1.value() == "10");

    auto flatMapped2 = flatMap(opt_num, int_to_opt_empty);  // 10 -> nullopt
    REQUIRE_FALSE(flatMapped2.has_value());

    auto flatMapped3 = flatMap(opt_empty, int_to_opt_str);  // empty -> empty
    REQUIRE_FALSE(flatMapped3.has_value());

    // Rvalue overload
    auto flatMapped4 = flatMap(std::make_optional(5), int_to_opt_str);
    REQUIRE(flatMapped4.has_value());
    REQUIRE(flatMapped4.value() == "5");
  }

  /// @brief Tests orElse operation on optionals
  SECTION("orElse")
  {
    REQUIRE(orElse(opt_num, -1) == 10);
    REQUIRE(orElse(opt_empty, -1) == -1);
    REQUIRE(orElse(opt_str, std::string("default")) == "hello");
    REQUIRE(orElse(std::optional<std::string> {}, std::string("default"))
            == "default");
    // Test type conversion
    REQUIRE(orElse(std::optional<double> {}, 10)
            == Catch::Approx(10.0));  // int -> double
  }

  /// @brief Tests orElseGet operation on optionals
  SECTION("orElseGet")
  {
    default_func_calls = 0;
    REQUIRE(orElseGet(opt_num, get_default_val) == 10);
    REQUIRE(default_func_calls == 0);  // Default func should not be called

    default_func_calls = 0;
    REQUIRE(orElseGet(opt_empty, get_default_val) == -1);
    REQUIRE(default_func_calls == 1);  // Default func should be called once

    // Test type conversion from func result
    REQUIRE(orElseGet(std::optional<double> {}, []() { return 5; })
            == Catch::Approx(5.0));
  }

  /// @brief Tests filter operation on optionals
  SECTION("filter")
  {
    REQUIRE(filter(opt_num, is_even).has_value());
    REQUIRE(filter(opt_num, is_even).value() == 10);

    REQUIRE_FALSE(filter(opt_num_odd, is_even).has_value());
    REQUIRE_FALSE(filter(opt_empty, is_even).has_value());

    // Rvalue overload
    auto filtered_rval = filter(std::make_optional(10), is_even);
    REQUIRE(filtered_rval.has_value());
    REQUIRE(filtered_rval.value() == 10);

    auto filtered_rval_fail = filter(std::make_optional(9), is_even);
    REQUIRE_FALSE(filtered_rval_fail.has_value());
  }
}

/// @brief Test case for variant matching functionality
/// @details Tests matching different types in a variant and executing
/// corresponding handlers
TEST_CASE("Functional Variant Match", "[functional][variant][match]")
{
  std::variant<int, std::string, double> var;

  /// @brief Test matching an int value
  SECTION("Match int")
  {
    var = 123;
    bool int_matched = false;
    match(
        var,
        [&](int i)
        {
          REQUIRE(i == 123);
          int_matched = true;
        },
        [](const std::string&) { FAIL("Incorrect type matched"); },
        [](double) { FAIL("Incorrect type matched"); });
    REQUIRE(int_matched);
  }

  /// @brief Test matching a string value
  SECTION("Match string")
  {
    var = "hello";
    bool string_matched = false;
    match(
        var,
        [](int) { FAIL("Incorrect type matched"); },
        [&](const std::string& s)
        {
          REQUIRE(s == "hello");
          string_matched = true;
        },
        [](double) { FAIL("Incorrect type matched"); });
    REQUIRE(string_matched);
  }

  /// @brief Test matching a double value
  SECTION("Match double")
  {
    var = 3.14;
    bool double_matched = false;
    match(
        var,
        [](int) { FAIL("Incorrect type matched"); },
        [](const std::string&) { FAIL("Incorrect type matched"); },
        [&](double d)
        {
          REQUIRE(d == Catch::Approx(3.14));
          double_matched = true;
        });
    REQUIRE(double_matched);
  }

  /// @brief Test matching with return value
  SECTION("Match with return value")
  {
    var = "world";
    std::string result = match(
        var,
        [](int i) { return std::string("Got int ") + std::to_string(i); },
        [](const std::string& s) { return std::string("Got string ") + s; },
        [](double d)
        { return std::string("Got double ") + std::to_string(d); });
    REQUIRE(result == "Got string world");
  }

  /// @brief Test matching with rvalue variant
  SECTION("Match with rvalue variant")
  {
    std::variant<int, std::string> rval_var = std::string("move me");
    std::string result = match(
        std::move(rval_var),
        [](int i) -> std::string { return "int"; },
        // Visitor for rvalue string can take by value or rvalue ref
        [](std::string s) -> std::string { return "moved string " + s; });
    REQUIRE(result == "moved string move me");
    // Check original variant state (likely holds moved-from string)
    // REQUIRE(std::get<std::string>(rval_var).empty()); // Behavior depends on
    // string move
  }
}

/// @brief Test case for simplified variant mapping functionality
/// @details Tests mapping variant values to different types using a visitor
TEST_CASE("Functional Variant Map (Simplified)", "[functional][variant][map]")
{
  std::variant<int, std::string, double> var;
  using ResultVariant = std::variant<size_t, double>;  // Expect string->size_t,
                                                       // int/double->double

  // Define a visitor lambda that handles all types and explicitly returns
  // ResultVariant
  auto visitor = [](const auto& x) -> ResultVariant
  {
    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
      return ResultVariant {static_cast<double>(x) * 1.5};  // int -> double
    } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>)
    {
      return ResultVariant {x.length()};  // string -> size_t
    } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, double>) {
      return ResultVariant {x + 1.0};  // double -> double
    }
    // Should be unreachable for the given variant types
    return ResultVariant {};  // Or throw an error
  };

  /// @brief Test mapping int to double
  SECTION("Map int to double")
  {
    var = 10;
    auto result = map<ResultVariant>(var, visitor);
    REQUIRE(std::holds_alternative<double>(result));
    REQUIRE(std::get<double>(result) == Catch::Approx(15.0));
  }

  /// @brief Test mapping string to size_t
  SECTION("Map string to size_t")
  {
    var = "hello";
    auto result = map<ResultVariant>(var, visitor);
    REQUIRE(std::holds_alternative<size_t>(result));
    REQUIRE(std::get<size_t>(result) == 5);
  }

  /// @brief Test mapping double to double
  SECTION("Map double to double")
  {
    var = 3.14;
    auto result = map<ResultVariant>(var, visitor);
    REQUIRE(std::holds_alternative<double>(result));
    REQUIRE(std::get<double>(result) == Catch::Approx(4.14));
  }

  /// @brief Test mapping non-const variant with modification
  SECTION("Map non-const variant")
  {
    std::variant<int, std::string> var_nc = 5;
    auto result = map<std::variant<int>>(
        var_nc,
        [](auto& x) -> std::variant<int>
        {
          if constexpr (std::is_same_v<decltype(x), int&>) {
            x *= 2;
            return {x};
          } else
            return {};  // Return empty variant if string
        });
    REQUIRE(std::holds_alternative<int>(result));  // Check result holds int
    REQUIRE(std::get<int>(result) == 10);
    REQUIRE(std::get<int>(var_nc) == 10);  // Original modified
  }

  /// @brief Test mapping rvalue variant
  SECTION("Map rvalue variant")
  {
    std::variant<int, std::string> var_rv = std::string("move");
    auto result = map<std::variant<size_t>>(
        std::move(var_rv),
        [](auto&& x) -> std::variant<size_t>
        {
          if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>)
          {
            return {x.length()};
          } else
            return {};  // Return empty variant if int
        });
    REQUIRE(
        std::holds_alternative<size_t>(result));  // Check result holds size_t
    REQUIRE(std::get<size_t>(result) == 4);
  }
}

/// @brief Test case for container operations
/// @details Tests map, filter and reduce operations on containers
TEST_CASE("Functional Container Operations", "[functional][container]")
{
  std::vector<int> nums = {1, 2, 3, 4, 5, 6};
  std::vector<int> empty_vec;
  std::vector<std::string> strs = {"a", "bb", "ccc"};

  /// @brief Test mapping container elements
  SECTION("map container")
  {
    auto squares = map(nums, [](int x) { return x * x; });
    REQUIRE_THAT(squares, Equals(std::vector<int> {1, 4, 9, 16, 25, 36}));
    REQUIRE(map(empty_vec, [](int x) { return x * x; }).empty());

    auto lengths = map(strs, [](const std::string& s) { return s.length(); });
    REQUIRE_THAT(lengths, Equals(std::vector<size_t> {1, 2, 3}));

    // Check return type is vector even if input is array
    std::array<int, 3> arr = {1, 2, 3};
    auto arr_mapped = map(arr, [](int x) { return x + 1; });
    REQUIRE(std::is_same_v<decltype(arr_mapped), std::vector<int>>);
    REQUIRE_THAT(arr_mapped, Equals(std::vector<int> {2, 3, 4}));
  }

  /// @brief Test filtering container elements
  SECTION("filter container")
  {
    auto evens = filter(nums, [](int x) { return x % 2 == 0; });
    REQUIRE_THAT(evens, Equals(std::vector<int> {2, 4, 6}));

    auto odds = filter(nums, [](int x) { return x % 2 != 0; });
    REQUIRE_THAT(odds, Equals(std::vector<int> {1, 3, 5}));

    auto long_strs =
        filter(strs, [](const std::string& s) { return s.length() > 1; });
    REQUIRE_THAT(long_strs, Equals(std::vector<std::string> {"bb", "ccc"}));

    REQUIRE(filter(empty_vec, [](int x) { return true; }).empty());
    REQUIRE(filter(nums, [](int x) { return false; }).empty());
  }

  /// @brief Test reducing container elements
  SECTION("reduce container")
  {
    REQUIRE(reduce(nums, 0, std::plus<int>()) == 21);  // 1+2+3+4+5+6
    REQUIRE(reduce(nums, 1, std::multiplies<int>()) == 720);  // 1*2*3*4*5*6

    REQUIRE(reduce(strs, std::string(""), std::plus<std::string>())
            == "abbccc");

    // Reduce without identity
    REQUIRE(reduce(nums, std::plus<int>()) == 21);
    REQUIRE(reduce(strs, std::plus<std::string>()) == "abbccc");
    REQUIRE(reduce(std::vector<int> {5}, std::plus<int>()) == 5);
    REQUIRE_THROWS_AS(reduce(empty_vec, std::plus<int>()),
                      std::invalid_argument);

    // Check identity with non-zero/one value
    REQUIRE(reduce(nums, 100, std::plus<int>()) == 121);
  }
}

TEST_CASE("Functional Zip Vector", "[functional][zip][vector]")
{
  std::vector<int> nums = {1, 2, 3, 4};
  std::vector<char> chars = {'a', 'b', 'c'};  // Shorter
  std::vector<double> dbls = {1.1, 2.2, 3.3, 4.4, 5.5};  // Longer
  std::vector<int> empty_vec;

  /// @brief Test zipping two vectors of different lengths
  /// @details The resulting vector should have the length of the shorter input
  /// vector
  SECTION("Zip two vectors (different lengths)")
  {
    auto zipped = zip(nums, chars);
    // Expected type: std::vector<std::tuple<const int&, const char&>>
    REQUIRE(zipped.size() == 3);  // Length of shorter vector (chars)
    REQUIRE(std::get<0>(zipped[0]) == 1);
    REQUIRE(std::get<1>(zipped[0]) == 'a');
    REQUIRE(std::get<0>(zipped[1]) == 2);
    REQUIRE(std::get<1>(zipped[1]) == 'b');
    REQUIRE(std::get<0>(zipped[2]) == 3);
    REQUIRE(std::get<1>(zipped[2]) == 'c');
  }

  /// @brief Test zipping three vectors of different lengths
  /// @details The resulting vector should have the length of the shortest input
  /// vector
  SECTION("Zip three vectors (different lengths)")
  {
    auto zipped = zip(nums, chars, dbls);
    // Expected type: std::vector<std::tuple<const int&, const char&, const
    // double&>>
    REQUIRE(zipped.size() == 3);  // Length of shortest vector (chars)
    REQUIRE(std::get<0>(zipped[0]) == 1);
    REQUIRE(std::get<1>(zipped[0]) == 'a');
    REQUIRE(std::get<2>(zipped[0]) == Catch::Approx(1.1));
    REQUIRE(std::get<0>(zipped[2]) == 3);
    REQUIRE(std::get<1>(zipped[2]) == 'c');
    REQUIRE(std::get<2>(zipped[2]) == Catch::Approx(3.3));
  }

  /// @brief Test zipping with empty vectors
  /// @details The result should always be empty if any input vector is empty
  SECTION("Zip with empty vector")
  {
    auto zipped1 = zip(nums, empty_vec);
    REQUIRE(zipped1.empty());

    auto zipped2 = zip(empty_vec, nums);
    REQUIRE(zipped2.empty());

    auto zipped3 = zip(empty_vec, empty_vec);
    REQUIRE(zipped3.empty());
  }

  /// @brief Test zipping with no input vectors
  /// @details Should return an empty vector
  SECTION("Zip zero vectors")
  {
    auto zipped = zip();  // Should return empty vector
    REQUIRE(zipped.empty());
  }

  /// @brief Test that zipped elements are references to original data
  /// @details Changes to original vectors should be reflected in zipped results
  SECTION("Check references")
  {
    auto zipped = zip(nums, chars);
    REQUIRE(zipped.size() == 3);
    // Modify original vector element
    nums[0] = 100;
    // Check if reference in tuple reflects the change
    REQUIRE(std::get<0>(zipped[0]) == 100);
  }
}

TEST_CASE("Functional Zip To Unordered Map", "[functional][zip][map]")
{
  std::vector<int> keys1 = {1, 2, 3};
  std::vector<std::string> values1 = {"one", "two", "three"};

  std::vector<std::string> keys2 = {"a", "b", "a"};  // Duplicate key
  std::vector<int> values2 = {10, 20, 30};

  std::vector<int> keys3 = {1, 2};
  std::vector<std::string> values3 = {"x", "y", "z"};  // Longer values

  std::vector<int> empty_vec;

  /// @brief Test basic map creation from two vectors
  /// @details Should create a map with key-value pairs from the input vectors
  SECTION("Basic zip to map")
  {
    auto map = zip_to_unordered_map(keys1, values1);
    REQUIRE(map.size() == 3);
    REQUIRE(map.at(1) == "one");
    REQUIRE(map.at(2) == "two");
    REQUIRE(map.at(3) == "three");
    REQUIRE(map.count(4) == 0);
  }

  /// @brief Test handling of duplicate keys
  /// @details First occurrence of a key should be kept, later duplicates
  /// ignored
  SECTION("Duplicate keys")
  {
    auto map = zip_to_unordered_map(keys2, values2);
    REQUIRE(map.size() == 2);  // Only 'a' and 'b' should be present
    REQUIRE(map.at("a") == 10);  // First occurrence of 'a' wins due to emplace
    REQUIRE(map.at("b") == 20);
    REQUIRE(map.count("c") == 0);
  }

  /// @brief Test vectors of different lengths
  /// @details Should use the length of the shorter vector
  SECTION("Different lengths (uses shortest)")
  {
    auto map = zip_to_unordered_map(keys3, values3);
    REQUIRE(map.size() == 2);  // Length of keys3
    REQUIRE(map.at(1) == "x");
    REQUIRE(map.at(2) == "y");
    REQUIRE(map.count(3) == 0);

    auto map2 = zip_to_unordered_map(values3, keys3);  // keys shorter now
    REQUIRE(map2.size() == 2);
    REQUIRE(map2.at("x") == 1);
    REQUIRE(map2.at("y") == 2);
    REQUIRE(map2.count("z") == 0);
  }

  /// @brief Test with empty input vectors
  /// @details Should return an empty map
  SECTION("Empty input")
  {
    auto map1 = zip_to_unordered_map(empty_vec, values1);
    REQUIRE(map1.empty());

    auto map2 = zip_to_unordered_map(keys1, empty_vec);
    REQUIRE(map2.empty());

    auto map3 = zip_to_unordered_map(empty_vec, empty_vec);
    REQUIRE(map3.empty());
  }

  /// @brief Test explicit value type conversion
  /// @details Should convert values to specified type if possible
  SECTION("Explicit value type conversion")
  {
    //          auto map = zip_to_unordered_map<std::vector<int>,
    //          std::vector<std::string>, int, double>(keys1, values1);
    //          REQUIRE(map.size() == 3);
    //          REQUIRE(map_invalid_conversion.size() == 3); // This would be 3
    //          if conversion worked
    // Let's change the example to something convertible
    std::vector<int> int_values = {10, 20, 30};
    auto map_double =
        zip_to_unordered_map<std::vector<int>, std::vector<int>, int, double>(
            keys1, int_values);
    REQUIRE(map_double.size() == 3);
    REQUIRE(map_double.at(1) == Catch::Approx(10.0));
    REQUIRE(map_double.at(2) == Catch::Approx(20.0));
    REQUIRE(map_double.at(3) == Catch::Approx(30.0));
  }

  /// @brief Test with custom key type requiring hash and equality functions
  /// @details Should work with user-defined types that provide necessary
  /// operations
  SECTION("Custom key type (requires hash/equal_to)")
  {
    struct MyKey
    {
      int id;
      std::string name;
    };
    // Need hash and equal_to for MyKey
    struct MyKeyHash
    {
      std::size_t operator()(const MyKey& k) const
      {
        // Simple hash combination
        size_t h1 = std::hash<int> {}(k.id);
        size_t h2 = std::hash<std::string> {}(k.name);
        return h1 ^ (h2 << 1);
      }
    };
    struct MyKeyEqual
    {
      bool operator()(const MyKey& lhs, const MyKey& rhs) const
      {
        return lhs.id == rhs.id && lhs.name == rhs.name;
      }
    };

    std::vector<MyKey> keys = {{1, "a"}, {2, "b"}};
    std::vector<int> values = {100, 200};

    auto map = zip_to_unordered_map<std::vector<MyKey>,
                                    std::vector<int>,
                                    MyKey,
                                    int,
                                    MyKeyHash,
                                    MyKeyEqual>(keys, values);
    REQUIRE(map.size() == 2);
    REQUIRE(map.at({1, "a"}) == 100);
    REQUIRE(map.at({2, "b"}) == 200);
  }
}

/// @brief Test case for memoization functionality
/// @details Tests memoization of functions with different signatures
TEST_CASE("Functional Memoize Tests", "[functional][memoize]")
{
  /// @brief Test basic function memoization
  /// @details Verifies that results are cached and function is only called once
  /// per unique input
  SECTION("Basic Caching")
  {
    global_call_count = 0;  // Reset counter
    auto memoized_func =
        memoize_explicit<int, int, const std::string&>(function_to_memoize);

    // First call with (1, "hello")
    int result1 = memoized_func(1, "hello");
    REQUIRE(result1 == 6);  // 1 + 5
    REQUIRE(global_call_count == 1);

    // Second call with (1, "hello") - should use cache
    int result2 = memoized_func(1, "hello");
    REQUIRE(result2 == 6);
    REQUIRE(global_call_count == 1);  // Counter shouldn't increase

    // First call with (2, "world")
    int result3 = memoized_func(2, "world");
    REQUIRE(result3 == 7);  // 2 + 5
    REQUIRE(global_call_count == 2);  // Counter should increase

    // Second call with (2, "world") - should use cache
    int result4 = memoized_func(2, "world");
    REQUIRE(result4 == 7);
    REQUIRE(global_call_count == 2);  // Counter shouldn't increase

    // Another call with (1, "hello") - should still use cache
    int result5 = memoized_func(1, "hello");
    REQUIRE(result5 == 6);
    REQUIRE(global_call_count == 2);  // Counter shouldn't increase
  }

  /// @brief Test memoization with different function signatures
  /// @details Verifies memoization works with different parameter and return
  /// types
  SECTION("Different Function Signature")
  {
    global_call_count = 0;
    auto memoized_double = memoize_explicit<double, double>(another_function);

    REQUIRE(memoized_double(10.0) == Catch::Approx(15.0));
    REQUIRE(global_call_count == 1);
    REQUIRE(memoized_double(10.0) == Catch::Approx(15.0));
    REQUIRE(global_call_count == 1);  // Cached

    REQUIRE(memoized_double(20.0) == Catch::Approx(30.0));
    REQUIRE(global_call_count == 2);
    REQUIRE(memoized_double(20.0) == Catch::Approx(30.0));
    REQUIRE(global_call_count == 2);  // Cached
  }

  /// @brief Test thread safety with same arguments
  /// @details Verifies memoization works correctly when multiple threads call
  /// with identical arguments
  SECTION("Thread Safety - Same Arguments")
  {
    global_call_count = 0;
    auto memoized_func =
        memoize_explicit<int, int, const std::string&>(function_to_memoize);
    const int num_threads = 8;
    const int calls_per_thread = 5;
    std::vector<std::thread> threads;
    std::vector<std::future<int>> results;

    // All threads call with same arguments (5, "test")
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back(
          [&memoized_func]()
          {
            int res = 0;
            for (int j = 0; j < calls_per_thread; ++j) {
              res = memoized_func(5, "test");  // Expected: 5 + 4 = 9
              // Small delay to increase chance of concurrent access
              std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
          });
    }

    for (auto& t : threads) {
      t.join();
    }

    // Verify: original function should be called only once
    REQUIRE(global_call_count == 1);

    // Call again to verify correct result
    REQUIRE(memoized_func(5, "test") == 9);
    REQUIRE(global_call_count == 1);  // Still 1
  }

  /// @brief Test thread safety with different arguments
  /// @details Verifies memoization works correctly when multiple threads call
  /// with different arguments
  SECTION("Thread Safety - Different Arguments")
  {
    global_call_count = 0;
    auto memoized_func =
        memoize_explicit<int, int, const std::string&>(function_to_memoize);
    const int num_threads = 8;
    std::vector<std::thread> threads;
    std::vector<std::future<int>> futures;
    futures.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
      // Each thread uses different arguments
      futures.emplace_back(std::async(
          std::launch::async,
          [&memoized_func, i]()
          {
            // Multiple calls to test caching
            int res = 0;
            for (int k = 0; k < 3; ++k) {
              res = memoized_func(
                  i, std::to_string(i));  // Args: (0,"0"), (1,"1"), ...
            }
            return res;
          }));
    }

    // Verify results
    for (int i = 0; i < num_threads; ++i) {
      int expected_result = i + static_cast<int>(std::to_string(i).length());
      REQUIRE(futures[i].valid());
      REQUIRE(futures[i].get() == expected_result);
    }

    // Verify: original function should be called once per unique argument
    // combination
    REQUIRE(global_call_count == num_threads);

    // Call again with some arguments, should hit cache
    REQUIRE(memoized_func(1, "1") == (1 + 1));
    REQUIRE(global_call_count == num_threads);  // Count remains the same
  }

  /// @brief Test recursive function memoization using Fibonacci
  /// @details Verifies memoization significantly reduces calls in recursive
  /// functions
  SECTION("Recursive Memoization (Fibonacci)")
  {
    setup_fib_rec();  // Initialize memoized recursive function fib_rec and
                      // counter

    // Calculate fib(10)
    long long result1 = fib_rec(10);
    REQUIRE(result1 == 55);
    int calls1 = fib_call_count.load();
    // For fib(10), actual calculations should be far fewer than 2^10
    // Should be exactly 11 different fib(n) calls (0 to 10)
    REQUIRE(calls1 <= 11);  // <= because implementation may vary slightly
    std::cout << "Fib(10) took " << calls1 << " actual calls." << std::endl;

    // Calculate fib(10) again, should hit cache completely
    fib_call_count = 0;  // Reset counter to observe second call
    long long result2 = fib_rec(10);
    REQUIRE(result2 == 55);
    int calls2 = fib_call_count.load();
    REQUIRE(calls2 == 0);  // Second call, counter should be 0

    // Calculate fib(12), should use cached results from fib(0) to fib(10)
    fib_call_count = 0;  // Reset counter to observe incremental calls
    long long result3 = fib_rec(12);
    REQUIRE(result3 == 144);
    int calls3 = fib_call_count.load();
    // Only needs to calculate fib(11) and fib(12)
    REQUIRE(calls3 <= 2);  // Theoretically 2 new calculations
    std::cout << "Fib(12) after Fib(10) took " << calls3 << " additional calls."
              << std::endl;
  }
}

template<typename T, size_t N, typename F>
auto map(const std::array<T, N>& container, F&& func)
{
  std::array<decltype(func(std::declval<T>())), N> result;
  std::transform(container.begin(),
                 container.end(),
                 result.begin(),
                 std::forward<F>(func));
  return result;
}
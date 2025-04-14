#include <array>
#include <limits>
#include <numeric>  // For std::iota in large vector test
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/types/minmax.hpp>
#include <cpp-toolbox/types/point.hpp>

// Assuming minmax_t is in toolbox::types
using toolbox::types::minmax_t;
using toolbox::types::point_cloud_t;
using toolbox::types::point_t;  // Using declaration for convenience

// --- Tests for minmax_t ---

TEST_CASE("MinMax default construction", "[minmax]")
{
  SECTION("Integer")
  {
    minmax_t<int> mm_int;
    CHECK(mm_int.min == std::numeric_limits<int>::max());
    CHECK(mm_int.max == std::numeric_limits<int>::min());
  }
  SECTION("Float")
  {
    minmax_t<float> mm_float;
    CHECK(mm_float.min == Catch::Approx(std::numeric_limits<float>::max()));
    CHECK(mm_float.max == Catch::Approx(std::numeric_limits<float>::lowest()));
  }
  SECTION("Double")
  {
    minmax_t<double> mm_double;
    CHECK(mm_double.min == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm_double.max
          == Catch::Approx(std::numeric_limits<double>::lowest()));
  }
  SECTION("Unsigned Int")
  {
    minmax_t<unsigned int> mm_uint;
    CHECK(mm_uint.min == std::numeric_limits<unsigned int>::max());
    CHECK(mm_uint.max == std::numeric_limits<unsigned int>::min());
  }
}

TEST_CASE("MinMax construction with values", "[minmax]")
{
  minmax_t<int> mm_int(10, 20);
  CHECK(mm_int.min == 10);
  CHECK(mm_int.max == 20);

  minmax_t<double> mm_double(-5.5, 5.5);
  CHECK(mm_double.min == Catch::Approx(-5.5));
  CHECK(mm_double.max == Catch::Approx(5.5));
}

TEST_CASE("MinMax copy and move semantics", "[minmax]")
{
  minmax_t<int> mm1(5, 15);

  // Copy constructor
  minmax_t<int> mm2 = mm1;
  CHECK(mm2.min == 5);
  CHECK(mm2.max == 15);

  // Copy assignment
  minmax_t<int> mm3;
  mm3 = mm1;
  CHECK(mm3.min == 5);
  CHECK(mm3.max == 15);

  // Move constructor
  minmax_t<int> mm1_to_move = mm1;
  minmax_t<int> mm4 = std::move(mm1_to_move);
  CHECK(mm4.min == 5);
  CHECK(mm4.max == 15);
  // Check moved-from state (optional, behavior might vary)

  // Move assignment
  minmax_t<int> mm5;
  minmax_t<int> mm4_to_move = mm4;
  mm5 = std::move(mm4_to_move);
  CHECK(mm5.min == 5);
  CHECK(mm5.max == 15);
  // Check moved-from state (optional)

  // Self assignment (should not change)
  mm5 = mm5;
  CHECK(mm5.min == 5);
  CHECK(mm5.max == 15);
  mm5 = std::move(mm5);
  CHECK(mm5.min == 5);
  CHECK(mm5.max == 15);
}

TEST_CASE("MinMax operator plus equals", "[minmax]")
{
  SECTION("Integer")
  {
    minmax_t<int> mm;
    mm += 5;
    CHECK(mm.min == 5);
    CHECK(mm.max == 5);
    mm += -2;
    CHECK(mm.min == -2);
    CHECK(mm.max == 5);
    mm += 10;
    CHECK(mm.min == -2);
    CHECK(mm.max == 10);
    mm += 0;
    CHECK(mm.min == -2);
    CHECK(mm.max == 10);
  }
  SECTION("Double")
  {
    minmax_t<double> mm;
    mm += 3.14;
    CHECK(mm.min == Catch::Approx(3.14));
    CHECK(mm.max == Catch::Approx(3.14));
    mm += -1.0;
    CHECK(mm.min == Catch::Approx(-1.0));
    CHECK(mm.max == Catch::Approx(3.14));
    mm += 100.5;
    CHECK(mm.min == Catch::Approx(-1.0));
    CHECK(mm.max == Catch::Approx(100.5));
    mm += 0.0;
    CHECK(mm.min == Catch::Approx(-1.0));
    CHECK(mm.max == Catch::Approx(100.5));
  }
}

TEST_CASE("MinMax point specialization", "[minmax][point]")
{
  SECTION("Default Construction")
  {
    minmax_t<point_t<double>> mm_point;
    // Check against the expected initial values based on numeric_limits
    CHECK(!mm_point.initialized_);  // Should start uninitialized
    CHECK(mm_point.min.x == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm_point.min.y == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm_point.min.z == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm_point.max.x
          == Catch::Approx(std::numeric_limits<double>::lowest()));
    CHECK(mm_point.max.y
          == Catch::Approx(std::numeric_limits<double>::lowest()));
    CHECK(mm_point.max.z
          == Catch::Approx(std::numeric_limits<double>::lowest()));

    minmax_t<point_t<int>> mm_point_int;
    CHECK(!mm_point_int.initialized_);
    CHECK(mm_point_int.min.x == std::numeric_limits<int>::max());
    CHECK(mm_point_int.min.y == std::numeric_limits<int>::max());
    CHECK(mm_point_int.min.z == std::numeric_limits<int>::max());
    CHECK(mm_point_int.max.x == std::numeric_limits<int>::min());
    CHECK(mm_point_int.max.y == std::numeric_limits<int>::min());
    CHECK(mm_point_int.max.z == std::numeric_limits<int>::min());
  }
  SECTION("Operator Plus Equals")
  {
    minmax_t<point_t<double>> mm_point;
    point_t<double> p1(1.0, 5.0, -2.0);
    mm_point += p1;
    // First point becomes both min and max
    CHECK(mm_point.min.x == Catch::Approx(1.0));
    CHECK(mm_point.min.y == Catch::Approx(5.0));
    CHECK(mm_point.min.z == Catch::Approx(-2.0));
    CHECK(mm_point.max.x == Catch::Approx(1.0));
    CHECK(mm_point.max.y == Catch::Approx(5.0));
    CHECK(mm_point.max.z == Catch::Approx(-2.0));

    point_t<double> p2(-3.0, 6.0, 4.0);
    mm_point += p2;
    // Check component-wise min/max update
    CHECK(mm_point.min.x == Catch::Approx(-3.0));  // min(1.0, -3.0)
    CHECK(mm_point.min.y == Catch::Approx(5.0));  // min(5.0, 6.0)
    CHECK(mm_point.min.z == Catch::Approx(-2.0));  // min(-2.0, 4.0)
    CHECK(mm_point.max.x == Catch::Approx(1.0));  // max(1.0, -3.0)
    CHECK(mm_point.max.y == Catch::Approx(6.0));  // max(5.0, 6.0)
    CHECK(mm_point.max.z == Catch::Approx(4.0));  // max(-2.0, 4.0)

    point_t<double> p3(0.0, 0.0, 0.0);
    mm_point += p3;
    // Check update with zero point
    CHECK(mm_point.min.x == Catch::Approx(-3.0));  // min(-3.0, 0.0)
    CHECK(mm_point.min.y == Catch::Approx(0.0));  // min(5.0, 0.0)
    CHECK(mm_point.min.z == Catch::Approx(-2.0));  // min(-2.0, 0.0)
    CHECK(mm_point.max.x == Catch::Approx(1.0));  // max(1.0, 0.0)
    CHECK(mm_point.max.y == Catch::Approx(6.0));  // max(6.0, 0.0)
    CHECK(mm_point.max.z == Catch::Approx(4.0));  // max(4.0, 0.0)
  }
}

// --- Existing tests for point_t and point_cloud_t ---

TEST_CASE("Point constructors", "[point]")
{
  const point_t<double> p1_default;  // Default constructor
  CHECK(p1_default.x == Catch::Approx(0.0));
  CHECK(p1_default.y == Catch::Approx(0.0));
  CHECK(p1_default.z == Catch::Approx(0.0));

  const point_t<float> p2_value(1.0F, 2.0F, 3.0F);  // Value constructor
  CHECK(p2_value.x == Catch::Approx(1.0F));
  CHECK(p2_value.y == Catch::Approx(2.0F));
  CHECK(p2_value.z == Catch::Approx(3.0F));

  const point_t<float> p3_copy = p2_value;  // Copy constructor
  CHECK(p3_copy.x == Catch::Approx(1.0F));
  CHECK(p3_copy.y == Catch::Approx(2.0F));
  CHECK(p3_copy.z == Catch::Approx(3.0F));

  point_t<float> p3_copy_moved = p3_copy;
  point_t<float> p4_move = std::move(p3_copy_moved);  // Move constructor
  CHECK(p4_move.x == Catch::Approx(1.0F));
  CHECK(p4_move.y == Catch::Approx(2.0F));
  CHECK(p4_move.z == Catch::Approx(3.0F));
}

TEST_CASE("Point assignment", "[point]")
{
  const point_t<int> p1_source(1, 2, 3);
  point_t<int> p2_copy_target;
  p2_copy_target = p1_source;  // Copy assignment
  CHECK(p2_copy_target.x == 1);
  CHECK(p2_copy_target.y == 2);
  CHECK(p2_copy_target.z == 3);

  point_t<int> p3_move_target;
  point_t<int> p2_copy_target_moved = p2_copy_target;
  p3_move_target = std::move(p2_copy_target_moved);  // Move assignment
  CHECK(p3_move_target.x == 1);
  CHECK(p3_move_target.y == 2);
  CHECK(p3_move_target.z == 3);

  // Self-assignment check
  p3_move_target = p3_move_target;  // Self copy assignment
  CHECK(p3_move_target.x == 1);
  CHECK(p3_move_target.y == 2);
  CHECK(p3_move_target.z == 3);
  p3_move_target = std::move(p3_move_target);  // Self move assignment
  CHECK(p3_move_target.x == 1);
  CHECK(p3_move_target.y == 2);
  CHECK(p3_move_target.z == 3);
}

TEST_CASE("Point arithmetic operations", "[point]")
{
  point_t<double> p1_arith(1.0, 2.0, 3.0);
  const point_t<double> p2_operand(0.5, -0.5, 1.0);

  p1_arith += p2_operand;
  CHECK(p1_arith.x == Catch::Approx(1.5));
  CHECK(p1_arith.y == Catch::Approx(1.5));
  CHECK(p1_arith.z == Catch::Approx(4.0));

  p1_arith -= p2_operand;
  CHECK(p1_arith.x == Catch::Approx(1.0));
  CHECK(p1_arith.y == Catch::Approx(2.0));
  CHECK(p1_arith.z == Catch::Approx(3.0));

  p1_arith *= 2.0;
  CHECK(p1_arith.x == Catch::Approx(2.0));
  CHECK(p1_arith.y == Catch::Approx(4.0));
  CHECK(p1_arith.z == Catch::Approx(6.0));

  p1_arith /= 2.0;
  CHECK(p1_arith.x == Catch::Approx(1.0));
  CHECK(p1_arith.y == Catch::Approx(2.0));
  CHECK(p1_arith.z == Catch::Approx(3.0));

  CHECK_THROWS_AS(p1_arith /= 0.0, std::runtime_error);
}

TEST_CASE("Point vector operations", "[point]")
{
  const point_t<double> p1_vec(1.0, 2.0, 3.0);
  const point_t<double> p2_vec(4.0, 5.0, 6.0);

  double d = p1_vec.dot(p2_vec);
  CHECK(d == Catch::Approx(32.0));  // 4 + 10 + 18 = 32

  point_t<double> p3 = p1_vec.cross(p2_vec);
  CHECK(p3.x == Catch::Approx(-3.0));
  CHECK(p3.y == Catch::Approx(6.0));
  CHECK(p3.z == Catch::Approx(-3.0));
}

TEST_CASE("Point norm and distance", "[point]")
{
  const point_t<double> p1_norm(3.0, 0.0, 4.0);
  const point_t<double> p2_zero(0.0, 0.0, 0.0);

  CHECK(p1_norm.norm() == Catch::Approx(5.0));
  CHECK(p2_zero.norm() == Catch::Approx(0.0));

  point_t<int> p3(3, 0, 4);
  CHECK(p3.norm() == Catch::Approx(5.0));

  CHECK(p1_norm.distance(p2_zero) == Catch::Approx(5.0));
}

TEST_CASE("Point normalization", "[point]")
{
  const point_t<double> p1_to_norm(3.0, 0.0, 4.0);
  const point_t<double> p_normalized = p1_to_norm.normalize();
  CHECK(p_normalized.x == Catch::Approx(3.0 / 5.0));
  CHECK(p_normalized.y == Catch::Approx(0.0));
  CHECK(p_normalized.z == Catch::Approx(4.0 / 5.0));
  CHECK(p_normalized.norm() == Catch::Approx(1.0));

  const point_t<double> p_zero(0.0, 0.0, 0.0);
  const point_t<double> p_zero_norm = p_zero.normalize();
  CHECK(p_zero_norm.x == Catch::Approx(0.0));
  CHECK(p_zero_norm.y == Catch::Approx(0.0));
  CHECK(p_zero_norm.z == Catch::Approx(0.0));
  CHECK(p_zero_norm.norm() == Catch::Approx(0.0));

  const point_t<int> p_int(3, 0, 4);
  const point_t<double> p_int_normalized = p_int.normalize();
  CHECK(p_int_normalized.x == Catch::Approx(3.0 / 5.0));
  CHECK(p_int_normalized.y == Catch::Approx(0.0));
  CHECK(p_int_normalized.z == Catch::Approx(4.0 / 5.0));
}

TEST_CASE("Point static min/max", "[point]")
{
  const point_t<int> min_int_p = point_t<int>::min_value();
  const point_t<int> max_int_p = point_t<int>::max_value();
  CHECK(min_int_p.x == std::numeric_limits<int>::min());
  CHECK(max_int_p.x == std::numeric_limits<int>::max());

  const point_t<float> min_float_p = point_t<float>::min_value();
  const point_t<float> max_float_p = point_t<float>::max_value();
  CHECK(min_float_p.x == Catch::Approx(std::numeric_limits<float>::lowest()));
  CHECK(max_float_p.x == Catch::Approx(std::numeric_limits<float>::max()));
}

TEST_CASE("Point stream output", "[point]")
{
  const point_t<int> p_stream(1, -2, 30);
  std::stringstream ss;
  ss << p_stream;
  CHECK(ss.str() == "(1, -2, 30)");
}

TEST_CASE("Point cloud basic operations", "[point_cloud]")
{
  point_cloud_t<float> pc_basic;
  CHECK(pc_basic.empty());
  CHECK(pc_basic.size() == 0);

  pc_basic.reserve(10);

  const point_t<float> p1_add(1.0F, 1.0F, 1.0F);
  pc_basic += p1_add;
  CHECK_FALSE(pc_basic.empty());
  CHECK(pc_basic.size() == 1);
  REQUIRE_FALSE(pc_basic.points.empty());
  CHECK(pc_basic.points[0].x == Catch::Approx(1.0F));

  point_t<float> p2_move(2.0F, 2.0F, 2.0F);
  pc_basic += std::move(p2_move);
  CHECK(pc_basic.size() == 2);
  REQUIRE(pc_basic.points.size() >= 2);
  CHECK(pc_basic.points[1].x == Catch::Approx(2.0F));

  pc_basic.clear();
  CHECK(pc_basic.empty());
  CHECK(pc_basic.size() == 0);
  CHECK(pc_basic.normals.empty());
  CHECK(pc_basic.colors.empty());
  CHECK(pc_basic.intensity == Catch::Approx(0.0F));
}

TEST_CASE("Point cloud addition", "[point_cloud]")
{
  point_cloud_t<double> pc1_target;
  pc1_target += point_t<double>(1.0, 1.0, 1.0);
  pc1_target.intensity = 10.0;

  point_cloud_t<double> pc2_source;
  pc2_source += point_t<double>(2.0, 2.0, 2.0);
  pc2_source += point_t<double>(3.0, 3.0, 3.0);
  pc2_source.intensity = 5.0;

  pc1_target += pc2_source;
  CHECK(pc1_target.size() == 3);
  REQUIRE(pc1_target.points.size() >= 3);
  CHECK(pc1_target.points[0].x == Catch::Approx(1.0));
  CHECK(pc1_target.points[1].x == Catch::Approx(2.0));
  CHECK(pc1_target.points[2].x == Catch::Approx(3.0));
  CHECK(pc1_target.intensity == Catch::Approx(15.0));

  point_cloud_t<double> pc3_self;
  pc3_self += point_t<double>(1.0, 1.0, 1.0);
  pc3_self.intensity = 1.0;
  const point_cloud_t<double> pc3_copy_for_self_add = pc3_self;
  pc3_self += pc3_copy_for_self_add;
  CHECK(pc3_self.size() == 2);
  REQUIRE(pc3_self.points.size() >= 2);
  CHECK(pc3_self.points[0].x == Catch::Approx(1.0));
  CHECK(pc3_self.points[1].x == Catch::Approx(1.0));
  CHECK(pc3_self.intensity == Catch::Approx(2.0));

  point_cloud_t<double> pc4_move_target;
  pc4_move_target += point_t<double>(1.0, 1.0, 1.0);
  pc4_move_target.intensity = 10.0;
  point_cloud_t<double> pc5_move_source;
  pc5_move_source += point_t<double>(2.0, 2.0, 2.0);
  pc5_move_source.intensity = 5.0;

  pc4_move_target += std::move(pc5_move_source);
  CHECK(pc4_move_target.size() == 2);
  REQUIRE(pc4_move_target.points.size() >= 2);
  CHECK(pc4_move_target.points[1].x == Catch::Approx(2.0));
  CHECK(pc4_move_target.intensity == Catch::Approx(15.0));
  CHECK(pc5_move_source.empty());
  CHECK(pc5_move_source.intensity == Catch::Approx(0.0));
}

TEST_CASE("Point cloud copy and move", "[point_cloud]")
{
  point_cloud_t<int> pc1_orig;
  pc1_orig += point_t<int>(1, 1, 1);
  pc1_orig += point_t<int>(2, 2, 2);
  pc1_orig.intensity = 5;

  point_cloud_t<int> pc2_copy = pc1_orig;
  CHECK(pc2_copy.size() == 2);
  REQUIRE(pc2_copy.points.size() >= 1);
  CHECK(pc2_copy.points[0].x == 1);
  CHECK(pc2_copy.intensity == 5);
  if (!pc1_orig.points.empty() && !pc2_copy.points.empty()) {
    CHECK(pc1_orig.points.data() != pc2_copy.points.data());
  }

  point_cloud_t<int> pc1_to_move_from = pc1_orig;
  point_cloud_t<int> pc3_move = std::move(pc1_to_move_from);
  CHECK(pc3_move.size() == 2);
  REQUIRE(pc3_move.points.size() >= 1);
  CHECK(pc3_move.points[0].x == 1);
  CHECK(pc3_move.intensity == 5);
  CHECK(pc1_to_move_from.empty());
  CHECK(pc1_to_move_from.intensity == 0);

  point_cloud_t<int> pc4_copy_assign;
  pc4_copy_assign = pc3_move;
  CHECK(pc4_copy_assign.size() == 2);
  REQUIRE(pc4_copy_assign.points.size() >= 2);
  CHECK(pc4_copy_assign.points[1].x == 2);
  CHECK(pc4_copy_assign.intensity == 5);

  point_cloud_t<int> pc5_move_assign;
  point_cloud_t<int> pc4_to_move_assign_from = pc4_copy_assign;
  pc5_move_assign = std::move(pc4_to_move_assign_from);
  CHECK(pc5_move_assign.size() == 2);
  REQUIRE(pc5_move_assign.points.size() >= 2);
  CHECK(pc5_move_assign.points[1].x == 2);
  CHECK(pc5_move_assign.intensity == 5);
  CHECK(pc4_to_move_assign_from.empty());
  CHECK(pc4_to_move_assign_from.intensity == 0);
}

// --- Tests for SEQUENTIAL calculate_minmax ---

TEST_CASE("Sequential Calculate MinMax with single values",
          "[calculate_minmax][sequential]")
{
  SECTION("Integer")
  {
    auto mm = toolbox::types::calculate_minmax(5);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<int>>);
    CHECK(mm.min == 5);
    CHECK(mm.max == 5);
  }
  SECTION("Double")
  {
    auto mm = toolbox::types::calculate_minmax(-3.14);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<double>>);
    CHECK(mm.min == Catch::Approx(-3.14));
    CHECK(mm.max == Catch::Approx(-3.14));
  }
  SECTION("Point")
  {
    const point_t<float> p(1.0F, -2.0F, 3.0F);
    auto mm = toolbox::types::calculate_minmax(p);
    REQUIRE(
        std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<float>>>);
    CHECK(mm.min.x == Catch::Approx(1.0F));
    CHECK(mm.min.y == Catch::Approx(-2.0F));
    CHECK(mm.min.z == Catch::Approx(3.0F));
    CHECK(mm.max.x == Catch::Approx(1.0F));
    CHECK(mm.max.y == Catch::Approx(-2.0F));
    CHECK(mm.max.z == Catch::Approx(3.0F));
  }
}

TEST_CASE("Sequential Calculate MinMax with containers",
          "[calculate_minmax][sequential][container]")
{
  SECTION("Vector of Integers")
  {
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    auto mm =
        toolbox::types::calculate_minmax(vec);  // Calls sequential version
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<int>>);
    CHECK(mm.min == 1);
    CHECK(mm.max == 9);
  }
  SECTION("Empty Vector")
  {
    std::vector<double> empty_vec;
    auto mm = toolbox::types::calculate_minmax(empty_vec);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<double>>);
    CHECK(mm.min == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm.max == Catch::Approx(std::numeric_limits<double>::lowest()));
  }
  SECTION("Array of Floats")
  {
    std::array<float, 5> arr = {1.1F, -2.2F, 0.0F, 5.5F, -5.5F};
    auto mm = toolbox::types::calculate_minmax(arr);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<float>>);
    CHECK(mm.min == Catch::Approx(-5.5F));
    CHECK(mm.max == Catch::Approx(5.5F));
  }
  SECTION("Vector of Points")
  {
    std::vector<point_t<double>> points = {
        {1.0, 5.0, -2.0}, {-3.0, 6.0, 4.0}, {0.0, 0.0, 0.0}};
    auto mm =
        toolbox::types::calculate_minmax(points);  // Calls sequential version
    REQUIRE(std::is_same_v<decltype(mm),
                           toolbox::types::minmax_t<point_t<double>>>);
    CHECK(mm.min.x == Catch::Approx(-3.0));
    CHECK(mm.min.y == Catch::Approx(0.0));
    CHECK(mm.min.z == Catch::Approx(-2.0));
    CHECK(mm.max.x == Catch::Approx(1.0));
    CHECK(mm.max.y == Catch::Approx(6.0));
    CHECK(mm.max.z == Catch::Approx(4.0));
  }
  SECTION("Empty Vector of Points")
  {
    std::vector<point_t<float>> empty_points;
    auto mm = toolbox::types::calculate_minmax(empty_points);
    REQUIRE(
        std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<float>>>);
    // For empty containers, result should be uninitialized
    CHECK(!mm.initialized_);
    // Optionally check default values if needed, but initialized_ is key
    CHECK(mm.min.x == Catch::Approx(std::numeric_limits<float>::max()));
    CHECK(mm.max.x == Catch::Approx(std::numeric_limits<float>::lowest()));
  }
}

TEST_CASE("Sequential Calculate MinMax with PointCloud",
          "[calculate_minmax][sequential][point_cloud]")
{
  point_cloud_t<double> cloud;
  cloud += point_t<double>(1.0, 5.0, -2.0);
  cloud += point_t<double>(-3.0, 6.0, 4.0);
  cloud += point_t<double>(0.0, 0.0, 0.0);

  auto mm =
      toolbox::types::calculate_minmax(cloud);  // Calls sequential version
  REQUIRE(
      std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<double>>>);
  CHECK(mm.min.x == Catch::Approx(-3.0));
  CHECK(mm.min.y == Catch::Approx(0.0));
  CHECK(mm.min.z == Catch::Approx(-2.0));
  CHECK(mm.max.x == Catch::Approx(1.0));
  CHECK(mm.max.y == Catch::Approx(6.0));
  CHECK(mm.max.z == Catch::Approx(4.0));

  point_cloud_t<float> empty_cloud;
  auto mm_empty = toolbox::types::calculate_minmax(empty_cloud);
  REQUIRE(std::is_same_v<decltype(mm_empty),
                         toolbox::types::minmax_t<point_t<float>>>);
  // For empty cloud, result should be uninitialized
  CHECK(!mm_empty.initialized_);
  CHECK(mm_empty.min.x == Catch::Approx(std::numeric_limits<float>::max()));
  CHECK(mm_empty.max.x == Catch::Approx(std::numeric_limits<float>::lowest()));
}

// --- Tests for PARALLEL calculate_minmax_parallel ---

TEST_CASE("Parallel Calculate MinMax with single values",
          "[calculate_minmax][parallel]")
{
  // Parallel version just calls sequential for single items
  SECTION("Integer")
  {
    auto mm = toolbox::types::calculate_minmax_parallel(5);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<int>>);
    CHECK(mm.min == 5);
    CHECK(mm.max == 5);
  }
  SECTION("Double")
  {
    auto mm = toolbox::types::calculate_minmax_parallel(-3.14);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<double>>);
    CHECK(mm.min == Catch::Approx(-3.14));
    CHECK(mm.max == Catch::Approx(-3.14));
  }
  SECTION("Point")
  {
    const point_t<float> p(1.0F, -2.0F, 3.0F);
    auto mm = toolbox::types::calculate_minmax_parallel(p);
    REQUIRE(
        std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<float>>>);
    CHECK(mm.min.x == Catch::Approx(1.0F));
    CHECK(mm.min.y == Catch::Approx(-2.0F));
    CHECK(mm.min.z == Catch::Approx(3.0F));
    CHECK(mm.max.x == Catch::Approx(1.0F));
    CHECK(mm.max.y == Catch::Approx(-2.0F));
    CHECK(mm.max.z == Catch::Approx(3.0F));
  }
}

TEST_CASE("Parallel Calculate MinMax with containers",
          "[calculate_minmax][parallel][container]")
{
  SECTION("Vector of Integers (Small - Sequential Fallback)")
  {
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};  // Below threshold
    auto mm = toolbox::types::calculate_minmax_parallel(
        vec);  // Calls parallel (falls back)
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<int>>);
    CHECK(mm.min == 1);
    CHECK(mm.max == 9);
  }
  SECTION("Vector of Integers (Large - Parallel Execution)")
  {
    std::vector<int> large_vec(2048);  // Larger than threshold in minmax.hpp
    std::iota(large_vec.begin(), large_vec.end(), -1000);
    large_vec[500] = -2000;  // Set min
    large_vec[1500] = 3000;  // Set max
    auto mm = toolbox::types::calculate_minmax_parallel(large_vec);
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<int>>);
    CHECK(mm.min == -2000);
    CHECK(mm.max == 3000);
  }
  SECTION("Empty Vector")
  {
    std::vector<double> empty_vec;
    auto mm = toolbox::types::calculate_minmax_parallel(
        empty_vec);  // Calls parallel (handles empty)
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<double>>);
    CHECK(mm.min == Catch::Approx(std::numeric_limits<double>::max()));
    CHECK(mm.max == Catch::Approx(std::numeric_limits<double>::lowest()));
  }
  SECTION("Array of Floats (Small - Sequential Fallback)")
  {
    std::array<float, 5> arr = {
        1.1F, -2.2F, 0.0F, 5.5F, -5.5F};  // Below threshold
    auto mm = toolbox::types::calculate_minmax_parallel(
        arr);  // Calls parallel (falls back)
    REQUIRE(std::is_same_v<decltype(mm), toolbox::types::minmax_t<float>>);
    CHECK(mm.min == Catch::Approx(-5.5F));
    CHECK(mm.max == Catch::Approx(5.5F));
  }
  SECTION("Vector of Points (Small - Sequential Fallback)")
  {
    std::vector<point_t<double>> points = {
        {1.0, 5.0, -2.0}, {-3.0, 6.0, 4.0}, {0.0, 0.0, 0.0}};
    auto mm = toolbox::types::calculate_minmax_parallel(
        points);  // Calls parallel (falls back)
    REQUIRE(std::is_same_v<decltype(mm),
                           toolbox::types::minmax_t<point_t<double>>>);
    CHECK(mm.min.x == Catch::Approx(-3.0));
    CHECK(mm.min.y == Catch::Approx(0.0));
    CHECK(mm.min.z == Catch::Approx(-2.0));
    CHECK(mm.max.x == Catch::Approx(1.0));
    CHECK(mm.max.y == Catch::Approx(6.0));
    CHECK(mm.max.z == Catch::Approx(4.0));
  }
  SECTION("Vector of Points (Large - Parallel Execution)")
  {
    constexpr size_t large_size = 2048;
    std::vector<point_t<double>> large_points(large_size);

    // Generate data with a simple, predictable pattern
    // x = [0, 2047], y = [1, 2048], z = [-2047, 0]
    for (size_t i = 0; i < large_size; ++i) {
      large_points[i] = {static_cast<double>(i),
                         static_cast<double>(large_size - i),
                         static_cast<double>(-static_cast<double>(i))};
    }

    // Define min/max points clearly outside the generated range
    const point_t<double> min_point = {
        -10.0, -10.0, -static_cast<double>(large_size) - 10.0};
    const point_t<double> max_point = {static_cast<double>(large_size) + 10.0,
                                       static_cast<double>(large_size) + 10.0,
                                       10.0};

    // Insert min/max points
    large_points[large_size / 3] = min_point;
    large_points[large_size * 2 / 3] = max_point;

    // Calculate min/max in parallel
    auto mm = toolbox::types::calculate_minmax_parallel(large_points);

    REQUIRE(std::is_same_v<decltype(mm),
                           toolbox::types::minmax_t<point_t<double>>>);

    // Check against the explicitly inserted min/max points
    CHECK(mm.min.x == Catch::Approx(min_point.x));
    CHECK(mm.min.y == Catch::Approx(min_point.y));
    CHECK(mm.min.z == Catch::Approx(min_point.z));
    CHECK(mm.max.x == Catch::Approx(max_point.x));
    CHECK(mm.max.y == Catch::Approx(max_point.y));
    CHECK(mm.max.z == Catch::Approx(max_point.z));
  }
  SECTION("Empty Vector of Points")
  {
    std::vector<point_t<float>> empty_points;
    auto mm = toolbox::types::calculate_minmax(
        empty_points);  // Calls parallel (handles empty -> sequential)
    REQUIRE(
        std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<float>>>);
    // For empty containers, result should be uninitialized
    CHECK(!mm.initialized_);
    // Optionally check default values
    CHECK(mm.min.x == Catch::Approx(std::numeric_limits<float>::max()));
    CHECK(mm.max.x == Catch::Approx(std::numeric_limits<float>::lowest()));
  }
}

TEST_CASE("Parallel Calculate MinMax with PointCloud",
          "[calculate_minmax][parallel][point_cloud]")
{
  point_cloud_t<double> cloud;
  cloud += point_t<double>(1.0, 5.0, -2.0);
  cloud += point_t<double>(-3.0, 6.0, 4.0);
  cloud += point_t<double>(0.0, 0.0, 0.0);

  auto mm = toolbox::types::calculate_minmax_parallel(
      cloud);  // Calls parallel version (falls back)
  REQUIRE(
      std::is_same_v<decltype(mm), toolbox::types::minmax_t<point_t<double>>>);
  CHECK(mm.min.x == Catch::Approx(-3.0));
  CHECK(mm.min.y == Catch::Approx(0.0));
  CHECK(mm.min.z == Catch::Approx(-2.0));
  CHECK(mm.max.x == Catch::Approx(1.0));
  CHECK(mm.max.y == Catch::Approx(6.0));
  CHECK(mm.max.z == Catch::Approx(4.0));

  point_cloud_t<double> large_cloud;
  constexpr size_t large_size = 2048;
  large_cloud.points.resize(large_size);

  // Generate data with the same simple pattern as the vector test
  // x = [0, 2047], y = [1, 2048], z = [-2047, 0]
  for (size_t i = 0; i < large_size; ++i) {
    large_cloud.points[i] = {static_cast<double>(i),
                             static_cast<double>(large_size - i),
                             static_cast<double>(-static_cast<double>(i))};
  }

  // Define and insert the same min/max points
  const point_t<double> min_point = {
      -10.0, -10.0, -static_cast<double>(large_size) - 10.0};
  const point_t<double> max_point = {static_cast<double>(large_size) + 10.0,
                                     static_cast<double>(large_size) + 10.0,
                                     10.0};
  large_cloud.points[large_size / 3] = min_point;
  large_cloud.points[large_size * 2 / 3] = max_point;

  // Calculate min/max in parallel
  auto mm_large = toolbox::types::calculate_minmax_parallel(large_cloud);

  REQUIRE(std::is_same_v<decltype(mm_large),
                         toolbox::types::minmax_t<point_t<double>>>);

  // Check against the explicitly inserted min/max points
  CHECK(mm_large.min.x == Catch::Approx(min_point.x));
  CHECK(mm_large.min.y == Catch::Approx(min_point.y));
  CHECK(mm_large.min.z == Catch::Approx(min_point.z));
  CHECK(mm_large.max.x == Catch::Approx(max_point.x));
  CHECK(mm_large.max.y == Catch::Approx(max_point.y));
  CHECK(mm_large.max.z == Catch::Approx(max_point.z));

  point_cloud_t<float> empty_cloud;
  auto mm_empty = toolbox::types::calculate_minmax_parallel(empty_cloud);
  REQUIRE(std::is_same_v<decltype(mm_empty),
                         toolbox::types::minmax_t<point_t<float>>>);
  // For empty cloud, result should be uninitialized
  CHECK(!mm_empty.initialized_);
  CHECK(mm_empty.min.x == Catch::Approx(std::numeric_limits<float>::max()));
  CHECK(mm_empty.max.x == Catch::Approx(std::numeric_limits<float>::lowest()));
}

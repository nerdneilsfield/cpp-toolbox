#include <limits>  // For numeric_limits
#include <numeric>  // For std::iota in vector<double> benchmark
#include <random>  // For generating random points
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

// Include the headers to be benchmarked
#include <cpp-toolbox/types/minmax.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/types/point_utils.hpp>  // Contains point generation functions

// --- Helper Function to generate random points ---
std::vector<toolbox::types::point_t<double>> generate_random_points(
    size_t count)
{
  std::vector<toolbox::types::point_t<double>> points;
  points.reserve(count);

  std::mt19937 gen(12345);  // Fixed seed for reproducibility
  std::uniform_real_distribution<double> distrib(-1000.0, 1000.0);

  for (size_t i = 0; i < count; ++i) {
    points.emplace_back(distrib(gen), distrib(gen), distrib(gen));
  }
  return points;
}

// --- Benchmarks ---

TEST_CASE("Benchmark Types Utilities")
{
  // --- Setup Common Data ---
  const size_t data_size = 500'000;  // Half a million elements

  // Define bounds for point generation
  toolbox::types::minmax_t<toolbox::types::point_t<double>> bounds(
      toolbox::types::point_t<double>(-1000.0, -1000.0, -1000.0),
      toolbox::types::point_t<double>(1000.0, 1000.0, 1000.0));

  // Pre-generate data for minmax benchmarks to isolate calculation time
  auto points_vector_for_minmax =
      toolbox::types::generate_random_points(data_size, bounds);
  REQUIRE(points_vector_for_minmax.size() == data_size);
  toolbox::types::point_cloud_t<double> point_cloud;
  point_cloud.points = points_vector_for_minmax;  // Copy points
  REQUIRE(point_cloud.size() == data_size);
  std::vector<double> double_vector(data_size);
  std::iota(double_vector.begin(), double_vector.end(), 0.0);
  REQUIRE(double_vector.size() == data_size);

  // --- Benchmark MinMax Calculation Sections ---

  SECTION("MinMax Calculation: Vector of Points")
  {
    BENCHMARK("Sequential MinMax (vector<point_t>)")
    {
      auto result = toolbox::types::calculate_minmax(points_vector_for_minmax);
      return result.min.x;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (vector<point_t>)")
    {
      auto result =
          toolbox::types::calculate_minmax_parallel(points_vector_for_minmax);
      return result.min.x;  // Prevent optimization
    };
  }

  SECTION("MinMax Calculation: PointCloud")
  {
    BENCHMARK("Sequential MinMax (point_cloud_t)")
    {
      auto result = toolbox::types::calculate_minmax(point_cloud);
      return result.min.x;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (point_cloud_t)")
    {
      auto result = toolbox::types::calculate_minmax_parallel(point_cloud);
      return result.min.x;  // Prevent optimization
    };
  }

  SECTION("MinMax Calculation: Vector of Doubles")
  {
    BENCHMARK("Sequential MinMax (vector<double>)")
    {
      auto result = toolbox::types::calculate_minmax(double_vector);
      return result.min;  // Prevent optimization
    };

    BENCHMARK("Parallel MinMax (vector<double>)")
    {
      auto result = toolbox::types::calculate_minmax_parallel(double_vector);
      return result.min;  // Prevent optimization
    };
  }

  // --- Benchmark Point Generation Section ---
  SECTION("Point Generation")
  {
    // Use a slightly smaller size for generation benchmark if needed,
    // as generation might be slower overall. Sticking to data_size for now.
    const size_t generation_size = data_size;

    BENCHMARK("Sequential Generation (generate_random_points)")
    {
      // Generate points inside the benchmark
      auto points =
          toolbox::types::generate_random_points(generation_size, bounds);
      return points.size();  // Return size to prevent optimization
    };

    BENCHMARK("Parallel Generation (generate_random_points_parallel)")
    {
      // Generate points inside the benchmark
      auto points = toolbox::types::generate_random_points_parallel(
          generation_size, bounds);
      return points.size();  // Return size to prevent optimization
    };
  }
}

// Reminder: Ensure cpp-toolbox library (especially thread pool) is linked
// correctly and include directories are set up in your CMakeLists.txt for this
// benchmark target.

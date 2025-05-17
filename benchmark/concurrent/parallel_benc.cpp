#include <algorithm>  // For std::for_each, std::transform
// #include <execution>  // For std::execution::par (requires C++17 and
// potentially TBB)
#include <chrono>
#include <iostream>  // For potential error output
#include <numeric>  // For std::accumulate, std::iota
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "cpp-toolbox/utils/plot.hpp"

// Include your parallel header
#include <cpp-toolbox/concurrent/parallel.hpp>  // Corrected include path

// --- Helper Functions ---

// Serial sum function
long long serial_sum(const std::vector<int>& data)
{
  return std::accumulate(data.begin(), data.end(), 0LL);
}

// Parallel sum using std::execution::par (Requires C++17 and proper
// compiler/linker flags)
/* // Temporarily commented out due to potential build configuration issues
long long std_parallel_sum(const std::vector<int>& data)
{
  // Note: This might cause compile errors if C++17 parallel algorithms aren't
  // properly configured The header <execution> is needed. Linker flags might be
  // required (e.g. -ltbb)
  try {
    // Ensure C++17 is enabled in your build system
    return std::accumulate(std::execution::par, data.begin(), data.end(), 0LL);
  } catch (const std::exception& e) {
    // Handle cases where parallel execution is not supported or fails
    // Using std::cerr for errors is generally better than mixing with benchmark
    // output std::cerr << "std::execution::par failed: " << e.what() <<
    // std::endl; For a benchmark, perhaps just return a value indicating
    // failure or fallback. Returning the serial sum might skew results if
    // failure is intermittent. Let's return an unlikely value or rethrow
    // depending on desired behavior. For simplicity here, we just return 0, but
    // acknowledge this isn't ideal.
    return 0;  // Indicate error/fallback (not ideal for benchmarking accuracy
               // on failure)
  } catch (...) {
    // Catch any other potential exceptions
    return 0;  // Indicate error/fallback
  }
}
*/

// Parallel sum using your toolbox implementation
long long toolbox_parallel_sum(const std::vector<int>& data)
{
  // Using cbegin/cend as data is const. Assuming identity 0LL for long long
  // sum.
  return toolbox::concurrent::parallel_reduce(
      data.cbegin(), data.cend(), 0LL, std::plus<long long>());
}

// Function to apply in for_each/transform benchmarks (e.g., square the number)
int square_op(int x)
{
  return x * x;
}
void square_in_place_op(int& x)
{
  x *= x;
}

// --- Benchmarks ---

TEST_CASE("Benchmark Parallel Algorithms")
{
  // Prepare large test data
  const size_t data_size = 1'000'000;  // One million elements
  std::vector<int> data(data_size);
  std::iota(data.begin(), data.end(), 1);  // Fill with 1, 2, 3, ...

  std::vector<int> output_data(data_size);  // For transform output
  std::vector<long long> scan_output(data_size);  // For inclusive scan

  // --- Correctness checks -------------------------------------------------
  SECTION("Correctness")
  {
    long long expected_sum = serial_sum(data);
    REQUIRE(toolbox_parallel_sum(data) == expected_sum);

    std::vector<int> expected_for_each = data;
    std::for_each(
        expected_for_each.begin(), expected_for_each.end(), square_in_place_op);
    std::vector<int> parallel_for_each_vec = data;
    toolbox::concurrent::parallel_for_each(parallel_for_each_vec.begin(),
                                           parallel_for_each_vec.end(),
                                           square_in_place_op);
    REQUIRE(parallel_for_each_vec == expected_for_each);

    std::vector<int> expected_transform(data_size);
    std::transform(
        data.cbegin(), data.cend(), expected_transform.begin(), square_op);
    std::vector<int> parallel_transform_out(data_size);
    toolbox::concurrent::parallel_transform(
        data.cbegin(), data.cend(), parallel_transform_out.begin(), square_op);
    REQUIRE(parallel_transform_out == expected_transform);

    std::vector<long long> expected_scan(data_size);
    std::inclusive_scan(data.cbegin(),
                        data.cend(),
                        expected_scan.begin(),
                        std::plus<long long>(),
                        0LL);
    std::vector<long long> parallel_scan_out(data_size);
    toolbox::concurrent::parallel_inclusive_scan(data.cbegin(),
                                                 data.cend(),
                                                 parallel_scan_out.begin(),
                                                 0LL,
                                                 std::plus<long long>(),
                                                 0LL);
    REQUIRE(parallel_scan_out == expected_scan);

    std::vector<int> expected_sort = data;
    std::sort(expected_sort.begin(), expected_sort.end());
    std::vector<int> parallel_sort = data;
    toolbox::concurrent::parallel_merge_sort(parallel_sort.begin(),
                                             parallel_sort.end());
    REQUIRE(parallel_sort == expected_sort);

    std::vector<int> expected_tim = data;
    std::stable_sort(expected_tim.begin(), expected_tim.end());
    std::vector<int> tim_sorted = data;
    toolbox::concurrent::parallel_tim_sort(tim_sorted.begin(),
                                           tim_sorted.end());
    REQUIRE(tim_sorted == expected_tim);
  }

  // --- Benchmark Reduction (Summation) ---
  SECTION("Reduction Benchmarks")
  {
    // Verify correctness first (optional but recommended)
    // long long expected_sum = serial_sum(data);
    // REQUIRE(toolbox_parallel_sum(data) == expected_sum);
    // if constexpr (/* check if std::execution::par is expected to work */
    // false) {
    //    REQUIRE(std_parallel_sum(data) == expected_sum);
    // }

    BENCHMARK("Serial Sum (std::accumulate)")
    {
      return serial_sum(data);
    };

    // Optional: Benchmark std::execution::par if configured
    /* // Temporarily commented out
    BENCHMARK("Parallel Sum (std::execution::par)") {
         return std_parallel_sum(data);
    };
    */

    BENCHMARK("Parallel Sum (toolbox::parallel_reduce)")
    {
      return toolbox_parallel_sum(data);
    };
  }

  // --- Benchmark Inclusive Scan ---
  SECTION("Inclusive Scan Benchmarks")
  {
    BENCHMARK("Serial Inclusive Scan (std::inclusive_scan)")
    {
      std::inclusive_scan(data.begin(), data.end(), scan_output.begin());
      return scan_output.back();
    };

    BENCHMARK("Parallel Inclusive Scan (toolbox::parallel_inclusive_scan)")
    {
      toolbox::concurrent::parallel_inclusive_scan(data.begin(),
                                                   data.end(),
                                                   scan_output.begin(),
                                                   0LL,
                                                   std::plus<long long>(),
                                                   0LL);
      return scan_output.back();
    };
  }

  // --- Benchmark For Each (Apply function in-place) ---
  SECTION("For Each Benchmarks")
  {
    // Need a copy for in-place modification benchmarks
    std::vector<int> data_copy_for_each;  // Declare outside benchmark

    BENCHMARK_ADVANCED("Serial For Each (range-based for)")(
        Catch::Benchmark::Chronometer meter)
    {
      data_copy_for_each = data;  // Reset data for each run inside the lambda
      meter.measure(
          [&]()
          {
            for (int& x : data_copy_for_each) {
              square_in_place_op(x);
            }
            // Prevent optimization by returning a value depending on the result
            return data_copy_for_each.back();
          });
    };

    BENCHMARK_ADVANCED("Serial For Each (std::for_each)")(
        Catch::Benchmark::Chronometer meter)
    {
      data_copy_for_each = data;  // Reset data for each run
      meter.measure(
          [&]()
          {
            std::for_each(data_copy_for_each.begin(),
                          data_copy_for_each.end(),
                          square_in_place_op);
            return data_copy_for_each.back();  // Prevent optimization
          });
    };

    // Optional: Benchmark std::execution::par for_each if configured
    /* // Temporarily commented out
    BENCHMARK_ADVANCED("Parallel For Each
    (std::execution::par)")(Catch::Benchmark::Chronometer meter) {
         data_copy_for_each = data; // Reset data for each run
         meter.measure([&]() {
             std::for_each(std::execution::par, data_copy_for_each.begin(),
    data_copy_for_each.end(), square_in_place_op); return
    data_copy_for_each.back(); // Prevent optimization
         });
     };
    */

    BENCHMARK_ADVANCED("Parallel For Each (toolbox::parallel_for_each)")(
        Catch::Benchmark::Chronometer meter)
    {
      data_copy_for_each = data;  // Reset data for each run
      meter.measure(
          [&]()
          {
            toolbox::concurrent::parallel_for_each(data_copy_for_each.begin(),
                                                   data_copy_for_each.end(),
                                                   square_in_place_op);
            return data_copy_for_each.back();  // Prevent optimization
          });
    };
  }

  // --- Benchmark Transform (Apply function to output) ---
  SECTION("Transform Benchmarks")
  {
    // Verify correctness first (optional)
    // std::vector<int> expected_output(data_size);
    // std::transform(data.cbegin(), data.cend(), expected_output.begin(),
    // square_op); std::vector<int> actual_output(data_size);

    BENCHMARK("Serial Transform (std::transform)")
    {
      std::transform(
          data.cbegin(), data.cend(), output_data.begin(), square_op);
      return output_data.back();  // Return something to prevent optimization
    };

    // Optional: Benchmark std::execution::par transform if configured
    /* // Temporarily commented out
    BENCHMARK("Parallel Transform (std::execution::par)") {
         std::transform(std::execution::par, data.cbegin(), data.cend(),
    output_data.begin(), square_op); return output_data.back();
    };
    */

    BENCHMARK("Parallel Transform (toolbox::parallel_transform)")
    {
      // Ensure output_data has the correct size before calling
      toolbox::concurrent::parallel_transform(
          data.cbegin(), data.cend(), output_data.begin(), square_op);
      // REQUIRE(output_data == expected_output); // Check correctness within
      // benchmark is tricky
      return output_data.back();  // Return something to prevent optimization
    };
  }

  // --- Benchmark Merge Sort ---
  SECTION("Merge Sort Benchmarks")
  {
    std::vector<int> sort_tmp;

    BENCHMARK_ADVANCED("Serial Sort (std::sort)")(
        Catch::Benchmark::Chronometer meter)
    {
      meter.measure(
          [&]()
          {
            sort_tmp = data;
            std::sort(sort_tmp.begin(), sort_tmp.end());
            return sort_tmp.back();
          });
    };

    BENCHMARK_ADVANCED("Parallel Merge Sort (toolbox::parallel_merge_sort)")(
        Catch::Benchmark::Chronometer meter)
    {
      meter.measure(
          [&]()
          {
            sort_tmp = data;
            toolbox::concurrent::parallel_merge_sort(sort_tmp.begin(),
                                                     sort_tmp.end());
            return sort_tmp.back();
          });
    };
  }

  // --- Benchmark Tim Sort ---
  SECTION("Tim Sort Benchmarks")
  {
    std::vector<int> sort_tmp;

    BENCHMARK_ADVANCED("Serial Stable Sort (std::stable_sort)")(
        Catch::Benchmark::Chronometer meter)
    {
      meter.measure(
          [&]()
          {
            sort_tmp = data;
            std::stable_sort(sort_tmp.begin(), sort_tmp.end());
            return sort_tmp.back();
          });
    };

    BENCHMARK_ADVANCED("Parallel Tim Sort (toolbox::parallel_tim_sort)")(
        Catch::Benchmark::Chronometer meter)
    {
      meter.measure(
          [&]()
          {
            sort_tmp = data;
            toolbox::concurrent::parallel_tim_sort(sort_tmp.begin(),
                                                   sort_tmp.end());
            return sort_tmp.back();
          });
    };
  }

  // --- Simple Timing Plot -------------------------------------------------
  SECTION("Timing Plot")
  {
    using namespace std::chrono;
    auto measure = [&](auto&& func)
    {
      const int iters = 3;
      double total = 0.0;
      for (int i = 0; i < iters; ++i) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        total += duration<double>(end - start).count();
      }
      return total / static_cast<double>(iters);
    };

    double t_serial = measure([&]() { serial_sum(data); });
    double t_parallel = measure([&]() { toolbox_parallel_sum(data); });

    toolbox::utils::plot_t plot;
    plot.set_title("Sum Benchmark (sec)");
    plot.set_x_axis();
    plot.set_y_axis();
    plot.enable_axis_grid();
    plot.add_scatter_series({1.0, 2.0},
                            {t_serial, t_parallel},
                            toolbox::utils::color_t::GREEN,
                            toolbox::utils::plot_t::style_t::CROSS);
    std::cout << plot.render(40, 10) << "\n";

    REQUIRE(t_serial > 0.0);
    REQUIRE(t_parallel > 0.0);
  }
}

// It's good practice to ensure the thread pool singleton is properly managed,
// though for simple benchmarks it might not be strictly necessary if the
// program exits cleanly. You might need a dedicated setup/teardown
// if your thread pool requires explicit shutdown. For example:
// struct ThreadPoolManager {
//    ThreadPoolManager() { /* Optional: Explicit init if needed */ }
//    ~ThreadPoolManager() {
//        // Ensure pool is shutdown cleanly, especially if threads might
//        outlive main try {
//            toolbox::concurrent::default_pool().shutdown(); // Adjust if
//            shutdown method differs
//        } catch (const std::exception& e) {
//            std::cerr << "Error shutting down thread pool: " << e.what() <<
//            std::endl;
//        } catch (...) {
//            std::cerr << "Unknown error shutting down thread pool." <<
//            std::endl;
//        }
//    }
// };
// static ThreadPoolManager pool_manager; // Static object to manage pool
// lifetime

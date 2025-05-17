#include <algorithm>  // For std::for_each, std::transform, std::sort
#include <random>
#include <iomanip>
#include <sstream>
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
  const size_t data_size = 5'000'000;  // Five million elements to stress parallelism
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

  // --- Benchmark Inclusive Scan (Prefix Sum) ---
  SECTION("Inclusive Scan Benchmarks")
  {
    std::vector<int> scan_out(data_size);

    BENCHMARK("Serial Inclusive Scan (std::inclusive_scan)")
    {
      std::inclusive_scan(data.cbegin(), data.cend(), scan_out.begin());
      return scan_out.back();
    };

    BENCHMARK("Parallel Inclusive Scan (toolbox::parallel_inclusive_scan)")
    {
      toolbox::concurrent::parallel_inclusive_scan(data.cbegin(),
                                                   data.cend(),
                                                   scan_out.begin(),
                                                   0,
                                                   std::plus<int>(),
                                                   0);
      return scan_out.back();
    };
  }

  // --- Benchmark Merge Sort ---
  SECTION("Merge Sort Benchmarks")
  {
    std::vector<int> sort_data(data_size);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    for (auto& v : sort_data) {
      v = dist(rng);
    }

    BENCHMARK("Serial Sort (std::sort)")
    {
      auto tmp = sort_data;
      std::sort(tmp.begin(), tmp.end());
      return tmp.back();
    };

    BENCHMARK("Parallel Merge Sort (toolbox::parallel_merge_sort)")
    {
      auto tmp = sort_data;
      toolbox::concurrent::parallel_merge_sort(tmp.begin(), tmp.end());
      return tmp.back();
    };
  }

  // --- Timing Table and Plot ---------------------------------------------
  SECTION("Timing Table")
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

    // Measure all algorithms
    double reduce_serial = measure([&]() { serial_sum(data); });
    double reduce_parallel = measure([&]() { toolbox_parallel_sum(data); });

    double for_each_serial = measure([&]() {
      std::vector<int> tmp = data;
      std::for_each(tmp.begin(), tmp.end(), square_in_place_op);
    });
    double for_each_parallel = measure([&]() {
      std::vector<int> tmp = data;
      toolbox::concurrent::parallel_for_each(tmp.begin(), tmp.end(),
                                             square_in_place_op);
    });

    double transform_serial = measure([&]() {
      std::transform(data.begin(), data.end(), output_data.begin(), square_op);
    });
    double transform_parallel = measure([&]() {
      toolbox::concurrent::parallel_transform(data.begin(), data.end(),
                                              output_data.begin(), square_op);
    });

    std::vector<int> scan_tmp(data_size);
    double scan_serial = measure([&]() {
      std::inclusive_scan(data.begin(), data.end(), scan_tmp.begin());
    });
    double scan_parallel = measure([&]() {
      toolbox::concurrent::parallel_inclusive_scan(data.begin(), data.end(),
                                                   scan_tmp.begin(), 0,
                                                   std::plus<int>(), 0);
    });

    std::vector<int> sort_input(data_size);
    {
      std::mt19937 rng(123);
      std::uniform_int_distribution<int> dist(0, 1'000'000);
      for (auto& v : sort_input) {
        v = dist(rng);
      }
    }
    double sort_serial = measure([&]() {
      auto tmp = sort_input;
      std::sort(tmp.begin(), tmp.end());
    });
    double sort_parallel = measure([&]() {
      auto tmp = sort_input;
      toolbox::concurrent::parallel_merge_sort(tmp.begin(), tmp.end());
    });

    toolbox::utils::table_t table;
    table.set_headers({"Benchmark", "Serial (s)", "Parallel (s)", "Speedup"});
    auto add_row = [&](const std::string& name, double s, double p) {
      std::ostringstream ss;
      ss.setf(std::ios::fixed);
      ss << std::setprecision(6) << s;
      std::ostringstream sp;
      sp.setf(std::ios::fixed);
      sp << std::setprecision(6) << p;
      double speedup = s / p;
      std::ostringstream sd;
      sd.setf(std::ios::fixed);
      sd << std::setprecision(2) << speedup;
      table.add_row(name, ss.str(), sp.str(), sd.str());
    };

    add_row("Reduce", reduce_serial, reduce_parallel);
    add_row("For Each", for_each_serial, for_each_parallel);
    add_row("Transform", transform_serial, transform_parallel);
    add_row("Inclusive Scan", scan_serial, scan_parallel);
    add_row("Merge Sort", sort_serial, sort_parallel);

    std::cout << table << "\n";

    toolbox::utils::plot_t plot;
    plot.set_title("Sum Benchmark (sec)");
    plot.set_x_axis();
    plot.set_y_axis();
    plot.enable_axis_grid();
    plot.add_scatter_series({1.0, 2.0},
                            {reduce_serial, reduce_parallel},
                            toolbox::utils::color_t::GREEN,
                            toolbox::utils::plot_t::style_t::CROSS);
    std::cout << plot.render(40, 10) << "\n";

    REQUIRE(reduce_serial > 0.0);
    REQUIRE(reduce_parallel > 0.0);
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

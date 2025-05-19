#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>  // For std::numeric_limits
#include <numeric>  // For std::accumulate
#include <random>  // For random data generation
#include <sstream>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/file/memory_mapped_file.hpp>
#include <cpp-toolbox/utils/print.hpp>
#include <cpp-toolbox/utils/timer.hpp>

// Helper function to create a large file with random binary data
// Returns true on success, false otherwise.
bool create_large_random_file(const std::filesystem::path& path,
                              size_t size_bytes)
{
  std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
  if (!ofs) {
    std::cerr << "Benchmark Error: Failed to open file for writing: " << path
              << std::endl;
    return false;
  }

  // Use a random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  // Use unsigned int for distribution, then cast to char
  std::uniform_int_distribution<unsigned int> distrib(
      0, std::numeric_limits<unsigned char>::max());

  // Write random data in chunks
  size_t buffer_size = 4096;
  std::vector<char> buffer(buffer_size);
  size_t bytes_written = 0;
  while (bytes_written < size_bytes) {
    size_t chunk_size = std::min(buffer_size, size_bytes - bytes_written);
    for (size_t i = 0; i < chunk_size; ++i) {
      // Cast the generated unsigned int back to char for the buffer
      buffer[i] = static_cast<char>(distrib(gen));
    }
    ofs.write(buffer.data(), chunk_size);
    if (!ofs) {
      std::cerr << "Benchmark Error: Failed to write to file: " << path
                << std::endl;
      std::filesystem::remove(path);  // Attempt cleanup
      return false;
    }
    bytes_written += chunk_size;
  }

  ofs.close();
  if (!ofs) {
    std::cerr << "Benchmark Error: Failed to close file properly: " << path
              << std::endl;
    // File might still be usable, but log the error.
  }
  std::cout << "Benchmark Info: Created large file: " << path
            << " with size: " << bytes_written << " bytes." << std::endl;
  return true;
}

TEST_CASE("File Reading Benchmark", "[hide][benchmark][file]")
{
  // --- Benchmark Setup ---
  const size_t file_size_mb = 128;
  const size_t file_size_bytes = file_size_mb * 1024 * 1024;
  const std::filesystem::path benchmark_file_path = "large_benchmark_file.bin";

  // Create the large file before running benchmarks
  if (!create_large_random_file(benchmark_file_path, file_size_bytes)) {
    FAIL(
        "Benchmark Error: Failed to create the large benchmark file. Aborting "
        "benchmark.");
  }

  // Ensure the file exists and has the correct size
  std::error_code ec;
  uintmax_t actual_size = std::filesystem::file_size(benchmark_file_path, ec);
  if (ec || actual_size != file_size_bytes) {
    std::filesystem::remove(benchmark_file_path);  // Cleanup
    FAIL("Benchmark Error: Benchmark file size verification failed. Expected: "
         << file_size_bytes << " Got: " << actual_size
         << " Error: " << ec.message());
  }

  // --- Benchmarks ---

  BENCHMARK("Traditional Read (ifstream)")
  {
    std::ifstream ifs(
        benchmark_file_path,
        std::ios::binary | std::ios::ate);  // Open at end to get size
    if (!ifs)
      return false;  // Indicate failure if file cannot be opened
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);  // Seek back to beginning

    std::vector<char> buffer(size);
    if (ifs.read(buffer.data(), size)) {
      // Optional: Access data to prevent optimization removing the read
      // return std::accumulate(buffer.begin(), buffer.end(), (unsigned long
      // long)0);
      return true;  // Indicate success
    }
    return false;  // Indicate failure
  };

  BENCHMARK("Memory Mapped Access (read all bytes)")
  {
    toolbox::file::memory_mapped_file_t mapped_file;
    if (mapped_file.open(benchmark_file_path)) {
      const unsigned char* data = mapped_file.data();
      size_t size = mapped_file.size();
      // Access data to ensure pages are loaded (e.g., calculate sum)
      // Use volatile to potentially prevent compiler optimizing the loop away
      volatile unsigned long long sum = 0;
      for (size_t i = 0; i < size; ++i) {
        sum += data[i];
      }
      mapped_file.close();
      return true;  // Indicate success
    } else {
      return false;  // Indicate failure
    }
  };

  // --- Timing Table ---
  SECTION("Timing Table")
  {
    // 使用 toolbox::utils::stop_watch_timer_t 进行更准确的计时
    // Use toolbox::utils::stop_watch_timer_t for more accurate timing
    auto measure = [&](auto&& func)
    {
      const int iters = 5;  // 增加迭代次数以获得更稳定的结果
      double total_ms = 0.0;
      for (int i = 0; i < iters; ++i) {
        toolbox::utils::stop_watch_timer_t timer;
        timer.start();
        func();
        timer.stop();
        total_ms += timer.elapsed_time_ms();  // 直接获取毫秒值
      }
      return total_ms / static_cast<double>(iters);
    };

    // Measure both methods
    double traditional_read = measure(
        [&]()
        {
          std::ifstream ifs(benchmark_file_path,
                            std::ios::binary | std::ios::ate);
          if (!ifs)
            return;
          std::streamsize size = ifs.tellg();
          ifs.seekg(0, std::ios::beg);
          std::vector<char> buffer(size);
          ifs.read(buffer.data(), size);
        });

    double memory_mapped = measure(
        [&]()
        {
          toolbox::file::memory_mapped_file_t mapped_file;
          if (mapped_file.open(benchmark_file_path)) {
            const unsigned char* data = mapped_file.data();
            size_t size = mapped_file.size();
            volatile unsigned long long sum = 0;
            for (size_t i = 0; i < size; ++i) {
              sum += data[i];
            }
            mapped_file.close();
          }
        });

    // Create and display the results table
    toolbox::utils::table_t table;
    table.set_headers({"Benchmark", "Time (ms)", "Relative Speed"});
    auto add_row =
        [&](const std::string& name, double time_ms, double reference_ms)
    {
      // 添加调试输出，显示实际的计时值
      // Add debug output to show actual timing values
      std::cout << "DEBUG - " << name << " - Time: " << time_ms
                << " ms, Reference: " << reference_ms
                << " ms, Relative: " << (reference_ms / time_ms) << "x\n";

      // 已经是毫秒值，不需要转换
      // Already in milliseconds, no conversion needed
      std::ostringstream time_str;
      time_str.setf(std::ios::fixed);
      time_str << std::setprecision(3) << time_ms;

      // 计算相对速度
      // Calculate relative speed
      double relative = 1.0;
      if (time_ms > 0.001) {  // 避免除以非常小的值 / Avoid division by very
                              // small values
        relative = reference_ms / time_ms;
      }

      std::ostringstream relative_str;
      relative_str.setf(std::ios::fixed);
      relative_str << std::setprecision(2) << relative << "x";

      table.add_row(name, time_str.str(), relative_str.str());
    };

    // Add rows with traditional read as the reference
    add_row("Traditional Read", traditional_read, traditional_read);
    add_row("Memory Mapped", memory_mapped, traditional_read);

    std::cout << table << "\n";

    REQUIRE(traditional_read > 0.0);
    REQUIRE(memory_mapped > 0.0);
  }

  // --- Benchmark Teardown ---
  std::cout << "Benchmark Info: Removing large file: " << benchmark_file_path
            << "\n";
  std::filesystem::remove(benchmark_file_path);
}

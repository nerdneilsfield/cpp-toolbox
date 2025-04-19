#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>  // For std::numeric_limits
#include <numeric>  // For std::accumulate
#include <random>  // For random data generation
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/file/memory_mapped_file.hpp>

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

  // --- Benchmark Teardown ---
  std::cout << "Benchmark Info: Removing large file: " << benchmark_file_path
            << std::endl;
  std::filesystem::remove(benchmark_file_path);
}

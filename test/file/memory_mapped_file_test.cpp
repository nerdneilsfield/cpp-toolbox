#include <algorithm>  // For std::equal
#include <filesystem>
#include <fstream>
#include <iterator>  // For std::ostreambuf_iterator
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/file/memory_mapped_file.hpp>

// Helper function to create a test file with specific content
void create_test_file(const std::filesystem::path& path,
                      const std::string& content)
{
  std::ofstream ofs(path, std::ios::binary);
  ofs << content;
}

TEST_CASE("MemoryMappedFile Tests", "[file][memory_mapped]")
{
  toolbox::file::memory_mapped_file_t mapped_file;
  const std::filesystem::path non_existent_file = "non_existent_test_file.txt";
  const std::filesystem::path empty_file_path = "empty_test_file.txt";
  const std::filesystem::path valid_file_path = "valid_test_file.txt";
  const std::string test_content = "Hello, Memory Mapped World!";

  // Ensure files don't exist initially (cleanup from previous runs)
  std::filesystem::remove(non_existent_file);
  std::filesystem::remove(empty_file_path);
  std::filesystem::remove(valid_file_path);

  // Create necessary files for testing
  create_test_file(empty_file_path, "");
  create_test_file(valid_file_path, test_content);

  SECTION("Opening non-existent file")
  {
    REQUIRE_FALSE(mapped_file.open(non_existent_file));
    REQUIRE_FALSE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == 0);
    REQUIRE(mapped_file.data() == nullptr);
  }

  SECTION("Opening an empty file")
  {
    // The current implementation considers empty files as errors during size
    // check
    REQUIRE_FALSE(mapped_file.open(empty_file_path));
    REQUIRE_FALSE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == 0);  // Size remains 0
    REQUIRE(mapped_file.data() == nullptr);
  }

  SECTION("Opening a valid file")
  {
    REQUIRE(mapped_file.open(valid_file_path));
    REQUIRE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == test_content.length());
    REQUIRE(mapped_file.data() != nullptr);

    // Verify content
    const unsigned char* mapped_data = mapped_file.data();
    std::vector<unsigned char> content_vec(test_content.begin(),
                                           test_content.end());
    REQUIRE(std::equal(
        mapped_data, mapped_data + mapped_file.size(), content_vec.begin()));
  }

  SECTION("Closing a file")
  {
    REQUIRE(mapped_file.open(valid_file_path));
    REQUIRE(mapped_file.is_open());

    mapped_file.close();

    REQUIRE_FALSE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == 0);
    REQUIRE(mapped_file.data() == nullptr);

    // Closing again should be safe
    REQUIRE_NOTHROW(mapped_file.close());
  }

  SECTION("RAII behavior - automatic closing")
  {
    {
      toolbox::file::memory_mapped_file_t scoped_mmap;
      REQUIRE(scoped_mmap.open(valid_file_path));
      REQUIRE(scoped_mmap.is_open());
      // scoped_mmap goes out of scope here, destructor should call close()
    }
    // We cannot directly check if the file handle/mapping is closed without
    // platform-specific code or accessing private members.
    // We rely on the destructor calling close() properly, which is tested
    // implicitly in other sections (e.g., reopening works).
  }

  SECTION("Reopening a different file")
  {
    // Open the first file
    REQUIRE(mapped_file.open(valid_file_path));
    REQUIRE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == test_content.length());

    // Create a second file
    const std::filesystem::path another_file_path = "another_test_file.txt";
    const std::string another_content = "Another file content.";
    create_test_file(another_file_path, another_content);

    // Open the second file - should automatically close the first one
    REQUIRE(mapped_file.open(another_file_path));
    REQUIRE(mapped_file.is_open());
    REQUIRE(mapped_file.size() == another_content.length());

    // Verify content of the second file
    const unsigned char* mapped_data = mapped_file.data();
    std::vector<unsigned char> content_vec(another_content.begin(),
                                           another_content.end());
    REQUIRE(std::equal(
        mapped_data, mapped_data + mapped_file.size(), content_vec.begin()));

    // Explicitly close the file before removing it
    mapped_file.close();
    REQUIRE_FALSE(mapped_file.is_open()); // Optional check

    // Clean up second file
    std::filesystem::remove(another_file_path);
  }

  // Explicitly close the main mapped_file object before final cleanup
  mapped_file.close();

  // Cleanup test files
  std::filesystem::remove(empty_file_path);
  std::filesystem::remove(valid_file_path);
}

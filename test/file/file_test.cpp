#include <iostream>

#include "cpp-toolbox/file/file.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

TEST_CASE("get_current_working_directory", "[file]")
{
  auto dir = toolbox::file::get_current_working_directory();
  REQUIRE(!dir.empty());
}

TEST_CASE("get_current_directory", "[file]")
{
  auto dir = toolbox::file::get_current_directory();
  REQUIRE(!dir.empty());
}

TEST_CASE("traverse_directory", "[file]")
{
  auto dir = toolbox::file::traverse_directory(".");
  REQUIRE(!dir.empty());
}

TEST_CASE("recursive_traverse_directory", "[file]")
{
  auto dir = toolbox::file::recursive_traverse_directory(".");
  REQUIRE(!dir.empty());
}

TEST_CASE("traverse_directory_files", "[file]")
{
  std::function<void(const std::filesystem::path&)> callback =
      [](const std::filesystem::path& path) { std::cout << path << "\n"; };

  toolbox::file::traverse_directory_files(".", callback);
}

TEST_CASE("recursive_traverse_directory_files", "[file]")
{
  std::function<void(const std::filesystem::path&)> callback =
      [](const std::filesystem::path& path) { std::cout << path << "\n"; };

  toolbox::file::recursive_traverse_directory_files(".", callback);
}

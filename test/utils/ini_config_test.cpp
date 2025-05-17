#include <catch2/catch_test_macros.hpp>

#include <cpp-toolbox/utils/ini_config.hpp>
#include <filesystem>
#include "test_data_dir.hpp"

using namespace toolbox::utils;

TEST_CASE("INI config basic load", "[ini]")
{
  ini_config_t cfg;
  bool loaded = cfg.load(std::filesystem::path(test_data_dir) / "sample.ini");
  REQUIRE(loaded);
  REQUIRE(cfg.has("section1", "key1"));
  REQUIRE(cfg.get_string("section1", "key1") == "value1");
}

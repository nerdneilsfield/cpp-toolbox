#include <catch2/catch_test_macros.hpp>
#include <cpp-toolbox/utils/ini_config.hpp>
#include <cpp-toolbox/utils/ini_struct.hpp>
#include <filesystem>
#include "test_data_dir.hpp"

using namespace toolbox::utils;

struct demo_config
{
  std::string key1;
  int key2 = 0;
};

TOOLBOX_INI_STRUCT(demo_config,
    TOOLBOX_INI_FIELD(demo_config, key1, "section1", "key1"),
    TOOLBOX_INI_FIELD(demo_config, key2, "section1", "key2"))

TEST_CASE("INI struct reflection load", "[ini_struct]")
{
  ini_config_t cfg;
  REQUIRE(cfg.load(std::filesystem::path(test_data_dir) / "sample.ini"));
  demo_config dc{};
  REQUIRE(load_struct_from_ini(cfg, dc));
  REQUIRE(dc.key1 == "value1");
  REQUIRE(dc.key2 == 42);
}

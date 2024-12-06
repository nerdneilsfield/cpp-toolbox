#include "cpp-toolbox/base/env.hpp"

#include <catch2/catch_test_macros.hpp>
#define CATCH_CONFIG_MAIN

TEST_CASE("set_environment_variable")
{
  auto value =
      toolbox::base::set_environment_variable("TEST_ENV", "test_value");
  REQUIRE(value);

  auto value2 = toolbox::base::get_environment_variable("TEST_ENV");
  REQUIRE(value2 == "test_value");
}

TEST_CASE("remove_environment_variable")
{
  auto value = toolbox::base::remove_environment_variable("TEST_ENV");
  REQUIRE(value);

  auto value2 = toolbox::base::get_environment_variable("TEST_ENV");
  REQUIRE(value2.empty());
}

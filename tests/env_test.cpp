#include "cpp_toolbox.hpp"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in
#include "catch.hpp"

using namespace cpp_toolbox::env;

TEST_CASE("Environment variables can be set", "[environment]") {
  SECTION("Set and get environment variables") {
    SetEnv("TEST_ENV_VAR", "TEST_ENV_VAR_VALUE");
    REQUIRE(GetEnv("TEST_ENV_VAR") == "TEST_ENV_VAR_VALUE");
  }
}
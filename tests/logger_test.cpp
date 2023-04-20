#include "cpp_toolbox.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in
#include "catch.hpp"

using namespace cpp_toolbox;

TEST_CASE("TEST LOGGER"){
  logger::SetLoggerVerbosityLevel(logger::VerbosityLevel::Debug);
  LOG_INFO("THIS IS A INFO");
  LOG_WARNING("THIS IS A WARN");
  LOG_ERROR("THIS IS A ERROR");
  LOG_DEBUG("THIS IS A DEBUG");
}
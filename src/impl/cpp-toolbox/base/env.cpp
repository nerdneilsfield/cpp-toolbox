#include <cstdio>
#include <cstdlib>
#include <string>

#include "cpp-toolbox/macro.hpp"

#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
#  include <stdlib.h>
#else
#  include <unistd.h>
#endif

#include "cpp-toolbox/base/env.hpp"

namespace toolbox::base
{

/**
 * @brief Get an environment variable
 * @param name The name of the environment variable
 * @return The value of the environment variable
 *
 * @details This function is used to get an environment variable for the current
 * process. The environment variable is retrieved using the `std::getenv`
 * function.
 * @see https://en.cppreference.com/w/cpp/utility/program/getenv
 */
auto get_environment_variable(const std::string& name) -> std::string
{
  // Get the environment variable using std::getenv
  const char* value = std::getenv(name.c_str());

  // If the environment variable does not exist, return an empty string
  if (value == nullptr) {
    return {};
  }

  // Otherwise, return the string
  return value;
}

/**
 * @brief Set an environment variable
 * @param name The name of the environment variable
 * @param value The value of the environment variable
 * @return True if the environment variable was set, false otherwise
 *
 * @details This function is used to set an environment variable for the current
 * process. The environment variable is set using the `setenv` function.
 * @see https://en.cppreference.com/w/cpp/utility/program/setenv
 */
auto set_environment_variable(const std::string& name, const std::string& value)
    -> bool
{
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
  return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
  return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

/**
 * @brief Remove an environment variable
 * @param name The name of the environment variable
 * @return True if the environment variable was removed, false otherwise
 *
 * @details This function is used to remove an environment variable for the
 * current process. The environment variable is removed using the `unsetenv`
 * function.
 * @see https://en.cppreference.com/w/cpp/utility/program/unsetenv
 */
auto remove_environment_variable(const std::string& name) -> bool
{
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
  return _putenv_s(name.c_str(), "") == 0;
#else
  return unsetenv(name.c_str()) == 0;
#endif
}

}  // namespace toolbox::base

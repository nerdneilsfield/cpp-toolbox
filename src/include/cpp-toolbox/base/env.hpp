#pragma once

#include <string>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/macro.hpp"

/**
 * @namespace toolbox::base
 * @brief Base namespace for toolbox library
 */
namespace toolbox::base
{

/**
 * @brief Retrieves the value of an environment variable.
 *
 * This function queries the system environment variables and returns the value
 * associated with the specified name. If the environment variable does not
 * exist, an empty string is returned.
 *
 * @param name The name of the environment variable to retrieve. Must be
 * non-empty.
 * @return The value of the environment variable as a string. Returns an empty
 * string if the variable is not found.
 *
 * @code{.cpp}
 * // Get the PATH environment variable
 * auto path = get_environment_variable("PATH");
 * // path will contain the PATH environment variable value
 *
 * // Get a non-existent variable
 * auto value = get_environment_variable("NON_EXISTENT_VAR");
 * // value will be an empty string
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_environment_variable(const std::string& name)
    -> std::string;

/**
 * @brief Sets or updates an environment variable.
 *
 * This function creates or modifies an environment variable with the specified
 * name and value. The change affects the current process and any child
 * processes spawned after the change.
 *
 * @param name The name of the environment variable to set. Must be non-empty.
 * @param value The value to assign to the environment variable.
 * @return True if the environment variable was successfully set, false
 * otherwise. Failure can occur due to invalid input or system limitations.
 *
 * @code{.cpp}
 * // Set a new environment variable
 * bool success = set_environment_variable("MY_VAR", "12345");
 * // success will be true if the variable was set
 *
 * // Update an existing variable
 * set_environment_variable("PATH", "/new/path:" +
 * get_environment_variable("PATH"));
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto set_environment_variable(const std::string& name,
                                                 const std::string& value)
    -> bool;

/**
 * @brief Removes an environment variable from the current process environment.
 *
 * This function deletes the specified environment variable from the current
 * process's environment. The change affects the current process and any child
 * processes spawned after the change.
 *
 * @param name The name of the environment variable to remove. Must be
 * non-empty.
 * @return True if the environment variable was successfully removed, false
 * otherwise. Returns false if the variable did not exist or could not be
 * removed.
 *
 * @code{.cpp}
 * // Remove a temporary environment variable
 * bool removed = remove_environment_variable("TEMP_VAR");
 * // removed will be true if the variable existed and was removed
 *
 * // Attempt to remove a non-existent variable
 * bool result = remove_environment_variable("NON_EXISTENT_VAR");
 * // result will be false
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove_environment_variable(const std::string& name)
    -> bool;

}  // namespace toolbox::base

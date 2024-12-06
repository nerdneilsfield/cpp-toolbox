#include <string>

#include "cpp-toolbox/macro.hpp"

namespace toolbox::base
{

/**
 * @brief Get an environment variable
 * @param name The name of the environment variable
 * @return The value of the environment variable
 */
CPP_TOOLBOX_EXPORT auto get_environment_variable(const std::string& name)
    -> std::string;

/**
 * @brief Set an environment variable
 * @param name The name of the environment variable
 * @param value The value of the environment variable
 * @return True if the environment variable was set, false otherwise
 */
CPP_TOOLBOX_EXPORT auto set_environment_variable(const std::string& name,
                                                 const std::string& value)
    -> bool;

/**
 * @brief Remove an environment variable
 * @param name The name of the environment variable
 * @return True if the environment variable was removed, false otherwise
 */
CPP_TOOLBOX_EXPORT auto remove_environment_variable(const std::string& name)
    -> bool;

}  // namespace toolbox::base

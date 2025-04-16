#pragma once

#include <string>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/macro.hpp"

/**
 * @namespace toolbox::base
 * @brief 工具箱库的基础命名空间 (Base namespace for toolbox library)
 */
namespace toolbox::base
{

/**
 * @brief 获取环境变量的值 (Retrieves the value of an environment variable)
 *
 * 该函数查询系统环境变量并返回与指定名称关联的值。如果环境变量不存在，则返回空字符串。
 * (This function queries the system environment variables and returns the value
 * associated with the specified name. If the environment variable does not
 * exist, an empty string is returned.)
 *
 * @param name 要获取的环境变量名称，不能为空 (The name of the environment
 * variable to retrieve. Must be non-empty)
 * @return 环境变量的字符串值。如果变量未找到则返回空字符串 (The value of the
 * environment variable as a string. Returns an empty string if not found)
 *
 * @code{.cpp}
 * // 获取 PATH 环境变量 (Get the PATH environment variable)
 * auto path = get_environment_variable("PATH");
 * // path 将包含 PATH 环境变量的值 (path will contain the PATH value)
 *
 * // 获取不存在的变量 (Get a non-existent variable)
 * auto value = get_environment_variable("NON_EXISTENT_VAR");
 * // value 将是空字符串 (value will be empty)
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto get_environment_variable(const std::string& name)
    -> std::string;

/**
 * @brief 设置或更新环境变量 (Sets or updates an environment variable)
 *
 * 该函数创建或修改具有指定名称和值的环境变量。更改会影响当前进程及其后创建的任何子进程。
 * (This function creates or modifies an environment variable with the specified
 * name and value. The change affects the current process and any child
 * processes spawned after the change.)
 *
 * @param name 要设置的环境变量名称，不能为空 (The name of the environment
 * variable to set. Must be non-empty)
 * @param value 要分配给环境变量的值 (The value to assign to the environment
 * variable)
 * @return 如果环境变量设置成功则返回true，否则返回false (True if successfully
 * set, false otherwise)
 *
 * @code{.cpp}
 * // 设置新的环境变量 (Set a new environment variable)
 * bool success = set_environment_variable("MY_VAR", "12345");
 * // success 如果变量设置成功将为true (success will be true if set)
 *
 * // 更新现有变量 (Update existing variable)
 * set_environment_variable("PATH", "/new/path:" +
 *                         get_environment_variable("PATH"));
 *
 * // 设置临时变量 (Set temporary variable)
 * set_environment_variable("TEMP_DIR", "/tmp/workdir");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto set_environment_variable(const std::string& name,
                                                 const std::string& value)
    -> bool;

/**
 * @brief 从当前进程环境中删除环境变量 (Removes an environment variable from the
 * current process environment)
 *
 * 该函数从当前进程的环境中删除指定的环境变量。更改会影响当前进程及其后创建的任何子进程。
 * (This function deletes the specified environment variable from the current
 * process's environment. The change affects the current process and any child
 * processes spawned after the change.)
 *
 * @param name 要删除的环境变量名称，不能为空 (The name of the environment
 * variable to remove. Must be non-empty)
 * @return 如果环境变量成功删除则返回true，否则返回false (True if successfully
 * removed, false otherwise)
 *
 * @code{.cpp}
 * // 删除临时环境变量 (Remove a temporary environment variable)
 * bool removed = remove_environment_variable("TEMP_VAR");
 * // removed 如果变量存在并被删除将为true (removed will be true if existed and
 * removed)
 *
 * // 尝试删除不存在的变量 (Try to remove non-existent variable)
 * bool result = remove_environment_variable("NON_EXISTENT_VAR");
 * // result 将为false (result will be false)
 *
 * // 清理会话变量 (Clean up session variables)
 * remove_environment_variable("SESSION_ID");
 * remove_environment_variable("USER_TEMP");
 * @endcode
 */
CPP_TOOLBOX_EXPORT auto remove_environment_variable(const std::string& name)
    -> bool;

}  // namespace toolbox::base

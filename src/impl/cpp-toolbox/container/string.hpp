/**
 * @brief Replaces occurrences of a substring within a string view with another substring, up to a specified count.
 *
 * @param s The original string view.
 * @param old_value The substring to be replaced.
 * @param new_value The substring to replace with.
 * @param count The maximum number of replacements to perform. Defaults to replacing all occurrences.
 * @return A new string with the replacements made.
 * 
 * @note Special case for empty old_value:
 *       When old_value is empty, the function inserts new_value between characters and at the start of the string.
 *       For example, replace("hello", "", "-", 3) returns "-h-e-llo".
 *       The count parameter determines how many insertions to make, including the one at the start.
 */
CPP_TOOLBOX_EXPORT auto replace(std::string_view s,
             std::string_view old_value,
             std::string_view new_value,
             std::size_t count = std::numeric_limits<std::size_t>::max())
    -> std::string; 
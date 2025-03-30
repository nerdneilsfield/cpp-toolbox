#pragma once

#ifndef CPP_TOOLBOX_CONTAINER_STRING_HPP
#define CPP_TOOLBOX_CONTAINER_STRING_HPP

#include <string>
#include <string_view>
#include <vector>
#include <limits>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::container::string
{
/**
 * @brief Splits a string view into a vector of strings based on a string delimiter.
 *
 * @param str The string view to split.
 * @param delimiter The string view to use as a delimiter.
 * @return A vector of strings resulting from the split. Empty parts are included.
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, std::string_view delimiter)
    -> std::vector<std::string>;

/**
 * @brief Splits a string view into a vector of strings based on a character delimiter.
 *
 * @param str The string view to split.
 * @param delimiter The character to use as a delimiter.
 * @return A vector of strings resulting from the split. Empty parts are included.
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, char delimiter) -> std::vector<std::string>;

/**
 * @brief Joins a vector of strings into a single string, separated by a glue string.
 *
 * @param parts The vector of strings to join.
 * @param glue The string view to insert between elements.
 * @return The resulting joined string.
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string>& parts, std::string_view glue)
    -> std::string;

/**
 * @brief Joins a vector of string views into a single string, separated by a glue string.
 *
 * @param parts The vector of string views to join.
 * @param glue The string view to insert between elements.
 * @return The resulting joined string.
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string_view>& parts, std::string_view glue)
    -> std::string;

/**
 * @brief Removes leading whitespace characters from a string view.
 *
 * Whitespace characters are determined by `std::isspace`.
 *
 * @param str The input string view.
 * @return A new string with leading whitespace removed.
 */
CPP_TOOLBOX_EXPORT auto trim_left(std::string_view str) -> std::string;

/**
 * @brief Removes trailing whitespace characters from a string view.
 *
 * Whitespace characters are determined by `std::isspace`.
 *
 * @param str The input string view.
 * @return A new string with trailing whitespace removed.
 */
CPP_TOOLBOX_EXPORT auto trim_right(std::string_view str) -> std::string;

/**
 * @brief Removes both leading and trailing whitespace characters from a string view.
 *
 * Whitespace characters are determined by `std::isspace`.
 *
 * @param str The input string view.
 * @return A new string with leading and trailing whitespace removed.
 */
CPP_TOOLBOX_EXPORT auto trim(std::string_view str) -> std::string;

/**
 * @brief Checks if a string view starts with a specified prefix.
 *
 * @param s The string view to check.
 * @param prefix The prefix string view to look for.
 * @return True if the string starts with the prefix, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto starts_with(std::string_view s, std::string_view prefix) -> bool;

/**
 * @brief Checks if a string view ends with a specified suffix.
 *
 * @param s The string view to check.
 * @param suffix The suffix string view to look for.
 * @return True if the string ends with the suffix, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto ends_with(std::string_view s, std::string_view suffix) -> bool;

/**
 * @brief Checks if a string view contains a specified substring.
 *
 * @param s The string view to search within.
 * @param substring The substring to search for.
 * @return True if the substring is found within the string, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto contains(std::string_view s, std::string_view substring) -> bool;

/**
 * @brief Checks if a string view is empty or consists only of whitespace characters.
 *
 * Whitespace characters are determined by `std::isspace`.
 *
 * @param s The string view to check.
 * @return True if the string is empty or contains only whitespace, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto is_empty_or_whitespace(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents a numeric value (integer or floating-point).
 *
 * This check might be basic and may not cover all edge cases like locale-specific formats.
 * It typically checks for digits, an optional sign, and an optional decimal point.
 *
 * @param s The string view to check.
 * @return True if the string appears to be numeric, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto is_numeric(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents an integer value.
 *
 * It typically checks for digits and an optional leading sign.
 *
 * @param s The string view to check.
 * @return True if the string appears to be an integer, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto is_integer(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents a floating-point value.
 *
 * It typically checks for digits, an optional leading sign, and an optional decimal point.
 *
 * @param s The string view to check.
 * @return True if the string appears to be a floating-point number, false otherwise.
 */
CPP_TOOLBOX_EXPORT auto is_float(std::string_view s) -> bool;

/**
 * @brief Replaces occurrences of a substring within a string view with another substring, up to a specified count.
 *
 * @param s The original string view.
 * @param old_value The substring to be replaced.
 * @param new_value The substring to replace with.
 * @param count The maximum number of replacements to perform. Defaults to replacing all occurrences.
 * @return A new string with the replacements made.
 */
CPP_TOOLBOX_EXPORT auto replace(std::string_view s,
             std::string_view old_value,
             std::string_view new_value,
             std::size_t count = std::numeric_limits<std::size_t>::max())
    -> std::string;

/**
 * @brief Replaces all occurrences of a substring within a string view with another substring.
 *
 * This is a convenience function equivalent to calling `replace` with the default count.
 *
 * @param s The original string view.
 * @param old_value The substring to be replaced.
 * @param new_value The substring to replace with.
 * @return A new string with all occurrences replaced.
 */
CPP_TOOLBOX_EXPORT auto replace_all(std::string_view s,
                 std::string_view old_value,
                 std::string_view new_value) -> std::string;

/**
* @brief Replaces the Nth occurrence of a substring within a string.
* @param s The original string view.
* @param old_value The substring to find. Cannot be empty.
* @param new_value The substring to replace with.
* @param n The 1-based index of the occurrence to replace (1 for the first, 2 for the second, etc.). Must be > 0.
* @return A new std::string with the Nth occurrence replaced, or a copy of the original string
* if n is invalid, 'from' is empty, or the Nth occurrence is not found.
*/
CPP_TOOLBOX_EXPORT auto replace_by_nth(std::string_view s,
                    std::string_view old_value,
                    std::string_view new_value,
                    std::size_t n) -> std::string;

/**
* @brief Removes the Nth occurrence of a substring within a string.
* @param s The original string view.
* @param from The substring to find and remove. Cannot be empty.
* @param n The 1-based index of the occurrence to remove (1 for the first, 2 for the second, etc.). Must be > 0.
* @return A new std::string with the Nth occurrence removed, or a copy of the original string
* if n is invalid, 'from' is empty, or the Nth occurrence is not found.
*/
CPP_TOOLBOX_EXPORT auto remove_nth(std::string_view s,
                std::string_view from,
                std::size_t n) -> std::string;


/**
 * @brief Removes occurrences of a substring from a string view, up to a specified count.
 *
 * This is equivalent to replacing the substring with an empty string.
 *
 * @param s The original string view.
 * @param value The substring to remove.
 * @param count The maximum number of occurrences to remove. Defaults to removing all occurrences.
 * @return A new string with the specified substring removed.
 */
CPP_TOOLBOX_EXPORT auto remove(std::string_view s,
            std::string_view value,
            std::size_t count = std::numeric_limits<std::size_t>::max())
    -> std::string;

/**
 * @brief Removes all occurrences of a substring from a string view.
 *
 * This is a convenience function equivalent to calling `remove` with the default count.
 *
 * @param s The original string view.
 * @param value The substring to remove.
 * @return A new string with all occurrences of the substring removed.
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, std::string_view value) -> std::string;

/**
 * @brief Removes all occurrences of a specific character from a string view.
 *
 * @param s The original string view.
 * @param value The character to remove.
 * @return A new string with all occurrences of the character removed.
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, char value) -> std::string;

/**
 * @brief Converts all characters in a string view to lowercase.
 *
 * Locale-dependent behavior might apply depending on the implementation.
 *
 * @param s The input string view.
 * @return A new string with all characters converted to lowercase.
 */
CPP_TOOLBOX_EXPORT auto to_lower(std::string_view s) -> std::string;

/**
 * @brief Converts all characters in a string view to uppercase.
 *
 * Locale-dependent behavior might apply depending on the implementation.
 *
 * @param s The input string view.
 * @return A new string with all characters converted to uppercase.
 */
CPP_TOOLBOX_EXPORT auto to_upper(std::string_view s) -> std::string;

/**
 * @brief Converts a string view to a standard string.
 *
 * Useful for obtaining an owned string copy from a view.
 *
 * @param s The input string view.
 * @return A std::string object containing the same character sequence.
 */
CPP_TOOLBOX_EXPORT auto to_string(std::string_view s) -> std::string;

/**
 * @brief Pads a string on the left with a specified character to reach a minimum width.
 *
 * If the input string's length is already greater than or equal to the target width,
 * a copy of the original string is returned.
 *
 * @param s The input string view.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @return A new string, padded on the left if necessary.
 */
CPP_TOOLBOX_EXPORT auto left_pad(std::string_view s, std::size_t width, char pad_char = ' ')
    -> std::string;

/**
 * @brief Pads a string on the right with a specified character to reach a minimum width.
 *
 * If the input string's length is already greater than or equal to the target width,
 * a copy of the original string is returned.
 *
 * @param s The input string view.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @return A new string, padded on the right if necessary.
 */
CPP_TOOLBOX_EXPORT auto right_pad(std::string_view s, std::size_t width, char pad_char = ' ')
    -> std::string;

/**
 * @brief Pads a string with a specified character to reach a minimum width, controlling padding position.
 *
 * If the input string's length is already greater than or equal to the target width,
 * a copy of the original string is returned. The `position` parameter's exact behavior
 * (e.g., 0 for left, 1 for right, other for center) depends on the implementation.
 * A default of 0 typically implies left padding.
 *
 * @param s The input string view.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @param position A value indicating the padding position (e.g., 0 for left). Defaults to 0.
 * @return A new string, padded according to the specified parameters.
 */
CPP_TOOLBOX_EXPORT auto pad(std::string_view s,
         std::size_t width,
         char pad_char = ' ',
         std::size_t position = 0) -> std::string;

/**
 * @brief Reverses the order of characters in a string view.
 *
 * @param s The input string view.
 * @return A new string with the characters in reverse order.
 */
CPP_TOOLBOX_EXPORT auto reverse(std::string_view s) -> std::string;

/**
 * @brief Attempts to parse an integer from a string view.
 *
 * Uses standard C++ parsing mechanisms (like `std::from_chars` or `std::stoi`).
 * Checks for potential overflow and invalid formats.
 *
 * @param s The string view to parse.
 * @param out Reference to an integer where the parsed value will be stored if successful.
 * @return True if parsing was successful, false otherwise. `out` is only modified on success.
 */
CPP_TOOLBOX_EXPORT auto try_parse_int(std::string_view s, int& out) -> bool;

/**
 * @brief Attempts to parse a double-precision floating-point number from a string view.
 *
 * Uses standard C++ parsing mechanisms (like `std::from_chars` or `std::stod`).
 * Checks for potential overflow/underflow and invalid formats.
 *
 * @param s The string view to parse.
 * @param out Reference to a double where the parsed value will be stored if successful.
 * @return True if parsing was successful, false otherwise. `out` is only modified on success.
 */
CPP_TOOLBOX_EXPORT auto try_parse_double(std::string_view s, double& out) -> bool;

/**
 * @brief Attempts to parse a single-precision floating-point number from a string view.
 *
 * Uses standard C++ parsing mechanisms (like `std::from_chars` or `std::stof`).
 * Checks for potential overflow/underflow and invalid formats.
 *
 * @param s The string view to parse.
 * @param out Reference to a float where the parsed value will be stored if successful.
 * @return True if parsing was successful, false otherwise. `out` is only modified on success.
 */
CPP_TOOLBOX_EXPORT auto try_parse_float(std::string_view s, float& out) -> bool;

}  // namespace toolbox::container::string
#endif  // CPP_TOOLBOX_CONTAINER_STRING_HPP

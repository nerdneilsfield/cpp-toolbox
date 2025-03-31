#pragma once

#ifndef CPP_TOOLBOX_CONTAINER_STRING_HPP
#  define CPP_TOOLBOX_CONTAINER_STRING_HPP

#  include <limits>
#  include <string>
#  include <string_view>
#  include <vector>

#  include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::container::string
{
/**
 * @brief Splits a string view into a vector of strings based on a string
 * delimiter.
 *
 * Splits the input string at each occurrence of the delimiter string. Empty
 * parts between delimiters are included in the result. The delimiter itself is
 * not included in the output.
 *
 * @param str The string view to split.
 * @param delimiter The string view to use as a delimiter.
 * @return std::vector<std::string> A vector of strings resulting from the
 * split.
 *
 * @example
 * // Split by string delimiter
 * auto result = split("apple,orange,,banana", ",");
 * // result will be {"apple", "orange", "", "banana"}
 *
 * @example
 * // Split with multi-character delimiter
 * auto result2 = split("one->two->three", "->");
 * // result2 will be {"one", "two", "three"}
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, std::string_view delimiter)
    -> std::vector<std::string>;

/**
 * @brief Splits a string view into a vector of strings based on a character
 * delimiter.
 *
 * Splits the input string at each occurrence of the delimiter character. Empty
 * parts between delimiters are included in the result. The delimiter character
 * itself is not included in the output.
 *
 * @param str The string view to split.
 * @param delimiter The character to use as a delimiter.
 * @return std::vector<std::string> A vector of strings resulting from the
 * split.
 *
 * @example
 * // Split by character delimiter
 * auto result = split("one|two||three", '|');
 * // result will be {"one", "two", "", "three"}
 *
 * @example
 * // Split with space delimiter
 * auto result2 = split("hello world  cpp", ' ');
 * // result2 will be {"hello", "world", "", "cpp"}
 */
CPP_TOOLBOX_EXPORT auto split(std::string_view str, char delimiter)
    -> std::vector<std::string>;

/**
 * @brief Joins a vector of strings into a single string, separated by a glue
 * string.
 *
 * Concatenates all strings in the vector, inserting the glue string between
 * each element. If the input vector is empty, returns an empty string.
 *
 * @param parts The vector of strings to join.
 * @param glue The string view to insert between elements.
 * @return std::string The resulting joined string.
 *
 * @example
 * // Join with comma separator
 * auto result = join({"apple", "banana", "cherry"}, ", ");
 * // result will be "apple, banana, cherry"
 *
 * @example
 * // Join with empty glue
 * auto result2 = join({"a", "b", "c"}, "");
 * // result2 will be "abc"
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string>& parts,
                             std::string_view glue) -> std::string;

/**
 * @brief Joins a vector of string views into a single string, separated by a
 * glue string.
 *
 * Concatenates all string views in the vector, inserting the glue string
 * between each element. If the input vector is empty, returns an empty string.
 *
 * @param parts The vector of string views to join.
 * @param glue The string view to insert between elements.
 * @return std::string The resulting joined string.
 *
 * @example
 * // Join string views with hyphen
 * auto result = join({"2023", "12", "31"}, "-");
 * // result will be "2023-12-31"
 *
 * @example
 * // Join with newline separator
 * auto result2 = join({"line1", "line2", "line3"}, "\n");
 * // result2 will be "line1\nline2\nline3"
 */
CPP_TOOLBOX_EXPORT auto join(const std::vector<std::string_view>& parts,
                             std::string_view glue) -> std::string;

/**
 * @brief Removes leading whitespace characters from a string view.
 *
 * Creates a new string with all leading whitespace characters removed.
 * Whitespace characters are determined by `std::isspace` (spaces, tabs,
 * newlines, etc.).
 *
 * @param str The input string view.
 * @return std::string A new string with leading whitespace removed.
 *
 * @example
 * // Trim leading spaces
 * auto result = trim_left("   hello");
 * // result will be "hello"
 *
 * @example
 * // Trim mixed leading whitespace
 * auto result2 = trim_left("\t\n  world");
 * // result2 will be "world"
 */
CPP_TOOLBOX_EXPORT auto trim_left(std::string_view str) -> std::string;

/**
 * @brief Removes trailing whitespace characters from a string view.
 *
 * Creates a new string with all trailing whitespace characters removed.
 * Whitespace characters are determined by `std::isspace` (spaces, tabs,
 * newlines, etc.).
 *
 * @param str The input string view.
 * @return std::string A new string with trailing whitespace removed.
 *
 * @example
 * // Trim trailing spaces
 * auto result = trim_right("hello   ");
 * // result will be "hello"
 *
 * @example
 * // Trim mixed trailing whitespace
 * auto result2 = trim_right("world\t\n  ");
 * // result2 will be "world"
 */
CPP_TOOLBOX_EXPORT auto trim_right(std::string_view str) -> std::string;

/**
 * @brief Removes both leading and trailing whitespace characters from a string
 * view.
 *
 * Creates a new string with all leading and trailing whitespace characters
 * removed. Whitespace characters are determined by `std::isspace`.
 *
 * @param str The input string view.
 * @return std::string A new string with leading and trailing whitespace
 * removed.
 *
 * @example
 * // Trim both ends
 * auto result = trim("   hello world   ");
 * // result will be "hello world"
 *
 * @example
 * // Trim mixed whitespace
 * auto result2 = trim("\t\n  test string \n\t");
 * // result2 will be "test string"
 */
CPP_TOOLBOX_EXPORT auto trim(std::string_view str) -> std::string;

/**
 * @brief Checks if a string view starts with a specified prefix.
 *
 * Performs a case-sensitive comparison to check if the string begins with the
 * given prefix. Both empty strings and empty prefixes are handled.
 *
 * @param s The string view to check.
 * @param prefix The prefix string view to look for.
 * @return bool True if the string starts with the prefix, false otherwise.
 *
 * @example
 * // Check prefix
 * bool result = starts_with("hello world", "hello");
 * // result will be true
 *
 * @example
 * // Check non-matching prefix
 * bool result2 = starts_with("hello world", "world");
 * // result2 will be false
 */
CPP_TOOLBOX_EXPORT auto starts_with(std::string_view s, std::string_view prefix)
    -> bool;

/**
 * @brief Checks if a string view ends with a specified suffix.
 *
 * Performs a case-sensitive comparison to check if the string ends with the
 * given suffix. Both empty strings and empty suffixes are handled.
 *
 * @param s The string view to check.
 * @param suffix The suffix string view to look for.
 * @return bool True if the string ends with the suffix, false otherwise.
 *
 * @example
 * // Check suffix
 * bool result = ends_with("hello world", "world");
 * // result will be true
 *
 * @example
 * // Check non-matching suffix
 * bool result2 = ends_with("hello world", "hello");
 * // result2 will be false
 */
CPP_TOOLBOX_EXPORT auto ends_with(std::string_view s, std::string_view suffix)
    -> bool;

/**
 * @brief Checks if a string view contains a specified substring.
 *
 * Performs a case-sensitive search for the substring within the string.
 * Both empty strings and empty substrings are handled.
 *
 * @param s The string view to search within.
 * @param substring The substring to search for.
 * @return bool True if the substring is found within the string, false
 * otherwise.
 *
 * @example
 * // Check for substring
 * bool result = contains("hello world", "lo wo");
 * // result will be true
 *
 * @example
 * // Check for non-existent substring
 * bool result2 = contains("hello world", "goodbye");
 * // result2 will be false
 */
CPP_TOOLBOX_EXPORT auto contains(std::string_view s, std::string_view substring)
    -> bool;

/**
 * @brief Checks if a string view is empty or consists only of whitespace
 * characters.
 *
 * Determines if the string is either empty or contains only characters that
 * are considered whitespace by `std::isspace`.
 *
 * @param s The string view to check.
 * @return bool True if the string is empty or contains only whitespace, false
 * otherwise.
 *
 * @example
 * // Check empty string
 * bool result = is_empty_or_whitespace("");
 * // result will be true
 *
 * @example
 * // Check whitespace string
 * bool result2 = is_empty_or_whitespace(" \t\n");
 * // result2 will be true
 *
 * @example
 * // Check non-whitespace string
 * bool result3 = is_empty_or_whitespace("hello");
 * // result3 will be false
 */
CPP_TOOLBOX_EXPORT auto is_empty_or_whitespace(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents a numeric value (integer or
 * floating-point).
 *
 * Performs a basic check for numeric format, including optional sign, digits,
 * and optional decimal point. Does not handle locale-specific formats or
 * scientific notation.
 *
 * @param s The string view to check.
 * @return bool True if the string appears to be numeric, false otherwise.
 *
 * @example
 * // Check integer
 * bool result = is_numeric("12345");
 * // result will be true
 *
 * @example
 * // Check floating-point
 * bool result2 = is_numeric("-123.45");
 * // result2 will be true
 *
 * @example
 * // Check non-numeric
 * bool result3 = is_numeric("123abc");
 * // result3 will be false
 */
CPP_TOOLBOX_EXPORT auto is_numeric(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents a valid integer value.
 *
 * This function verifies if the input string consists of optional leading sign
 * (+/-) followed by one or more digits. It does not handle locale-specific
 * formats or scientific notation.
 *
 * @param s The string view to check.
 * @return True if the string is a valid integer, false otherwise.
 *
 * @example
 * // Check positive integer
 * bool result1 = is_integer("12345");
 * // result1 will be true
 *
 * @example
 * // Check negative integer
 * bool result2 = is_integer("-42");
 * // result2 will be true
 *
 * @example
 * // Check invalid integer
 * bool result3 = is_integer("12.34");
 * // result3 will be false
 */
CPP_TOOLBOX_EXPORT auto is_integer(std::string_view s) -> bool;

/**
 * @brief Checks if a string view represents a valid floating-point value.
 *
 * This function verifies if the input string consists of optional leading sign
 * (+/-), digits, and optional decimal point. It does not handle locale-specific
 * formats, scientific notation, or special floating-point values (NaN,
 * infinity).
 *
 * @param s The string view to check.
 * @return True if the string is a valid floating-point number, false otherwise.
 *
 * @example
 * // Check positive float
 * bool result1 = is_float("123.45");
 * // result1 will be true
 *
 * @example
 * // Check negative float
 * bool result2 = is_float("-0.123");
 * // result2 will be true
 *
 * @example
 * // Check invalid float
 * bool result3 = is_float("12.34.56");
 * // result3 will be false
 */
CPP_TOOLBOX_EXPORT auto is_float(std::string_view s) -> bool;

/**
 * @brief Replaces occurrences of a substring within a string view with another
 * substring, up to a specified count.
 *
 * This function performs up to 'count' replacements of 'old_value' with
 * 'new_value' in the input string. If 'old_value' is empty, it inserts
 * 'new_value' between characters and at the start of the string.
 *
 * @param s The original string view.
 * @param old_value The substring to be replaced.
 * @param new_value The substring to replace with.
 * @param count The maximum number of replacements to perform. Defaults to
 * replacing all occurrences.
 * @return A new string with the replacements made.
 *
 * @note When old_value is empty, the function inserts new_value between
 * characters. For example, replace("hello", "", "-", 3) returns "-h-e-llo".
 *
 * @example
 * // Basic replacement
 * auto result1 = replace("hello world", "world", "cpp");
 * // result1 will be "hello cpp"
 *
 * @example
 * // Limited replacements
 * auto result2 = replace("banana", "na", "no", 2);
 * // result2 will be "bonona"
 *
 * @example
 * // Empty old_value case
 * auto result3 = replace("abc", "", "-", 2);
 * // result3 will be "-a-bc"
 */
CPP_TOOLBOX_EXPORT auto replace(
    std::string_view s,
    std::string_view old_value,
    std::string_view new_value,
    std::size_t count = std::numeric_limits<std::size_t>::max()) -> std::string;

/**
 * @brief Replaces all occurrences of a substring within a string view with
 * another substring.
 *
 * This function replaces all instances of 'old_value' with 'new_value' in the
 * input string. It is equivalent to calling `replace` with the default count.
 *
 * @param s The original string view.
 * @param old_value The substring to be replaced.
 * @param new_value The substring to replace with.
 * @return A new string with all occurrences replaced.
 *
 * @example
 * // Replace all occurrences
 * auto result = replace_all("banana", "na", "no");
 * // result will be "bonono"
 *
 * @example
 * // No match case
 * auto result2 = replace_all("hello", "x", "y");
 * // result2 will be "hello"
 */
CPP_TOOLBOX_EXPORT auto replace_all(std::string_view s,
                                    std::string_view old_value,
                                    std::string_view new_value) -> std::string;

/**
 * @brief Replaces the Nth occurrence of a substring within a string.
 *
 * This function replaces only the specified occurrence (1-based index) of
 * 'old_value' with 'new_value'. If the specified occurrence is not found or
 * 'old_value' is empty, returns a copy of the original string.
 *
 * @param s The original string view.
 * @param old_value The substring to find. Cannot be empty.
 * @param new_value The substring to replace with.
 * @param n The 1-based index of the occurrence to replace (1 for the first, 2
 * for the second, etc.). Must be > 0.
 * @return A new std::string with the Nth occurrence replaced, or a copy of the
 * original string if n is invalid, 'from' is empty, or the Nth occurrence is
 * not found.
 *
 * @example
 * // Replace second occurrence
 * auto result1 = replace_by_nth("banana", "na", "no", 2);
 * // result1 will be "banona"
 *
 * @example
 * // Occurrence not found
 * auto result2 = replace_by_nth("hello", "l", "x", 3);
 * // result2 will be "hello"
 */
CPP_TOOLBOX_EXPORT auto replace_by_nth(std::string_view s,
                                       std::string_view old_value,
                                       std::string_view new_value,
                                       std::size_t n) -> std::string;

/**
 * @brief Removes the Nth occurrence of a substring within a string.
 *
 * This function removes only the specified occurrence (1-based index) of
 * 'from' in the input string. If the specified occurrence is not found or
 * 'from' is empty, returns a copy of the original string.
 *
 * @param s The original string view.
 * @param from The substring to find and remove. Cannot be empty.
 * @param n The 1-based index of the occurrence to remove (1 for the first, 2
 * for the second, etc.). Must be > 0.
 * @return A new std::string with the Nth occurrence removed, or a copy of the
 * original string if n is invalid, 'from' is empty, or the Nth occurrence is
 * not found.
 *
 * @example
 * // Remove second occurrence
 * auto result1 = remove_nth("banana", "na", 2);
 * // result1 will be "bana"
 *
 * @example
 * // Occurrence not found
 * auto result2 = remove_nth("hello", "l", 3);
 * // result2 will be "hello"
 */
CPP_TOOLBOX_EXPORT auto remove_nth(std::string_view s,
                                   std::string_view from,
                                   std::size_t n) -> std::string;

/**
 * @brief Removes occurrences of a substring from a string view, up to a
 * specified count.
 *
 * This function removes up to 'count' occurrences of 'value' from the input
 * string. It is equivalent to replacing 'value' with an empty string.
 *
 * @param s The original string view.
 * @param value The substring to remove.
 * @param count The maximum number of occurrences to remove. Defaults to
 * removing all occurrences.
 * @return A new string with the specified substring removed.
 *
 * @example
 * // Remove first two occurrences
 * auto result1 = remove("banana", "na", 2);
 * // result1 will be "ba"
 *
 * @example
 * // Remove all occurrences
 * auto result2 = remove("hello world", "l");
 * // result2 will be "heo word"
 */
CPP_TOOLBOX_EXPORT auto remove(
    std::string_view s,
    std::string_view value,
    std::size_t count = std::numeric_limits<std::size_t>::max()) -> std::string;

/**
 * @brief Removes all occurrences of a substring from a string view.
 *
 * This function removes every instance of the specified substring from the
 * input string. It is equivalent to calling `remove` with the default count
 * (removing all occurrences).
 *
 * @param s The original string view to process.
 * @param value The substring to remove. Cannot be empty.
 * @return A new std::string with all occurrences of the substring removed.
 *
 * @example
 * // Remove all occurrences of a substring
 * auto result = remove_all("banana", "na");
 * // result will be "ba"
 *
 * @example
 * // Remove all occurrences of a multi-character substring
 * auto result2 = remove_all("hello world hello", "hello");
 * // result2 will be " world "
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, std::string_view value)
    -> std::string;

/**
 * @brief Removes all occurrences of a specific character from a string view.
 *
 * This function removes every instance of the specified character from the
 * input string.
 *
 * @param s The original string view to process.
 * @param value The character to remove.
 * @return A new std::string with all occurrences of the character removed.
 *
 * @example
 * // Remove all occurrences of a character
 * auto result = remove_all("hello world", 'l');
 * // result will be "heo word"
 *
 * @example
 * // Remove all spaces
 * auto result2 = remove_all("text with spaces", ' ');
 * // result2 will be "textwithspaces"
 */
CPP_TOOLBOX_EXPORT auto remove_all(std::string_view s, char value)
    -> std::string;

/**
 * @brief Converts all characters in a string view to lowercase.
 *
 * This function creates a new string where all characters are converted to
 * their lowercase equivalents. The conversion is locale-dependent.
 *
 * @param s The input string view to convert.
 * @return A new std::string with all characters converted to lowercase.
 *
 * @example
 * // Convert to lowercase
 * auto result = to_lower("Hello World");
 * // result will be "hello world"
 *
 * @example
 * // Convert mixed case string
 * auto result2 = to_lower("CamelCaseString");
 * // result2 will be "camelcasestring"
 */
CPP_TOOLBOX_EXPORT auto to_lower(std::string_view s) -> std::string;

/**
 * @brief Converts all characters in a string view to uppercase.
 *
 * This function creates a new string where all characters are converted to
 * their uppercase equivalents. The conversion is locale-dependent.
 *
 * @param s The input string view to convert.
 * @return A new std::string with all characters converted to uppercase.
 *
 * @example
 * // Convert to uppercase
 * auto result = to_upper("Hello World");
 * // result will be "HELLO WORLD"
 *
 * @example
 * // Convert mixed case string
 * auto result2 = to_upper("CamelCaseString");
 * // result2 will be "CAMELCASESTRING"
 */
CPP_TOOLBOX_EXPORT auto to_upper(std::string_view s) -> std::string;

/**
 * @brief Converts a string view to a standard string.
 *
 * This function creates a new std::string object containing the same character
 * sequence as the input string view. It is useful for obtaining an owned copy
 * of the string data.
 *
 * @param s The input string view to convert.
 * @return A std::string object containing the same character sequence.
 *
 * @example
 * // Convert string view to string
 * auto result = to_string("hello");
 * // result will be a std::string containing "hello"
 *
 * @example
 * // Convert substring view to string
 * std::string_view sv = "long string";
 * auto result2 = to_string(sv.substr(0, 4));
 * // result2 will be a std::string containing "long"
 */
CPP_TOOLBOX_EXPORT auto to_string(std::string_view s) -> std::string;

/**
 * @brief Pads a string on the left with a specified character to reach a
 * minimum width.
 *
 * This function adds padding characters to the left of the input string until
 * it reaches the specified width. If the string is already longer than the
 * target width, it returns a copy of the original string.
 *
 * @param s The input string view to pad.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @return A new std::string, padded on the left if necessary.
 *
 * @example
 * // Left pad with spaces
 * auto result = left_pad("hello", 10);
 * // result will be "     hello"
 *
 * @example
 * // Left pad with zeros
 * auto result2 = left_pad("42", 5, '0');
 * // result2 will be "00042"
 */
CPP_TOOLBOX_EXPORT auto left_pad(std::string_view s,
                                 std::size_t width,
                                 char pad_char = ' ') -> std::string;

/**
 * @brief Pads a string on the right with a specified character to reach a
 * minimum width.
 *
 * This function adds padding characters to the right of the input string until
 * it reaches the specified width. If the string is already longer than the
 * target width, it returns a copy of the original string.
 *
 * @param s The input string view to pad.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @return A new std::string, padded on the right if necessary.
 *
 * @example
 * // Right pad with spaces
 * auto result = right_pad("hello", 10);
 * // result will be "hello     "
 *
 * @example
 * // Right pad with dashes
 * auto result2 = right_pad("text", 8, '-');
 * // result2 will be "text----"
 */
CPP_TOOLBOX_EXPORT auto right_pad(std::string_view s,
                                  std::size_t width,
                                  char pad_char = ' ') -> std::string;

/**
 * @brief Pads a string with a specified character to reach a minimum width,
 * controlling padding position.
 *
 * This function adds padding characters to the string based on the specified
 * position parameter. The position parameter controls where the padding is
 * added: 0 for left padding, 1 for right padding, and other values for center
 * padding.
 *
 * @param s The input string view to pad.
 * @param width The minimum desired width of the resulting string.
 * @param pad_char The character to use for padding. Defaults to space (' ').
 * @param position A value indicating the padding position (0 for left, 1 for
 * right, other for center). Defaults to 0.
 * @return A new std::string, padded according to the specified parameters.
 *
 * @example
 * // Left padding (default)
 * auto result = pad("hello", 10);
 * // result will be "     hello"
 *
 * @example
 * // Right padding
 * auto result2 = pad("world", 10, ' ', 1);
 * // result2 will be "world     "
 *
 * @example
 * // Center padding
 * auto result3 = pad("center", 10, '-', 2);
 * // result3 will be "--center--"
 */
CPP_TOOLBOX_EXPORT auto pad(std::string_view s,
                            std::size_t width,
                            char pad_char = ' ',
                            std::size_t position = 0) -> std::string;

/**
 * @brief Reverses the order of characters in a string view.
 *
 * This function creates a new string where the characters are in reverse order
 * compared to the input string view.
 *
 * @param s The input string view to reverse.
 * @return A new std::string with the characters in reverse order.
 *
 * @example
 * // Reverse a string
 * auto result = reverse("hello");
 * // result will be "olleh"
 *
 * @example
 * // Reverse a palindrome
 * auto result2 = reverse("racecar");
 * // result2 will be "racecar"
 */
CPP_TOOLBOX_EXPORT auto reverse(std::string_view s) -> std::string;

/**
 * @brief Attempts to parse an integer from a string view.
 *
 * This function uses standard C++ parsing mechanisms (like `std::from_chars` or
 * `std::stoi`) to convert a string to an integer. It performs validation to
 * ensure the string contains a valid integer format and checks for potential
 * overflow conditions.
 *
 * @param s The string view to parse.
 * @param out Reference to an integer where the parsed value will be stored if
 * successful.
 * @return True if parsing was successful, false otherwise. The output parameter
 * `out` is only modified when the function returns true.
 *
 * @example
 * // Successful parsing
 * int value;
 * bool success = try_parse_int("12345", value);
 * // success will be true, value will be 12345
 *
 * @example
 * // Failed parsing (invalid format)
 * int value;
 * bool success = try_parse_int("12abc", value);
 * // success will be false, value remains unchanged
 *
 * @example
 * // Failed parsing (overflow)
 * int value;
 * bool success = try_parse_int("99999999999999999999", value);
 * // success will be false, value remains unchanged
 */
CPP_TOOLBOX_EXPORT auto try_parse_int(std::string_view s, int& out) -> bool;

/**
 * @brief Attempts to parse a double-precision floating-point number from a
 * string view.
 *
 * This function uses standard C++ parsing mechanisms (like `std::from_chars` or
 * `std::stod`) to convert a string to a double. It performs validation to
 * ensure the string contains a valid floating-point format and checks for
 * potential overflow/underflow conditions.
 *
 * @param s The string view to parse.
 * @param out Reference to a double where the parsed value will be stored if
 * successful.
 * @return True if parsing was successful, false otherwise. The output parameter
 * `out` is only modified when the function returns true.
 *
 * @example
 * // Successful parsing
 * double value;
 * bool success = try_parse_double("3.14159", value);
 * // success will be true, value will be 3.14159
 *
 * @example
 * // Failed parsing (invalid format)
 * double value;
 * bool success = try_parse_double("3.14.15", value);
 * // success will be false, value remains unchanged
 *
 * @example
 * // Failed parsing (overflow)
 * double value;
 * bool success = try_parse_double("1e999", value);
 * // success will be false, value remains unchanged
 */
CPP_TOOLBOX_EXPORT auto try_parse_double(std::string_view s, double& out)
    -> bool;

/**
 * @brief Attempts to parse a single-precision floating-point number from a
 * string view.
 *
 * This function uses standard C++ parsing mechanisms (like `std::from_chars` or
 * `std::stof`) to convert a string to a float. It performs validation to ensure
 * the string contains a valid floating-point format and checks for potential
 * overflow/underflow conditions.
 *
 * @param s The string view to parse.
 * @param out Reference to a float where the parsed value will be stored if
 * successful.
 * @return True if parsing was successful, false otherwise. The output parameter
 * `out` is only modified when the function returns true.
 *
 * @example
 * // Successful parsing
 * float value;
 * bool success = try_parse_float("2.71828", value);
 * // success will be true, value will be 2.71828f
 *
 * @example
 * // Failed parsing (invalid format)
 * float value;
 * bool success = try_parse_float("2.71.82", value);
 * // success will be false, value remains unchanged
 *
 * @example
 * // Failed parsing (overflow)
 * float value;
 * bool success = try_parse_float("1e99", value);
 * // success will be false, value remains unchanged
 */
CPP_TOOLBOX_EXPORT auto try_parse_float(std::string_view s, float& out) -> bool;

/**
 * @brief Calculates the Levenshtein distance between two strings.
 *
 * The Levenshtein distance is a string metric for measuring the difference
 * between two sequences. It is defined as the minimum number of
 * single-character edits (insertions, deletions, or substitutions) required to
 * change one string into the other.
 *
 * @param s1 The first string view.
 * @param s2 The second string view.
 * @return The Levenshtein distance between s1 and s2 (a non-negative integer).
 *
 * @example
 * // Distance between "kitten" and "sitting"
 * auto distance = levenshtein_distance("kitten", "sitting");
 * // distance will be 3 (k→s, e→i, +g)
 *
 * @example
 * // Distance between "book" and "back"
 * auto distance2 = levenshtein_distance("book", "back");
 * // distance2 will be 2 (o→a, o→c)
 *
 * @example
 * // Distance between identical strings
 * auto distance3 = levenshtein_distance("same", "same");
 * // distance3 will be 0
 */
CPP_TOOLBOX_EXPORT auto levenshtein_distance(std::string_view s1,
                                             std::string_view s2)
    -> std::size_t;

/**
 * @brief Calculates the length of the Longest Common Subsequence (LCS) of two
 * strings.
 *
 * A subsequence is a sequence that appears in the same relative order but not
 * necessarily contiguous. This function finds the length of the longest such
 * sequence common to both input strings.
 *
 * @param s1 The first string view.
 * @param s2 The second string view.
 * @return The length of the longest common subsequence.
 *
 * @example
 * // LCS of "ABCDEF" and "AUBCDF"
 * auto lcs_len = longest_common_subsequence_length("ABCDEF", "AUBCDF");
 * // lcs_len will be 4 (ABCD)
 *
 * @example
 * // LCS of "AGGTAB" and "GXTXAYB"
 * auto lcs_len2 = longest_common_subsequence_length("AGGTAB", "GXTXAYB");
 * // lcs_len2 will be 4 (GTAB)
 *
 * @example
 * // LCS of completely different strings
 * auto lcs_len3 = longest_common_subsequence_length("abc", "xyz");
 * // lcs_len3 will be 0
 */
CPP_TOOLBOX_EXPORT auto longest_common_subsequence_length(std::string_view s1,
                                                          std::string_view s2)
    -> std::size_t;

/**
 * @brief Calculates the length of the Longest Common Substring of two strings.
 *
 * A substring is a contiguous sequence of characters within a string. This
 * function finds the length of the longest such sequence common to both input
 * strings.
 *
 * @param s1 The first string view.
 * @param s2 The second string view.
 * @return The length of the longest common substring.
 *
 * @example
 * // Longest common substring of "ABABC" and "BABCA"
 * auto lcstr_len = longest_common_substring_length("ABABC", "BABCA");
 * // lcstr_len will be 4 (BABC)
 *
 * @example
 * // Longest common substring of "abcdef" and "zcdemf"
 * auto lcstr_len2 = longest_common_substring_length("abcdef", "zcdemf");
 * // lcstr_len2 will be 3 (cde)
 *
 * @example
 * // Longest common substring of completely different strings
 * auto lcstr_len3 = longest_common_substring_length("abc", "xyz");
 * // lcstr_len3 will be 0
 */
CPP_TOOLBOX_EXPORT auto longest_common_substring_length(std::string_view s1,
                                                        std::string_view s2)
    -> std::size_t;

/**
 * @brief Percent-encodes a string for safe inclusion in a URL.
 *
 * This function implements URL encoding (also known as percent-encoding)
 * according to RFC 3986. It encodes all characters except unreserved characters
 * (alphanumeric, '-',
 * '_', '.', '~'). Spaces are encoded as "%20", and other characters are encoded
 * as %XX where XX is the hexadecimal representation of the character's byte
 * value.
 *
 * @param s The string view to encode.
 * @return The URL-encoded string.
 *
 * @example
 * // Encoding a simple string
 * auto encoded = url_encode("Hello World!");
 * // encoded will be "Hello%20World%21"
 *
 * @example
 * // Encoding special characters
 * auto encoded2 = url_encode("100% true");
 * // encoded2 will be "100%25%20true"
 *
 * @example
 * // Encoding a URL with query parameters
 * auto encoded3 = url_encode("https://example.com/search?q=c++");
 * // encoded3 will be "https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B"
 */
CPP_TOOLBOX_EXPORT auto url_encode(std::string_view s) -> std::string;

/**
 * @brief Decodes a percent-encoded string (URL decoding).
 *
 * This function implements URL decoding (percent-decoding) according to RFC
 * 3986. It converts %XX sequences back to their original characters, where XX
 * is a hexadecimal value. Also handles '+' characters by converting them to
 * spaces, for compatibility with form encoding.
 *
 * @param s The URL-encoded string view to decode.
 * @return The decoded string. Returns an empty string if decoding fails due to
 * invalid % sequences or other encoding errors.
 *
 * @example
 * // Decode a simple URL-encoded string
 * auto decoded = url_decode("Hello%20World%21");
 * // decoded will be "Hello World!"
 *
 * @example
 * // Decode with form encoding compatibility
 * auto decoded2 = url_decode("100%25+true");
 * // decoded2 will be "100% true"
 *
 * @example
 * // Decode a complex URL
 * auto decoded3 =
 * url_decode("https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B");
 * // decoded3 will be "https://example.com/search?q=c++"
 */
CPP_TOOLBOX_EXPORT auto url_decode(std::string_view s) -> std::string;

/**
 * @brief Encodes binary data (represented as a string_view) into Base64 format.
 *
 * This function implements Base64 encoding according to RFC 4648. It converts
 * binary data into a ASCII string representation using the Base64 alphabet.
 * The output string will be padded with '=' characters if necessary to make
 * its length a multiple of 4.
 *
 * @param data The raw binary data to encode, represented as a string_view.
 * @return The Base64 encoded string.
 *
 * @example
 * // Encode simple text
 * auto encoded = base64_encode("Hello World");
 * // encoded will be "SGVsbG8gV29ybGQ="
 *
 * @example
 * // Encode binary data
 * std::string binary_data = {0x00, 0x01, 0x02, 0x03};
 * auto encoded2 = base64_encode(binary_data);
 * // encoded2 will be "AAECAw=="
 */
CPP_TOOLBOX_EXPORT auto base64_encode(std::string_view data) -> std::string;

/**
 * @brief Decodes a Base64 encoded string back into its original data.
 *
 * This function implements Base64 decoding according to RFC 4648. It handles
 * padding characters ('=') and ignores whitespace characters within the input
 * string. The function returns the decoded binary data as a string.
 *
 * @param encoded_data The Base64 encoded string view to decode.
 * @return The decoded data as a string. Returns an empty string if decoding
 * fails due to invalid input (e.g., invalid characters, incorrect padding).
 *
 * @example
 * // Decode simple Base64 string
 * auto decoded = base64_decode("SGVsbG8gV29ybGQ=");
 * // decoded will be "Hello World"
 *
 * @example
 * // Decode with whitespace
 * auto decoded2 = base64_decode(" AA EC Aw == ");
 * // decoded2 will be "\x00\x01\x02\x03"
 *
 * @example
 * // Invalid input returns empty string
 * auto decoded3 = base64_decode("Invalid!");
 * // decoded3 will be ""
 */
CPP_TOOLBOX_EXPORT auto base64_decode(std::string_view encoded_data)
    -> std::string;

/**
 * @brief Converts a string into a URL-friendly "slug".
 *
 * This function transforms a string into a format suitable for use in URLs.
 * It performs the following transformations:
 * 1. Converts the string to lowercase
 * 2. Replaces spaces and consecutive non-alphanumeric characters with a single
 * hyphen
 * 3. Removes other non-alphanumeric characters (except hyphens)
 * 4. Trims leading and trailing hyphens
 *
 * @param s The input string view to convert.
 * @return The generated slug string.
 *
 * @example
 * // Basic slug generation
 * auto slug = slugify("Hello World!");
 * // slug will be "hello-world"
 *
 * @example
 * // Complex string conversion
 * auto slug2 = slugify("This is a Test -- 123!");
 * // slug2 will be "this-is-a-test-123"
 *
 * @example
 * // Handling special characters
 * auto slug3 = slugify("C++ Programming 101");
 * // slug3 will be "c-programming-101"
 */
CPP_TOOLBOX_EXPORT auto slugify(std::string_view s) -> std::string;

}  // namespace toolbox::container::string
#endif  // CPP_TOOLBOX_CONTAINER_STRING_HPP

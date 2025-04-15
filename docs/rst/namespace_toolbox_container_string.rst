.. index:: pair: namespace; toolbox::container::string
.. _doxid-namespacetoolbox_1_1container_1_1string:

namespace toolbox::container::string
====================================

.. toctree::
	:hidden:

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace string {

	// global functions

	CPP_TOOLBOX_EXPORT auto
	:ref:`split<doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a>`(
		std::string_view str,
		std::string_view delimiter
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`split<doxid-namespacetoolbox_1_1container_1_1string_1a4df8b3fed6f1414348272acca9884f3c>`(
		std::string_view str,
		char delimiter
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`join<doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4>`(
		const std::vector<std::string>& parts,
		std::string_view glue
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`join<doxid-namespacetoolbox_1_1container_1_1string_1a4f457265ff3716c7be7782101b2a8c24>`(
		const std::vector<std::string_view>& parts,
		std::string_view glue
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`trim_left<doxid-namespacetoolbox_1_1container_1_1string_1a3d279eb61e3b616544f55127a6253157>`(std::string_view str);

	CPP_TOOLBOX_EXPORT auto
	:ref:`trim_right<doxid-namespacetoolbox_1_1container_1_1string_1afde8a34b13dae9538085d224846ae150>`(std::string_view str);

	CPP_TOOLBOX_EXPORT auto
	:ref:`trim<doxid-namespacetoolbox_1_1container_1_1string_1a0c3328d557f247bf50e7d217860c773b>`(std::string_view str);

	CPP_TOOLBOX_EXPORT auto
	:ref:`starts_with<doxid-namespacetoolbox_1_1container_1_1string_1af0c06fca35d12ef89f919b7adea4a603>`(
		std::string_view s,
		std::string_view prefix
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`ends_with<doxid-namespacetoolbox_1_1container_1_1string_1a338bcf1ed2a6371f5e1f9b8fcb37a3bd>`(
		std::string_view s,
		std::string_view suffix
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`contains<doxid-namespacetoolbox_1_1container_1_1string_1a7aeef0d623cbc0f3363f76254ad3dcbe>`(
		std::string_view s,
		std::string_view substring
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_empty_or_whitespace<doxid-namespacetoolbox_1_1container_1_1string_1a3c4e4f083067427959b909ef72c2572c>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_numeric<doxid-namespacetoolbox_1_1container_1_1string_1a298d9ca66bdcb1f33cfc7cf5a0a9408d>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_integer<doxid-namespacetoolbox_1_1container_1_1string_1a598ab7a0c4cdb7dd64b5f7beca650874>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_float<doxid-namespacetoolbox_1_1container_1_1string_1a02c094d493d2e3469b7dd1f908e1779a>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`replace<doxid-namespacetoolbox_1_1container_1_1string_1a07665505e11dbafe025bfe8bdf6f54ee>`(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` count = std::numeric_limits<:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`>::max()
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`replace_all<doxid-namespacetoolbox_1_1container_1_1string_1a59eb0df9fb535bd78ce425e5585a8b96>`(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`replace_by_nth<doxid-namespacetoolbox_1_1container_1_1string_1a6cd988a07b4fd7e8e5c9eb19b1b96dcf>`(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` n
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`remove_nth<doxid-namespacetoolbox_1_1container_1_1string_1af65f000fce00e0783423dd06feac6139>`(
		std::string_view s,
		std::string_view from,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` n
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`remove<doxid-namespacetoolbox_1_1container_1_1string_1aac5b7ea533715149a2aa9ee3a6c9bb8b>`(
		std::string_view s,
		std::string_view value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` count = std::numeric_limits<:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`>::max()
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`remove_all<doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38>`(
		std::string_view s,
		std::string_view value
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`remove_all<doxid-namespacetoolbox_1_1container_1_1string_1a3e4961d94fcd4a87a6956d6ff96bfe28>`(
		std::string_view s,
		char value
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`to_lower<doxid-namespacetoolbox_1_1container_1_1string_1a11fa99f0b4bf9ce0e0ed35306a8d210b>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`to_upper<doxid-namespacetoolbox_1_1container_1_1string_1a7590fc540fcff0df33cf1b3cfda6b03c>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`to_string<doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`left_pad<doxid-namespacetoolbox_1_1container_1_1string_1a048891f47e89fbd9e898ad8fb1f196c7>`(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' '
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`right_pad<doxid-namespacetoolbox_1_1container_1_1string_1a1db71b47681be86df5fea13f8999e2aa>`(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' '
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`pad<doxid-namespacetoolbox_1_1container_1_1string_1a8b3352651f569c5ed05d6ec2ce0a9412>`(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' ',
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` position = 0
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`reverse<doxid-namespacetoolbox_1_1container_1_1string_1a13e2b7a2101c8a7497a9b15ad307ab01>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`try_parse_int<doxid-namespacetoolbox_1_1container_1_1string_1a177efbbd07806730f409280e993beb94>`(
		std::string_view s,
		int& out
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`try_parse_double<doxid-namespacetoolbox_1_1container_1_1string_1afd44eb3b69488f814fb5eb059784af56>`(
		std::string_view s,
		double& out
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`try_parse_float<doxid-namespacetoolbox_1_1container_1_1string_1a477149220be79a76aeab534853bf5721>`(
		std::string_view s,
		float& out
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`levenshtein_distance<doxid-namespacetoolbox_1_1container_1_1string_1adf00945757752e93f4592917811033c5>`(
		std::string_view s1,
		std::string_view s2
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`longest_common_subsequence_length<doxid-namespacetoolbox_1_1container_1_1string_1a769608e53956a31c6920fdd30d30035b>`(
		std::string_view s1,
		std::string_view s2
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`longest_common_substring_length<doxid-namespacetoolbox_1_1container_1_1string_1a0b7d1d7d53c01978d757929bb13b204f>`(
		std::string_view s1,
		std::string_view s2
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`url_encode<doxid-namespacetoolbox_1_1container_1_1string_1a19556e830ae2bfbbbbe0905a454844f4>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`url_decode<doxid-namespacetoolbox_1_1container_1_1string_1ace2f63ba20582b4e048fefeac749de3d>`(std::string_view s);

	CPP_TOOLBOX_EXPORT auto
	:ref:`base64_encode<doxid-namespacetoolbox_1_1container_1_1string_1af2e3a6aeac0c69c43250b792e035e593>`(std::string_view data);

	CPP_TOOLBOX_EXPORT auto
	:ref:`base64_decode<doxid-namespacetoolbox_1_1container_1_1string_1a4267ad272a87495724c13bbb5b607ec5>`(std::string_view encoded_data);

	CPP_TOOLBOX_EXPORT auto
	:ref:`slugify<doxid-namespacetoolbox_1_1container_1_1string_1a5c4d6af2e41f817dca51139247152a02>`(std::string_view s);

	} // namespace string
.. _details-namespacetoolbox_1_1container_1_1string:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; split
.. _doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	split(
		std::string_view str,
		std::string_view delimiter
	)

Splits a string view into a vector of strings based on a string delimiter.

Splits the input string at each occurrence of the delimiter string. Empty parts between delimiters are included in the result. The delimiter itself is not included in the output.

.. ref-code-block:: cpp

	// Split by string delimiter
	auto result = :ref:`split <doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a>`("apple,orange,,banana", ",");
	// result will be {"apple", "orange", "", "banana"}
	
	// Split with multi-character delimiter
	auto result2 = :ref:`split <doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a>`("one->two->three", "->");
	// result2 will be {"one", "two", "three"}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The string view to split.

	*
		- delimiter

		- The string view to use as a delimiter.



.. rubric:: Returns:

std::vector<std::string> A vector of strings resulting from the split.

.. index:: pair: function; split
.. _doxid-namespacetoolbox_1_1container_1_1string_1a4df8b3fed6f1414348272acca9884f3c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	split(
		std::string_view str,
		char delimiter
	)

Splits a string view into a vector of strings based on a character delimiter.

Splits the input string at each occurrence of the delimiter character. Empty parts between delimiters are included in the result. The delimiter character itself is not included in the output.

.. ref-code-block:: cpp

	// Split by character delimiter
	auto result = :ref:`split <doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a>`("one|two||three", '|');
	// result will be {"one", "two", "", "three"}
	
	// Split with space delimiter
	auto result2 = :ref:`split <doxid-namespacetoolbox_1_1container_1_1string_1aa78cfc3e9c8968b9494a687e31b0484a>`("hello world  cpp", ' ');
	// result2 will be {"hello", "world", "", "cpp"}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The string view to split.

	*
		- delimiter

		- The character to use as a delimiter.



.. rubric:: Returns:

std::vector<std::string> A vector of strings resulting from the split.

.. index:: pair: function; join
.. _doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	join(
		const std::vector<std::string>& parts,
		std::string_view glue
	)

Joins a vector of strings into a single string, separated by a glue string.

Concatenates all strings in the vector, inserting the glue string between each element. If the input vector is empty, returns an empty string.

.. ref-code-block:: cpp

	// Join with comma separator
	auto result = :ref:`join <doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4>`({"apple", "banana", "cherry"}, ", ");
	// result will be "apple, banana, cherry"
	
	// Join with empty glue
	auto result2 = :ref:`join <doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4>`({"a", "b", "c"}, "");
	// result2 will be "abc"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- parts

		- The vector of strings to join.

	*
		- glue

		- The string view to insert between elements.



.. rubric:: Returns:

std::string The resulting joined string.

.. index:: pair: function; join
.. _doxid-namespacetoolbox_1_1container_1_1string_1a4f457265ff3716c7be7782101b2a8c24:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	join(
		const std::vector<std::string_view>& parts,
		std::string_view glue
	)

Joins a vector of string views into a single string, separated by a glue string.

Concatenates all string views in the vector, inserting the glue string between each element. If the input vector is empty, returns an empty string.

.. ref-code-block:: cpp

	// Join string views with hyphen and newline
	auto result = :ref:`join <doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4>`({"2023", "12", "31"}, "-");
	// result will be "2023-12-31"
	
	auto result2 = :ref:`join <doxid-namespacetoolbox_1_1container_1_1string_1ade0369d79e2b2e82054f851b9198c2b4>`({"line1", "line2", "line3"}, "\n");
	// result2 will be "line1\nline2\nline3"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- parts

		- The vector of string views to join.

	*
		- glue

		- The string view to insert between elements.



.. rubric:: Returns:

std::string The resulting joined string.

.. index:: pair: function; trim_left
.. _doxid-namespacetoolbox_1_1container_1_1string_1a3d279eb61e3b616544f55127a6253157:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	trim_left(std::string_view str)

Removes leading whitespace characters from a string view.

Creates a new string with all leading whitespace characters removed. Whitespace characters are determined by ``std::isspace`` (spaces, tabs, newlines, etc.).

.. ref-code-block:: cpp

	// Trim leading whitespace examples
	auto result = :ref:`trim_left <doxid-namespacetoolbox_1_1container_1_1string_1a3d279eb61e3b616544f55127a6253157>`("   hello");
	// result will be "hello"
	
	auto result2 = :ref:`trim_left <doxid-namespacetoolbox_1_1container_1_1string_1a3d279eb61e3b616544f55127a6253157>`("\t\n  world");
	// result2 will be "world"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The input string view.



.. rubric:: Returns:

std::string A new string with leading whitespace removed.

.. index:: pair: function; trim_right
.. _doxid-namespacetoolbox_1_1container_1_1string_1afde8a34b13dae9538085d224846ae150:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	trim_right(std::string_view str)

Removes trailing whitespace characters from a string view.

Creates a new string with all trailing whitespace characters removed. Whitespace characters are determined by ``std::isspace`` (spaces, tabs, newlines, etc.).

.. ref-code-block:: cpp

	// Trim trailing whitespace examples
	auto result = :ref:`trim_right <doxid-namespacetoolbox_1_1container_1_1string_1afde8a34b13dae9538085d224846ae150>`("hello   ");
	// result will be "hello"
	
	auto result2 = :ref:`trim_right <doxid-namespacetoolbox_1_1container_1_1string_1afde8a34b13dae9538085d224846ae150>`("world\t\n  ");
	// result2 will be "world"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The input string view.



.. rubric:: Returns:

std::string A new string with trailing whitespace removed.

.. index:: pair: function; trim
.. _doxid-namespacetoolbox_1_1container_1_1string_1a0c3328d557f247bf50e7d217860c773b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	trim(std::string_view str)

Removes both leading and trailing whitespace characters from a string view.

Creates a new string with all leading and trailing whitespace characters removed. Whitespace characters are determined by ``std::isspace``.

.. ref-code-block:: cpp

	// Trim both ends examples
	auto result = :ref:`trim <doxid-namespacetoolbox_1_1container_1_1string_1a0c3328d557f247bf50e7d217860c773b>`("   hello world   ");
	// result will be "hello world"
	
	auto result2 = :ref:`trim <doxid-namespacetoolbox_1_1container_1_1string_1a0c3328d557f247bf50e7d217860c773b>`("\t\n  test string \n\t");
	// result2 will be "test string"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The input string view.



.. rubric:: Returns:

std::string A new string with leading and trailing whitespace removed.

.. index:: pair: function; starts_with
.. _doxid-namespacetoolbox_1_1container_1_1string_1af0c06fca35d12ef89f919b7adea4a603:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	starts_with(
		std::string_view s,
		std::string_view prefix
	)

Checks if a string view starts with a specified prefix.

Performs a case-sensitive comparison to check if the string begins with the given prefix. Both empty strings and empty prefixes are handled.

.. ref-code-block:: cpp

	// Check prefix examples
	bool result = :ref:`starts_with <doxid-namespacetoolbox_1_1container_1_1string_1af0c06fca35d12ef89f919b7adea4a603>`("hello world", "hello");
	// result will be true
	
	bool result2 = :ref:`starts_with <doxid-namespacetoolbox_1_1container_1_1string_1af0c06fca35d12ef89f919b7adea4a603>`("hello world", "world");
	// result2 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.

	*
		- prefix

		- The prefix string view to look for.



.. rubric:: Returns:

bool True if the string starts with the prefix, false otherwise.

.. index:: pair: function; ends_with
.. _doxid-namespacetoolbox_1_1container_1_1string_1a338bcf1ed2a6371f5e1f9b8fcb37a3bd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	ends_with(
		std::string_view s,
		std::string_view suffix
	)

Checks if a string view ends with a specified suffix.

Performs a case-sensitive comparison to check if the string ends with the given suffix. Both empty strings and empty suffixes are handled.

.. ref-code-block:: cpp

	// Check suffix examples
	bool result = :ref:`ends_with <doxid-namespacetoolbox_1_1container_1_1string_1a338bcf1ed2a6371f5e1f9b8fcb37a3bd>`("hello world", "world");
	// result will be true
	
	bool result2 = :ref:`ends_with <doxid-namespacetoolbox_1_1container_1_1string_1a338bcf1ed2a6371f5e1f9b8fcb37a3bd>`("hello world", "hello");
	// result2 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.

	*
		- suffix

		- The suffix string view to look for.



.. rubric:: Returns:

bool True if the string ends with the suffix, false otherwise.

.. index:: pair: function; contains
.. _doxid-namespacetoolbox_1_1container_1_1string_1a7aeef0d623cbc0f3363f76254ad3dcbe:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	contains(
		std::string_view s,
		std::string_view substring
	)

Checks if a string view contains a specified substring.

Performs a case-sensitive search for the substring within the string. Both empty strings and empty substrings are handled.

.. ref-code-block:: cpp

	// Check substring examples
	bool result = :ref:`contains <doxid-namespacetoolbox_1_1container_1_1string_1a7aeef0d623cbc0f3363f76254ad3dcbe>`("hello world", "lo wo");
	// result will be true
	
	bool result2 = :ref:`contains <doxid-namespacetoolbox_1_1container_1_1string_1a7aeef0d623cbc0f3363f76254ad3dcbe>`("hello world", "goodbye");
	// result2 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to search within.

	*
		- substring

		- The substring to search for.



.. rubric:: Returns:

bool True if the substring is found within the string, false otherwise.

.. index:: pair: function; is_empty_or_whitespace
.. _doxid-namespacetoolbox_1_1container_1_1string_1a3c4e4f083067427959b909ef72c2572c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_empty_or_whitespace(std::string_view s)

Checks if a string view is empty or consists only of whitespace characters.

Determines if the string is either empty or contains only characters that are considered whitespace by ``std::isspace``.

.. ref-code-block:: cpp

	// Check empty or whitespace examples
	bool result = :ref:`is_empty_or_whitespace <doxid-namespacetoolbox_1_1container_1_1string_1a3c4e4f083067427959b909ef72c2572c>`("");
	// result will be true
	
	bool result2 = :ref:`is_empty_or_whitespace <doxid-namespacetoolbox_1_1container_1_1string_1a3c4e4f083067427959b909ef72c2572c>`(" \t\n");
	// result2 will be true
	
	bool result3 = :ref:`is_empty_or_whitespace <doxid-namespacetoolbox_1_1container_1_1string_1a3c4e4f083067427959b909ef72c2572c>`("hello");
	// result3 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.



.. rubric:: Returns:

bool True if the string is empty or contains only whitespace, false otherwise.

.. index:: pair: function; is_numeric
.. _doxid-namespacetoolbox_1_1container_1_1string_1a298d9ca66bdcb1f33cfc7cf5a0a9408d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_numeric(std::string_view s)

Checks if a string view represents a numeric value (integer or floating-point).

Performs a basic check for numeric format, including optional sign, digits, and optional decimal point. Does not handle locale-specific formats or scientific notation.

.. ref-code-block:: cpp

	// Check numeric format examples
	bool result = :ref:`is_numeric <doxid-namespacetoolbox_1_1container_1_1string_1a298d9ca66bdcb1f33cfc7cf5a0a9408d>`("12345");
	// result will be true
	
	bool result2 = :ref:`is_numeric <doxid-namespacetoolbox_1_1container_1_1string_1a298d9ca66bdcb1f33cfc7cf5a0a9408d>`("-123.45");
	// result2 will be true
	
	bool result3 = :ref:`is_numeric <doxid-namespacetoolbox_1_1container_1_1string_1a298d9ca66bdcb1f33cfc7cf5a0a9408d>`("123abc");
	// result3 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.



.. rubric:: Returns:

bool True if the string appears to be numeric, false otherwise.

.. index:: pair: function; is_integer
.. _doxid-namespacetoolbox_1_1container_1_1string_1a598ab7a0c4cdb7dd64b5f7beca650874:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_integer(std::string_view s)

Checks if a string view represents a valid integer value.

This function verifies if the input string consists of optional leading sign (+/-) followed by one or more digits. It does not handle locale-specific formats or scientific notation.

.. ref-code-block:: cpp

	// Check integer format examples
	bool result1 = :ref:`is_integer <doxid-namespacetoolbox_1_1container_1_1string_1a598ab7a0c4cdb7dd64b5f7beca650874>`("12345");
	// result1 will be true
	
	bool result2 = :ref:`is_integer <doxid-namespacetoolbox_1_1container_1_1string_1a598ab7a0c4cdb7dd64b5f7beca650874>`("-42");
	// result2 will be true
	
	bool result3 = :ref:`is_integer <doxid-namespacetoolbox_1_1container_1_1string_1a598ab7a0c4cdb7dd64b5f7beca650874>`("12.34");
	// result3 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.



.. rubric:: Returns:

True if the string is a valid integer, false otherwise.

.. index:: pair: function; is_float
.. _doxid-namespacetoolbox_1_1container_1_1string_1a02c094d493d2e3469b7dd1f908e1779a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_float(std::string_view s)

Checks if a string view represents a valid floating-point value.

This function verifies if the input string consists of optional leading sign (+/-), digits, and optional decimal point. It does not handle locale-specific formats, scientific notation, or special floating-point values (NaN, infinity).

.. ref-code-block:: cpp

	// Check float format examples
	bool result1 = :ref:`is_float <doxid-namespacetoolbox_1_1container_1_1string_1a02c094d493d2e3469b7dd1f908e1779a>`("123.45");
	// result1 will be true
	
	bool result2 = :ref:`is_float <doxid-namespacetoolbox_1_1container_1_1string_1a02c094d493d2e3469b7dd1f908e1779a>`("-0.123");
	// result2 will be true
	
	bool result3 = :ref:`is_float <doxid-namespacetoolbox_1_1container_1_1string_1a02c094d493d2e3469b7dd1f908e1779a>`("12.34.56");
	// result3 will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to check.



.. rubric:: Returns:

True if the string is a valid floating-point number, false otherwise.

.. index:: pair: function; replace
.. _doxid-namespacetoolbox_1_1container_1_1string_1a07665505e11dbafe025bfe8bdf6f54ee:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	replace(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` count = std::numeric_limits<:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`>::max()
	)

Replaces occurrences of a substring within a string view with another substring, up to a specified count.

This function performs up to 'count' replacements of 'old_value' with 'new_value' in the input string. If 'old_value' is empty, it inserts 'new_value' between characters and at the start of the string.

.. ref-code-block:: cpp

	// Replace substring examples
	auto result1 = :ref:`replace <doxid-namespacetoolbox_1_1container_1_1string_1a07665505e11dbafe025bfe8bdf6f54ee>`("hello world", "world", "cpp");
	// result1 will be "hello cpp"
	
	auto result2 = :ref:`replace <doxid-namespacetoolbox_1_1container_1_1string_1a07665505e11dbafe025bfe8bdf6f54ee>`("banana", "na", "no", 2);
	// result2 will be "bonona"
	
	auto result3 = :ref:`replace <doxid-namespacetoolbox_1_1container_1_1string_1a07665505e11dbafe025bfe8bdf6f54ee>`("abc", "", "-", 2);
	// result3 will be "-a-bc"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view.

	*
		- old_value

		- The substring to be replaced.

	*
		- new_value

		- The substring to replace with.

	*
		- count

		- The maximum number of replacements to perform. Defaults to replacing all occurrences.



.. rubric:: Returns:

A new string with the replacements made.

.. index:: pair: function; replace_all
.. _doxid-namespacetoolbox_1_1container_1_1string_1a59eb0df9fb535bd78ce425e5585a8b96:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	replace_all(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value
	)

Replaces all occurrences of a substring within a string view with another substring.

This function replaces all instances of 'old_value' with 'new_value' in the input string. It is equivalent to calling ``replace`` with the default count.

.. ref-code-block:: cpp

	// Replace all occurrences examples
	auto result = :ref:`replace_all <doxid-namespacetoolbox_1_1container_1_1string_1a59eb0df9fb535bd78ce425e5585a8b96>`("banana", "na", "no");
	// result will be "bonono"
	
	auto result2 = :ref:`replace_all <doxid-namespacetoolbox_1_1container_1_1string_1a59eb0df9fb535bd78ce425e5585a8b96>`("hello", "x", "y");
	// result2 will be "hello"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view.

	*
		- old_value

		- The substring to be replaced.

	*
		- new_value

		- The substring to replace with.



.. rubric:: Returns:

A new string with all occurrences replaced.

.. index:: pair: function; replace_by_nth
.. _doxid-namespacetoolbox_1_1container_1_1string_1a6cd988a07b4fd7e8e5c9eb19b1b96dcf:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	replace_by_nth(
		std::string_view s,
		std::string_view old_value,
		std::string_view new_value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` n
	)

Replaces the Nth occurrence of a substring within a string.

This function replaces only the specified occurrence (1-based index) of 'old_value' with 'new_value'. If the specified occurrence is not found or 'old_value' is empty, returns a copy of the original string.

.. ref-code-block:: cpp

	// Replace nth occurrence examples
	auto result1 = :ref:`replace_by_nth <doxid-namespacetoolbox_1_1container_1_1string_1a6cd988a07b4fd7e8e5c9eb19b1b96dcf>`("banana", "na", "no", 2);
	// result1 will be "banona"
	
	auto result2 = :ref:`replace_by_nth <doxid-namespacetoolbox_1_1container_1_1string_1a6cd988a07b4fd7e8e5c9eb19b1b96dcf>`("hello", "l", "x", 3);
	// result2 will be "hello"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view.

	*
		- old_value

		- The substring to find. Cannot be empty.

	*
		- new_value

		- The substring to replace with.

	*
		- n

		- The 1-based index of the occurrence to replace (1 for the first, 2 for the second, etc.). Must be > 0.



.. rubric:: Returns:

A new std::string with the Nth occurrence replaced, or a copy of the original string if n is invalid, 'from' is empty, or the Nth occurrence is not found.

.. index:: pair: function; remove_nth
.. _doxid-namespacetoolbox_1_1container_1_1string_1af65f000fce00e0783423dd06feac6139:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	remove_nth(
		std::string_view s,
		std::string_view from,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` n
	)

Removes the Nth occurrence of a substring within a string.

This function removes only the specified occurrence (1-based index) of 'from' in the input string. If the specified occurrence is not found or 'from' is empty, returns a copy of the original string.

.. ref-code-block:: cpp

	// Remove nth occurrence examples
	auto result1 = :ref:`remove_nth <doxid-namespacetoolbox_1_1container_1_1string_1af65f000fce00e0783423dd06feac6139>`("banana", "na", 2);
	// result1 will be "bana"
	
	auto result2 = :ref:`remove_nth <doxid-namespacetoolbox_1_1container_1_1string_1af65f000fce00e0783423dd06feac6139>`("hello", "l", 3);
	// result2 will be "hello"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view.

	*
		- from

		- The substring to find and remove. Cannot be empty.

	*
		- n

		- The 1-based index of the occurrence to remove (1 for the first, 2 for the second, etc.). Must be > 0.



.. rubric:: Returns:

A new std::string with the Nth occurrence removed, or a copy of the original string if n is invalid, 'from' is empty, or the Nth occurrence is not found.

.. index:: pair: function; remove
.. _doxid-namespacetoolbox_1_1container_1_1string_1aac5b7ea533715149a2aa9ee3a6c9bb8b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	remove(
		std::string_view s,
		std::string_view value,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` count = std::numeric_limits<:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`>::max()
	)

Removes occurrences of a substring from a string view, up to a specified count.

This function removes up to 'count' occurrences of 'value' from the input string. It is equivalent to replacing 'value' with an empty string.

.. ref-code-block:: cpp

	// Remove substring examples
	auto result1 = :ref:`remove <doxid-namespacetoolbox_1_1container_1_1string_1aac5b7ea533715149a2aa9ee3a6c9bb8b>`("banana", "na", 2);
	// result1 will be "ba"
	
	auto result2 = :ref:`remove <doxid-namespacetoolbox_1_1container_1_1string_1aac5b7ea533715149a2aa9ee3a6c9bb8b>`("hello world", "l");
	// result2 will be "heo word"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view.

	*
		- value

		- The substring to remove.

	*
		- count

		- The maximum number of occurrences to remove. Defaults to removing all occurrences.



.. rubric:: Returns:

A new string with the specified substring removed.

.. index:: pair: function; remove_all
.. _doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	remove_all(
		std::string_view s,
		std::string_view value
	)

Removes all occurrences of a substring from a string view.

This function removes every instance of the specified substring from the input string. It is equivalent to calling ``remove`` with the default count (removing all occurrences).

.. ref-code-block:: cpp

	// Remove all occurrences examples
	auto result = :ref:`remove_all <doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38>`("banana", "na");
	// result will be "ba"
	
	auto result2 = :ref:`remove_all <doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38>`("hello world hello", "hello");
	// result2 will be " world "



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view to process.

	*
		- value

		- The substring to remove. Cannot be empty.



.. rubric:: Returns:

A new std::string with all occurrences of the substring removed.

.. index:: pair: function; remove_all
.. _doxid-namespacetoolbox_1_1container_1_1string_1a3e4961d94fcd4a87a6956d6ff96bfe28:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	remove_all(
		std::string_view s,
		char value
	)

Removes all occurrences of a specific character from a string view.

This function removes every instance of the specified character from the input string.

.. ref-code-block:: cpp

	// Remove all character examples
	auto result = :ref:`remove_all <doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38>`("hello world", 'l');
	// result will be "heo word"
	
	auto result2 = :ref:`remove_all <doxid-namespacetoolbox_1_1container_1_1string_1a5b83e2e53fedfea757514e8867c94f38>`("text with spaces", ' ');
	// result2 will be "textwithspaces"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The original string view to process.

	*
		- value

		- The character to remove.



.. rubric:: Returns:

A new std::string with all occurrences of the character removed.

.. index:: pair: function; to_lower
.. _doxid-namespacetoolbox_1_1container_1_1string_1a11fa99f0b4bf9ce0e0ed35306a8d210b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	to_lower(std::string_view s)

Converts all characters in a string view to lowercase.

This function creates a new string where all characters are converted to their lowercase equivalents. The conversion is locale-dependent.

.. ref-code-block:: cpp

	// Convert to lowercase examples
	auto result = :ref:`to_lower <doxid-namespacetoolbox_1_1container_1_1string_1a11fa99f0b4bf9ce0e0ed35306a8d210b>`("Hello World");
	// result will be "hello world"
	
	auto result2 = :ref:`to_lower <doxid-namespacetoolbox_1_1container_1_1string_1a11fa99f0b4bf9ce0e0ed35306a8d210b>`("CamelCaseString");
	// result2 will be "camelcasestring"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to convert.



.. rubric:: Returns:

A new std::string with all characters converted to lowercase.

.. index:: pair: function; to_upper
.. _doxid-namespacetoolbox_1_1container_1_1string_1a7590fc540fcff0df33cf1b3cfda6b03c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	to_upper(std::string_view s)

Converts all characters in a string view to uppercase.

This function creates a new string where all characters are converted to their uppercase equivalents. The conversion is locale-dependent.

.. ref-code-block:: cpp

	// Convert to uppercase examples
	auto result = :ref:`to_upper <doxid-namespacetoolbox_1_1container_1_1string_1a7590fc540fcff0df33cf1b3cfda6b03c>`("Hello World");
	// result will be "HELLO WORLD"
	
	auto result2 = :ref:`to_upper <doxid-namespacetoolbox_1_1container_1_1string_1a7590fc540fcff0df33cf1b3cfda6b03c>`("CamelCaseString");
	// result2 will be "CAMELCASESTRING"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to convert.



.. rubric:: Returns:

A new std::string with all characters converted to uppercase.

.. index:: pair: function; to_string
.. _doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	to_string(std::string_view s)

Converts a string view to a standard string.

This function creates a new std::string object containing the same character sequence as the input string view. It is useful for obtaining an owned copy of the string data.

.. ref-code-block:: cpp

	// Convert string view to string examples
	auto result = :ref:`to_string <doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60>`("hello");
	// result will be a std::string containing "hello"
	
	std::string_view sv = "long string";
	auto result2 = :ref:`to_string <doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60>`(sv.substr(0, 4));
	// result2 will be a std::string containing "long"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to convert.



.. rubric:: Returns:

A std::string object containing the same character sequence.

.. index:: pair: function; left_pad
.. _doxid-namespacetoolbox_1_1container_1_1string_1a048891f47e89fbd9e898ad8fb1f196c7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	left_pad(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' '
	)

Pads a string on the left with a specified character to reach a minimum width.

This function adds padding characters to the left of the input string until it reaches the specified width. If the string is already longer than the target width, it returns a copy of the original string.

.. ref-code-block:: cpp

	// Left pad examples
	auto result = :ref:`left_pad <doxid-namespacetoolbox_1_1container_1_1string_1a048891f47e89fbd9e898ad8fb1f196c7>`("hello", 10);
	// result will be "     hello"
	
	auto result2 = :ref:`left_pad <doxid-namespacetoolbox_1_1container_1_1string_1a048891f47e89fbd9e898ad8fb1f196c7>`("42", 5, '0');
	// result2 will be "00042"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to pad.

	*
		- width

		- The minimum desired width of the resulting string.

	*
		- pad_char

		- The character to use for padding. Defaults to space (' ').



.. rubric:: Returns:

A new std::string, padded on the left if necessary.

.. index:: pair: function; right_pad
.. _doxid-namespacetoolbox_1_1container_1_1string_1a1db71b47681be86df5fea13f8999e2aa:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	right_pad(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' '
	)

Pads a string on the right with a specified character to reach a minimum width.

This function adds padding characters to the right of the input string until it reaches the specified width. If the string is already longer than the target width, it returns a copy of the original string.

.. ref-code-block:: cpp

	// Right pad examples
	auto result = :ref:`right_pad <doxid-namespacetoolbox_1_1container_1_1string_1a1db71b47681be86df5fea13f8999e2aa>`("hello", 10);
	// result will be "hello     "
	
	auto result2 = :ref:`right_pad <doxid-namespacetoolbox_1_1container_1_1string_1a1db71b47681be86df5fea13f8999e2aa>`("text", 8, '-');
	// result2 will be "text----"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to pad.

	*
		- width

		- The minimum desired width of the resulting string.

	*
		- pad_char

		- The character to use for padding. Defaults to space (' ').



.. rubric:: Returns:

A new std::string, padded on the right if necessary.

.. index:: pair: function; pad
.. _doxid-namespacetoolbox_1_1container_1_1string_1a8b3352651f569c5ed05d6ec2ce0a9412:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	pad(
		std::string_view s,
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` width,
		char pad_char = ' ',
		:ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` position = 0
	)

Pads a string with a specified character to reach a minimum width, controlling padding position.

This function adds padding characters to the string based on the specified position parameter. The position parameter controls where the padding is added: 0 for left padding, 1 for right padding, and other values for center padding.

.. ref-code-block:: cpp

	// Left padding (default)
	auto result = :ref:`pad <doxid-namespacetoolbox_1_1container_1_1string_1a8b3352651f569c5ed05d6ec2ce0a9412>`("hello", 10);
	// result will be "     hello"
	
	// Right padding
	auto result2 = :ref:`pad <doxid-namespacetoolbox_1_1container_1_1string_1a8b3352651f569c5ed05d6ec2ce0a9412>`("world", 10, ' ', 1);
	// result2 will be "world     "
	
	// Center padding
	auto result3 = :ref:`pad <doxid-namespacetoolbox_1_1container_1_1string_1a8b3352651f569c5ed05d6ec2ce0a9412>`("center", 10, '-', 2);
	// result3 will be "--center--"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to pad.

	*
		- width

		- The minimum desired width of the resulting string.

	*
		- pad_char

		- The character to use for padding. Defaults to space (' ').

	*
		- position

		- A value indicating the padding position (0 for left, 1 for right, other for center). Defaults to 0.



.. rubric:: Returns:

A new std::string, padded according to the specified parameters.

.. index:: pair: function; reverse
.. _doxid-namespacetoolbox_1_1container_1_1string_1a13e2b7a2101c8a7497a9b15ad307ab01:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	reverse(std::string_view s)

Reverses the order of characters in a string view.

This function creates a new string where the characters are in reverse order compared to the input string view.

.. ref-code-block:: cpp

	// Reverse a string
	auto result = :ref:`reverse <doxid-namespacetoolbox_1_1container_1_1string_1a13e2b7a2101c8a7497a9b15ad307ab01>`("hello");
	// result will be "olleh"
	
	// Reverse a palindrome
	auto result2 = :ref:`reverse <doxid-namespacetoolbox_1_1container_1_1string_1a13e2b7a2101c8a7497a9b15ad307ab01>`("racecar");
	// result2 will be "racecar"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to reverse.



.. rubric:: Returns:

A new std::string with the characters in reverse order.

.. index:: pair: function; try_parse_int
.. _doxid-namespacetoolbox_1_1container_1_1string_1a177efbbd07806730f409280e993beb94:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	try_parse_int(
		std::string_view s,
		int& out
	)

Attempts to parse an integer from a string view.

This function uses standard C++ parsing mechanisms (like ``std::from_chars`` or ``std::stoi``) to convert a string to an integer. It performs validation to ensure the string contains a valid integer format and checks for potential overflow conditions.

.. ref-code-block:: cpp

	// Successful parsing
	int value;
	bool success = :ref:`try_parse_int <doxid-namespacetoolbox_1_1container_1_1string_1a177efbbd07806730f409280e993beb94>`("12345", value);
	// success will be true, value will be 12345
	
	// Failed parsing (invalid format)
	int value;
	bool success = :ref:`try_parse_int <doxid-namespacetoolbox_1_1container_1_1string_1a177efbbd07806730f409280e993beb94>`("12abc", value);
	// success will be false, value remains unchanged
	
	// Failed parsing (overflow)
	int value;
	bool success = :ref:`try_parse_int <doxid-namespacetoolbox_1_1container_1_1string_1a177efbbd07806730f409280e993beb94>`("99999999999999999999", value);
	// success will be false, value remains unchanged



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to parse.

	*
		- out

		- Reference to an integer where the parsed value will be stored if successful.



.. rubric:: Returns:

True if parsing was successful, false otherwise. The output parameter ``out`` is only modified when the function returns true.

.. index:: pair: function; try_parse_double
.. _doxid-namespacetoolbox_1_1container_1_1string_1afd44eb3b69488f814fb5eb059784af56:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	try_parse_double(
		std::string_view s,
		double& out
	)

Attempts to parse a double-precision floating-point number from a string view.

This function uses standard C++ parsing mechanisms (like ``std::from_chars`` or ``std::stod``) to convert a string to a double. It performs validation to ensure the string contains a valid floating-point format and checks for potential overflow/underflow conditions.

.. ref-code-block:: cpp

	// Successful parsing
	double value;
	bool success = :ref:`try_parse_double <doxid-namespacetoolbox_1_1container_1_1string_1afd44eb3b69488f814fb5eb059784af56>`("3.14159", value);
	// success will be true, value will be 3.14159
	
	// Failed parsing (invalid format)
	double value;
	bool success = :ref:`try_parse_double <doxid-namespacetoolbox_1_1container_1_1string_1afd44eb3b69488f814fb5eb059784af56>`("3.14.15", value);
	// success will be false, value remains unchanged
	
	// Failed parsing (overflow)
	double value;
	bool success = :ref:`try_parse_double <doxid-namespacetoolbox_1_1container_1_1string_1afd44eb3b69488f814fb5eb059784af56>`("1e999", value);
	// success will be false, value remains unchanged



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to parse.

	*
		- out

		- Reference to a double where the parsed value will be stored if successful.



.. rubric:: Returns:

True if parsing was successful, false otherwise. The output parameter ``out`` is only modified when the function returns true.

.. index:: pair: function; try_parse_float
.. _doxid-namespacetoolbox_1_1container_1_1string_1a477149220be79a76aeab534853bf5721:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	try_parse_float(
		std::string_view s,
		float& out
	)

Attempts to parse a single-precision floating-point number from a string view.

This function uses standard C++ parsing mechanisms (like ``std::from_chars`` or ``std::stof``) to convert a string to a float. It performs validation to ensure the string contains a valid floating-point format and checks for potential overflow/underflow conditions.

.. ref-code-block:: cpp

	// Successful parsing
	float value;
	bool success = :ref:`try_parse_float <doxid-namespacetoolbox_1_1container_1_1string_1a477149220be79a76aeab534853bf5721>`("2.71828", value);
	// success will be true, value will be 2.71828f
	
	// Failed parsing (invalid format)
	float value;
	bool success = :ref:`try_parse_float <doxid-namespacetoolbox_1_1container_1_1string_1a477149220be79a76aeab534853bf5721>`("2.71.82", value);
	// success will be false, value remains unchanged
	
	// Failed parsing (overflow)
	float value;
	bool success = :ref:`try_parse_float <doxid-namespacetoolbox_1_1container_1_1string_1a477149220be79a76aeab534853bf5721>`("1e99", value);
	// success will be false, value remains unchanged



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to parse.

	*
		- out

		- Reference to a float where the parsed value will be stored if successful.



.. rubric:: Returns:

True if parsing was successful, false otherwise. The output parameter ``out`` is only modified when the function returns true.

.. index:: pair: function; levenshtein_distance
.. _doxid-namespacetoolbox_1_1container_1_1string_1adf00945757752e93f4592917811033c5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	levenshtein_distance(
		std::string_view s1,
		std::string_view s2
	)

Calculates the Levenshtein distance between two strings.

The Levenshtein distance is a string metric for measuring the difference between two sequences. It is defined as the minimum number of single-character edits (insertions, deletions, or substitutions) required to change one string into the other.

.. ref-code-block:: cpp

	// Distance between "kitten" and "sitting"
	auto distance = :ref:`levenshtein_distance <doxid-namespacetoolbox_1_1container_1_1string_1adf00945757752e93f4592917811033c5>`("kitten", "sitting");
	// distance will be 3 (k→s, e→i, +g)
	
	// Distance between "book" and "back"
	auto distance2 = :ref:`levenshtein_distance <doxid-namespacetoolbox_1_1container_1_1string_1adf00945757752e93f4592917811033c5>`("book", "back");
	// distance2 will be 2 (o→a, o→c)
	
	// Distance between identical strings
	auto distance3 = :ref:`levenshtein_distance <doxid-namespacetoolbox_1_1container_1_1string_1adf00945757752e93f4592917811033c5>`("same", "same");
	// distance3 will be 0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s1

		- The first string view.

	*
		- s2

		- The second string view.



.. rubric:: Returns:

The Levenshtein distance between s1 and s2 (a non-negative integer).

.. index:: pair: function; longest_common_subsequence_length
.. _doxid-namespacetoolbox_1_1container_1_1string_1a769608e53956a31c6920fdd30d30035b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	longest_common_subsequence_length(
		std::string_view s1,
		std::string_view s2
	)

Calculates the length of the Longest Common Subsequence (LCS) of two strings.

A subsequence is a sequence that appears in the same relative order but not necessarily contiguous. This function finds the length of the longest such sequence common to both input strings.

.. ref-code-block:: cpp

	// LCS of "ABCDEF" and "AUBCDF"
	auto lcs_len = :ref:`longest_common_subsequence_length <doxid-namespacetoolbox_1_1container_1_1string_1a769608e53956a31c6920fdd30d30035b>`("ABCDEF", "AUBCDF");
	// lcs_len will be 4 (ABCD)
	
	// LCS of "AGGTAB" and "GXTXAYB"
	auto lcs_len2 = :ref:`longest_common_subsequence_length <doxid-namespacetoolbox_1_1container_1_1string_1a769608e53956a31c6920fdd30d30035b>`("AGGTAB", "GXTXAYB");
	// lcs_len2 will be 4 (GTAB)
	
	// LCS of completely different strings
	auto lcs_len3 = :ref:`longest_common_subsequence_length <doxid-namespacetoolbox_1_1container_1_1string_1a769608e53956a31c6920fdd30d30035b>`("abc", "xyz");
	// lcs_len3 will be 0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s1

		- The first string view.

	*
		- s2

		- The second string view.



.. rubric:: Returns:

The length of the longest common subsequence.

.. index:: pair: function; longest_common_substring_length
.. _doxid-namespacetoolbox_1_1container_1_1string_1a0b7d1d7d53c01978d757929bb13b204f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	longest_common_substring_length(
		std::string_view s1,
		std::string_view s2
	)

Calculates the length of the Longest Common Substring of two strings.

A substring is a contiguous sequence of characters within a string. This function finds the length of the longest such sequence common to both input strings.

.. ref-code-block:: cpp

	// Longest common substring of "ABABC" and "BABCA"
	auto lcstr_len = :ref:`longest_common_substring_length <doxid-namespacetoolbox_1_1container_1_1string_1a0b7d1d7d53c01978d757929bb13b204f>`("ABABC", "BABCA");
	// lcstr_len will be 4 (BABC)
	
	// Longest common substring of "abcdef" and "zcdemf"
	auto lcstr_len2 = :ref:`longest_common_substring_length <doxid-namespacetoolbox_1_1container_1_1string_1a0b7d1d7d53c01978d757929bb13b204f>`("abcdef", "zcdemf");
	// lcstr_len2 will be 3 (cde)
	
	// Longest common substring of completely different strings
	auto lcstr_len3 = :ref:`longest_common_substring_length <doxid-namespacetoolbox_1_1container_1_1string_1a0b7d1d7d53c01978d757929bb13b204f>`("abc", "xyz");
	// lcstr_len3 will be 0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s1

		- The first string view.

	*
		- s2

		- The second string view.



.. rubric:: Returns:

The length of the longest common substring.

.. index:: pair: function; url_encode
.. _doxid-namespacetoolbox_1_1container_1_1string_1a19556e830ae2bfbbbbe0905a454844f4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	url_encode(std::string_view s)

Percent-encodes a string for safe inclusion in a URL.

This function implements URL encoding (also known as percent-encoding) according to RFC 3986. It encodes all characters except unreserved characters (alphanumeric, '-', '\_', '.', '~'). Spaces are encoded as "%20", and other characters are encoded as XX where XX is the hexadecimal representation of the character's byte value.

.. ref-code-block:: cpp

	// Encoding a simple string
	auto encoded = :ref:`url_encode <doxid-namespacetoolbox_1_1container_1_1string_1a19556e830ae2bfbbbbe0905a454844f4>`("Hello World!");
	// encoded will be "Hello%20World%21"
	
	// Encoding special characters
	auto encoded2 = :ref:`url_encode <doxid-namespacetoolbox_1_1container_1_1string_1a19556e830ae2bfbbbbe0905a454844f4>`("100% true");
	// encoded2 will be "100%25%20true"
	
	// Encoding a URL with query parameters
	auto encoded3 = :ref:`url_encode <doxid-namespacetoolbox_1_1container_1_1string_1a19556e830ae2bfbbbbe0905a454844f4>`("https://example.com/search?q=c++");
	// encoded3 will be "https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The string view to encode.



.. rubric:: Returns:

The URL-encoded string.

.. index:: pair: function; url_decode
.. _doxid-namespacetoolbox_1_1container_1_1string_1ace2f63ba20582b4e048fefeac749de3d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	url_decode(std::string_view s)

Decodes a percent-encoded string (URL decoding).

This function implements URL decoding (percent-decoding) according to RFC

#. It converts XX sequences back to their original characters, where XX is a hexadecimal value. Also handles '+' characters by converting them to spaces, for compatibility with form encoding.

.. ref-code-block:: cpp

	// Decode a simple URL-encoded string
	auto decoded = :ref:`url_decode <doxid-namespacetoolbox_1_1container_1_1string_1ace2f63ba20582b4e048fefeac749de3d>`("Hello%20World%21");
	// decoded will be "Hello World!"
	
	// Decode with form encoding compatibility
	auto decoded2 = :ref:`url_decode <doxid-namespacetoolbox_1_1container_1_1string_1ace2f63ba20582b4e048fefeac749de3d>`("100%25+true");
	// decoded2 will be "100% true"
	
	// Decode a complex URL
	auto decoded3 =
	:ref:`url_decode <doxid-namespacetoolbox_1_1container_1_1string_1ace2f63ba20582b4e048fefeac749de3d>`("https%3A%2F%2Fexample.com%2Fsearch%3Fq%3Dc%2B%2B");
	// decoded3 will be "https://example.com/search?q=c++"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The URL-encoded string view to decode.



.. rubric:: Returns:

The decoded string. Returns an empty string if decoding fails due to invalid % sequences or other encoding errors.

.. index:: pair: function; base64_encode
.. _doxid-namespacetoolbox_1_1container_1_1string_1af2e3a6aeac0c69c43250b792e035e593:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	base64_encode(std::string_view data)

Encodes binary data (represented as a string_view) into Base64 format.

This function implements Base64 encoding according to RFC 4648. It converts binary data into a ASCII string representation using the Base64 alphabet. The output string will be padded with '=' characters if necessary to make its length a multiple of 4.

.. ref-code-block:: cpp

	// Encode simple text
	auto encoded = :ref:`base64_encode <doxid-namespacetoolbox_1_1container_1_1string_1af2e3a6aeac0c69c43250b792e035e593>`("Hello World");
	// encoded will be "SGVsbG8gV29ybGQ="
	
	// Encode binary data
	std::string binary_data = {0x00, 0x01, 0x02, 0x03};
	auto encoded2 = :ref:`base64_encode <doxid-namespacetoolbox_1_1container_1_1string_1af2e3a6aeac0c69c43250b792e035e593>`(binary_data);
	// encoded2 will be "AAECAw=="



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- data

		- The raw binary data to encode, represented as a string_view.



.. rubric:: Returns:

The Base64 encoded string.

.. index:: pair: function; base64_decode
.. _doxid-namespacetoolbox_1_1container_1_1string_1a4267ad272a87495724c13bbb5b607ec5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	base64_decode(std::string_view encoded_data)

Decodes a Base64 encoded string back into its original data.

This function implements Base64 decoding according to RFC 4648. It handles padding characters ('=') and ignores whitespace characters within the input string. The function returns the decoded binary data as a string.

.. ref-code-block:: cpp

	// Decode simple Base64 string
	auto decoded = :ref:`base64_decode <doxid-namespacetoolbox_1_1container_1_1string_1a4267ad272a87495724c13bbb5b607ec5>`("SGVsbG8gV29ybGQ=");
	// decoded will be "Hello World"
	
	// Decode with whitespace
	auto decoded2 = :ref:`base64_decode <doxid-namespacetoolbox_1_1container_1_1string_1a4267ad272a87495724c13bbb5b607ec5>`(" AA EC Aw == ");
	// decoded2 will be "\x00\x01\x02\x03"
	
	// Invalid input returns empty string
	auto decoded3 = :ref:`base64_decode <doxid-namespacetoolbox_1_1container_1_1string_1a4267ad272a87495724c13bbb5b607ec5>`("Invalid!");
	// decoded3 will be ""



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- encoded_data

		- The Base64 encoded string view to decode.



.. rubric:: Returns:

The decoded data as a string. Returns an empty string if decoding fails due to invalid input (e.g., invalid characters, incorrect padding).

.. index:: pair: function; slugify
.. _doxid-namespacetoolbox_1_1container_1_1string_1a5c4d6af2e41f817dca51139247152a02:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	slugify(std::string_view s)

Converts a string into a URL-friendly "slug".

This function transforms a string into a format suitable for use in URLs. It performs the following transformations:

#. Converts the string to lowercase

#. Replaces spaces and consecutive non-alphanumeric characters with a single hyphen

#. Removes other non-alphanumeric characters (except hyphens)

#. Trims leading and trailing hyphens

.. ref-code-block:: cpp

	// Basic slug generation
	auto slug = :ref:`slugify <doxid-namespacetoolbox_1_1container_1_1string_1a5c4d6af2e41f817dca51139247152a02>`("Hello World!");
	// slug will be "hello-world"
	
	// Complex string conversion
	auto slug2 = :ref:`slugify <doxid-namespacetoolbox_1_1container_1_1string_1a5c4d6af2e41f817dca51139247152a02>`("This is a Test -- 123!");
	// slug2 will be "this-is-a-test-123"
	
	// Handling special characters
	auto slug3 = :ref:`slugify <doxid-namespacetoolbox_1_1container_1_1string_1a5c4d6af2e41f817dca51139247152a02>`("C++ Programming 101");
	// slug3 will be "c-programming-101"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- s

		- The input string view to convert.



.. rubric:: Returns:

The generated slug string.


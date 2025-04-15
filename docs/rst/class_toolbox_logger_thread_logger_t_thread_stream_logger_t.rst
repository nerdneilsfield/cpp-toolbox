.. index:: pair: class; toolbox::logger::thread_logger_t::thread_stream_logger_t
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t:

class toolbox::logger::thread_logger_t::thread_stream_logger_t
==============================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Thread-safe stream logger class for formatted output. :ref:`More...<details-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	class thread_stream_logger_t {
	public:
		// construction
	
		:ref:`thread_stream_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ac09791a964a6219aac08a4a1dafcb47b>`(
			:ref:`thread_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t>`& logger,
			:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level,
			:ref:`toolbox::base::object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t>`<std::stringstream>& pool
		);
	
		:target:`thread_stream_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a967d76e5cfb393aab55705a1ba2f8d8a>`(const thread_stream_logger_t&);
		:target:`thread_stream_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a410cba7b9c421e7b81817579c2d86800>`(thread_stream_logger_t&&);
		:ref:`~thread_stream_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a3a0e9c2bfaeed2b9d3a327523fdd9bac>`();

		// methods
	
		thread_stream_logger_t&
		:target:`operator=<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a72c3b2ea467978c6bfe222c59b7721e7>`(const thread_stream_logger_t&);
	
		thread_stream_logger_t&
		:target:`operator=<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1aeeea7cbd9b00b9fefc5a200dc2082f6e>`(thread_stream_logger_t&&);
	
		auto
		:ref:`str<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1aeac33666839401af2bd2bb5bff656b0f>`();
	
		template <typename T>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ab8beb7c8cdf06081cdc0273c48524a5c>`(const T& container);
	
		template <typename... Args>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a6b2e773bb7c208849b9444e00dc209b0>`(const std::tuple<Args...>& t);
	
		auto
		:ref:`red<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a2269dea461b207e1924a6c938271f03f>`(const std::string& text);
	
		auto
		:ref:`green<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a997d5382bd8456b2764ca15eec6c8993>`(const std::string& text);
	
		auto
		:ref:`yellow<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a19f229133a4d635f26db2c57e126a00e>`(const std::string& text);
	
		auto
		:ref:`bold<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a3dabbc117f69a5241f1d58c6db2eca0c>`(const std::string& text);
	
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1af7b0c288f07c785387655dc0c6126055>`(const char* value);
	
		template <typename T>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a4e10c5cd09173cb92ae140ed1706b4e8>`(const T& value);
	
		template <typename T>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a5820a4c7b6b776da403bdad817e5f500>`(T&& value);
	
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a7bc1e3cde55775af3d534d9d3defd103>`(thread_stream_logger_t& logger);
	
		template <
			typename K,
			typename V
		>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1afabe26dba003c7f70ad34e628454ecff>`(const std::map<K, V>& map);
	
		template <
			typename K,
			typename V
		>
		auto
		:ref:`operator<<<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a0778d036f44c3dbcb5ea7752fe1cce38>`(const std::unordered_map<K, V>& map);
	};
.. _details-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Thread-safe stream logger class for formatted output.

The ``:ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>``` class provides a thread-safe mechanism for logging messages using stream operators. It supports various data types including containers, tuples, and C-style strings. The logger automatically handles message formatting and ensures thread safety during logging operations.

.. note::

	This class is typically used through the :ref:`thread_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t>` interface and its associated macros. Direct instantiation is not recommended.



.. ref-code-block:: cpp

	// Basic usage with stream logger
	auto& logger = :ref:`toolbox::logger::thread_logger_t::instance <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`();
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Processing " << items.size() << " items";
	
	// Logging containers
	std::vector<int> values = {1, 2, 3};
	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << "Values: " << values;  // Output: Values: [1, 2, 3]



.. rubric:: See also:

:ref:`thread_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t>`

Construction
------------

.. index:: pair: function; thread_stream_logger_t
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ac09791a964a6219aac08a4a1dafcb47b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	thread_stream_logger_t(
		:ref:`thread_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t>`& logger,
		:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level,
		:ref:`toolbox::base::object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t>`<std::stringstream>& pool
	)

Construct a new thread stream logger object using the pool.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- logger

		- Reference to the parent thread logger.

	*
		- level

		- Logging level for this stream logger.

	*
		- pool

		- Reference to the stringstream object pool.

.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a3a0e9c2bfaeed2b9d3a327523fdd9bac:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	~thread_stream_logger_t()

Destroy the thread stream logger object.

Enqueues the final message. The stringstream is automatically returned to the pool by the ss_ptr\_ destructor.

Methods
-------

.. index:: pair: function; str
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1aeac33666839401af2bd2bb5bff656b0f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	str()

Get the logged message as a string.



.. rubric:: Returns:

The logged message as a string.

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ab8beb7c8cdf06081cdc0273c48524a5c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	auto
	operator<<(const T& container)

Overload operator<< for containers.

This operator allows logging of container types (e.g., vectors, lists) that don't have their own stream operator. The container elements are printed in a comma-separated list enclosed in square brackets.

.. ref-code-block:: cpp

	std::vector<int> vec = {1, 2, 3};
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Vector: " << vec;  // Output: Vector: [1, 2, 3]



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Container type (must satisfy is_container_v)

	*
		- container

		- The container to log



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a6b2e773bb7c208849b9444e00dc209b0:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename... Args>
	auto
	operator<<(const std::tuple<Args...>& t)

Overload operator<< for tuples.

This operator allows logging of std::tuple objects. The tuple elements are printed in a comma-separated list enclosed in parentheses.

.. ref-code-block:: cpp

	auto t = std::make_tuple(1, "test", 3.14);
	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << "Tuple: " << t;  // Output: Tuple: (1, test, 3.14)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Args

		- Types of tuple elements

	*
		- t

		- The tuple to log



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; red
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a2269dea461b207e1924a6c938271f03f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	red(const std::string& text)

Format text in red color.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- text

		- The text to format



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; green
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a997d5382bd8456b2764ca15eec6c8993:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	green(const std::string& text)

Format text in green color.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- text

		- The text to format



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; yellow
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a19f229133a4d635f26db2c57e126a00e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	yellow(const std::string& text)

Format text in yellow color.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- text

		- The text to format



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; bold
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a3dabbc117f69a5241f1d58c6db2eca0c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	bold(const std::string& text)

Format text in bold style.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- text

		- The text to format



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1af7b0c288f07c785387655dc0c6126055:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator<<(const char* value)

Overload operator<< for C-style strings.

This operator allows logging of C-style strings (const char\*). The string is directly written to the log stream.

.. ref-code-block:: cpp

	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "C-style string: " << "Hello World";



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- value

		- The C-style string to log



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a4e10c5cd09173cb92ae140ed1706b4e8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	auto
	operator<<(const T& value)

Overload operator<< for types that have an operator<< with an ostream.

This operator allows logging of any type that supports the standard stream insertion operator (operator<<). It checks the logging level before performing the operation and only logs if the current level is sufficient.

.. ref-code-block:: cpp

	// Logging basic types
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << 42 << " " << 3.14 << " " << true;
	// Output: 42 3.14 true
	
	// Logging custom types
	struct Point { int x, y; };
	std::ostream& :ref:`operator\<\< <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ab8beb7c8cdf06081cdc0273c48524a5c>`(std::ostream& os, const Point& p) {
	    return os << "(" << p.x << "," << p.y << ")";
	}
	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << Point{1, 2};
	// Output: (1,2)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of value to log, must have operator<< defined

	*
		- value

		- The value to print to the log stream



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a5820a4c7b6b776da403bdad817e5f500:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	auto
	operator<<(T&& value)

Overload operator<< for types with member operator<<.

This operator handles types that implement operator<< as a member function rather than a free function. It's particularly useful for types that can't modify the global namespace to add stream operators.

.. ref-code-block:: cpp

	struct Logger {
	    std::ostream& :ref:`operator\<\< <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1ab8beb7c8cdf06081cdc0273c48524a5c>`(std::ostream& os) const {
	        return os << "Logger instance";
	    }
	};
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << Logger();
	// Output: Logger instance



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of value to log, must have member operator<<

	*
		- value

		- The value to print to the log stream



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a7bc1e3cde55775af3d534d9d3defd103:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator<<(thread_stream_logger_t& logger)

Overload operator<< for combining loggers.

This operator allows combining the contents of one logger into another. It's useful for building complex log messages from multiple sources while maintaining the same logging level.

.. ref-code-block:: cpp

	auto& logger1 = :ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Part 1: ";
	auto& logger2 = :ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << "Part 2";
	logger1 << logger2;  // Only logs if INFO level is enabled
	// Output: Part 1: Part 2



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- logger

		- The other logger whose contents to append



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1afabe26dba003c7f70ad34e628454ecff:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename K,
		typename V
	>
	auto
	operator<<(const std::map<K, V>& map)

Overload operator<< for std::map containers.

This operator provides formatted output for std::map containers, displaying key-value pairs in a readable format. The output is similar to Python's dictionary representation.

.. ref-code-block:: cpp

	std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}};
	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << ages;
	// Output: {Alice: 30, Bob: 25}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- K

		- The key type of the map

	*
		- V

		- The value type of the map

	*
		- map

		- The map to print to the log stream



.. rubric:: Returns:

Reference to this logger for chaining

.. index:: pair: function; operator<<
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t_1a0778d036f44c3dbcb5ea7752fe1cce38:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename K,
		typename V
	>
	auto
	operator<<(const std::unordered_map<K, V>& map)

Overload operator<< for std::unordered_map containers.

This operator provides formatted output for std::unordered_map containers. The output format is similar to std::map, but the order of elements may vary due to the unordered nature of the container.

.. ref-code-block:: cpp

	std::unordered_map<int, std::string> ids = {{1, "Alice"}, {2, "Bob"}};
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << ids;
	// Output: {1: Alice, 2: Bob} (order may vary)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- K

		- The key type of the map

	*
		- V

		- The value type of the map

	*
		- map

		- The map to print to the log stream



.. rubric:: Returns:

Reference to this logger for chaining


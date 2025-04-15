.. index:: pair: class; toolbox::logger::thread_logger_t
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t:

class toolbox::logger::thread_logger_t
======================================

.. toctree::
	:hidden:

	enum_toolbox_logger_thread_logger_t_Level.rst
	class_toolbox_logger_thread_logger_t_thread_format_logger_t.rst
	class_toolbox_logger_thread_logger_t_thread_stream_logger_t.rst

Overview
~~~~~~~~

Thread-safe logging class with multiple logging levels and formats. :ref:`More...<details-classtoolbox_1_1logger_1_1thread__logger__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	class thread_logger_t {
	public:
		// enums
	
		enum :ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>`;

		// classes
	
		class :ref:`thread_format_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>`;
		class :ref:`thread_stream_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>`;

		// methods
	
		static
		auto
		:ref:`instance<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`();
	
		static
		void
		:ref:`shutdown<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a19db7b18e06048023d03e9d860d7d52f>`();
	
		auto
		:ref:`level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a8f822fcbcee8aadd60583d97d2aa45b8>`();
	
		auto
		:ref:`level_str<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a767cfaa33472f68fd9536dfc210aec6b>`();
	
		auto
		:ref:`set_level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a52a973b6b75e6278e7b099b8774de23c>`(:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level);
	
		auto
		:ref:`trace_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a87c89bdf54ca24f0ceebfd4597b27a9b>`();
	
		auto
		:ref:`debug_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a2fbc1244fa04b01301c8dfd16a7197ba>`();
	
		auto
		:ref:`info_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acd81a17202ffb123e8ba7fbb33761c01>`();
	
		auto
		:ref:`warn_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a618b9574195ad98c77d18f3e866ca963>`();
	
		auto
		:ref:`error_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ad4846b51ece6ae120a580d0db0fb1942>`();
	
		auto
		:ref:`critical_f<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ae330bc70bd65bf2105c63cde91684385>`();
	
		auto
		:ref:`trace_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ac67a39040494c1559b4b5384dd3ed85f>`();
	
		auto
		:ref:`debug_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a2d37d88931782f4766bafc4830c98b58>`();
	
		auto
		:ref:`info_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ac92b425365d15afbf46ce75bd437c906>`();
	
		auto
		:ref:`warn_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ae2fb595ab6a1b85aae88e387550eaaf9>`();
	
		auto
		:ref:`error_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a3c9390804f6e71bfa4e071392ae46fad>`();
	
		auto
		:ref:`critical_s<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a71c413499ec2090385405577482eaff5>`();
	};
.. _details-classtoolbox_1_1logger_1_1thread__logger__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Thread-safe logging class with multiple logging levels and formats.

The ``:ref:`thread_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t>``` class provides a thread-safe logging mechanism with support for different logging levels (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL). It offers both stream-based and format-based logging capabilities through its nested classes ``:ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>``` and ``:ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>```.

.. note::

	This class implements the singleton pattern and is thread-safe for concurrent logging operations from multiple threads.



.. ref-code-block:: cpp

	// Basic usage with stream logger
	auto& logger = :ref:`toolbox::logger::thread_logger_t::instance <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`();
	logger.set_level(:ref:`toolbox::logger::thread_logger_t::Level::INFO <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a551b723eafd6a31d444fcb2f5920fbd3>`);
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Application started with version " << version;
	
	// Basic usage with format logger
	auto& logger = :ref:`toolbox::logger::thread_logger_t::instance <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`();
	logger.set_level(:ref:`toolbox::logger::thread_logger_t::Level::DEBUG <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1adc30ec20708ef7b0f641ef78b7880a15>`);
	:ref:`LOG_DEBUG_F <doxid-group__LoggingMacros_1ga2eff2cba0a7856857be1df094bebf834>`("Processing {} items in batch {}", item_count, batch_id);



.. rubric:: See also:

:ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>`

:ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>`

Methods
-------

.. index:: pair: function; instance
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	auto
	instance()

Get the singleton instance of the logger.

Uses dynamic allocation and ensures the instance persists until program exit to mitigate issues with static destruction order. Thread-safe initialization.



.. rubric:: Returns:

Reference to the thread-safe logger instance.

.. index:: pair: function; shutdown
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a19db7b18e06048023d03e9d860d7d52f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	void
	shutdown()

Explicitly shuts down the logger instance's worker thread.

Stops the background worker thread. Should be called once before program termination to ensure clean shutdown, especially in complex scenarios like test frameworks.

.. index:: pair: function; level
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a8f822fcbcee8aadd60583d97d2aa45b8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	level()

Get the current logging level.

.. ref-code-block:: cpp

	if (logger.level() <= :ref:`thread_logger_t::Level::DEBUG <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1adc30ec20708ef7b0f641ef78b7880a15>`) {
	    // Perform debug-specific operations
	}



.. rubric:: Returns:

The current logging level as a Level enum value.

.. index:: pair: function; level_str
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a767cfaa33472f68fd9536dfc210aec6b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	level_str()

Get the current logging level as a string.

.. ref-code-block:: cpp

	std::cout << "Current log level: " << logger.level_str() << std::endl;



.. rubric:: Returns:

The current logging level as a human-readable string.

.. index:: pair: function; set_level
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a52a973b6b75e6278e7b099b8774de23c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	set_level(:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level)

Set the logging level.

.. ref-code-block:: cpp

	logger.set_level(:ref:`thread_logger_t::Level::WARN <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a32bd8a1db2275458673903bdb84cb277>`);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- level

		- The new logging level to set.

.. index:: pair: function; trace_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a87c89bdf54ca24f0ceebfd4597b27a9b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	trace_f()

Get a format logger for TRACE level messages.

.. ref-code-block:: cpp

	:ref:`LOG_TRACE_F <doxid-group__LoggingMacros_1ga9909fc8f33cb3e2c2d88d50e3088a0c2>`("Processing item {}", item_id);



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for TRACE level

.. index:: pair: function; debug_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a2fbc1244fa04b01301c8dfd16a7197ba:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	debug_f()

Get a format logger for DEBUG level messages.

.. ref-code-block:: cpp

	:ref:`LOG_DEBUG_F <doxid-group__LoggingMacros_1ga2eff2cba0a7856857be1df094bebf834>`("Received {} bytes of data", data_size);



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for DEBUG level

.. index:: pair: function; info_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acd81a17202ffb123e8ba7fbb33761c01:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	info_f()

Get a format logger for INFO level messages.

.. ref-code-block:: cpp

	:ref:`LOG_INFO_F <doxid-group__LoggingMacros_1ga0d7123d7bfdf773f24ed68b102c7f7c7>`("Application started successfully");



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for INFO level

.. index:: pair: function; warn_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a618b9574195ad98c77d18f3e866ca963:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	warn_f()

Get a format logger for WARN level messages.

.. ref-code-block:: cpp

	:ref:`LOG_WARN_F <doxid-group__LoggingMacros_1ga7ce06e64838465cc3c438e9d99f201b0>`("Disk space is low: {}MB remaining", free_space);



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for WARN level

.. index:: pair: function; error_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ad4846b51ece6ae120a580d0db0fb1942:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	error_f()

Get a format logger for ERROR level messages.

.. ref-code-block:: cpp

	:ref:`LOG_ERROR_F <doxid-group__LoggingMacros_1ga4e4e99c3796c36ea1d541844fb955cd8>`("Failed to process request: {}", error_message);



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for ERROR level

.. index:: pair: function; critical_f
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ae330bc70bd65bf2105c63cde91684385:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	critical_f()

Get a format logger for CRITICAL level messages.

.. ref-code-block:: cpp

	:ref:`LOG_CRITICAL_F <doxid-group__LoggingMacros_1gaaa6f7679d883afb22171314099518ee1>`("System shutdown due to critical error: {}", error);



.. rubric:: Returns:

A :ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>` instance configured for CRITICAL level

.. index:: pair: function; trace_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ac67a39040494c1559b4b5384dd3ed85f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	trace_s()

Get a stream logger for TRACE level messages.

.. ref-code-block:: cpp

	:ref:`LOG_TRACE_S <doxid-group__LoggingMacros_1ga583ca116ff9e9817f4fde393c79d1b6a>` << "Entering function " << __func__;



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for TRACE level

.. index:: pair: function; debug_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a2d37d88931782f4766bafc4830c98b58:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	debug_s()

Get a stream logger for DEBUG level messages.

.. ref-code-block:: cpp

	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << "Processing " << items.size() << " items";



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for DEBUG level

.. index:: pair: function; info_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ac92b425365d15afbf46ce75bd437c906:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	info_s()

Get a stream logger for INFO level messages.

.. ref-code-block:: cpp

	// Log application version info
	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Application started with version " << version;



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for INFO level

.. index:: pair: function; warn_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1ae2fb595ab6a1b85aae88e387550eaaf9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	warn_s()

Get a stream logger for WARN level messages.

.. ref-code-block:: cpp

	// Log unexpected value warning
	:ref:`LOG_WARN_S <doxid-group__LoggingMacros_1gad9b694972a4957493602ba1e7d3c5925>` << "Unexpected value: " << value << " (expected " << expected <<
	")";



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for WARN level

.. index:: pair: function; error_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a3c9390804f6e71bfa4e071392ae46fad:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	error_s()

Get a stream logger for ERROR level messages.

.. ref-code-block:: cpp

	// Log file open error
	:ref:`LOG_ERROR_S <doxid-group__LoggingMacros_1ga869e13d38a14388c77018138eb480ad5>` << "Failed to open file: " << filename;



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for ERROR level

.. index:: pair: function; critical_s
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1a71c413499ec2090385405577482eaff5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	critical_s()

Get a stream logger for CRITICAL level messages.

.. ref-code-block:: cpp

	// Log critical system error
	:ref:`LOG_CRITICAL_S <doxid-group__LoggingMacros_1ga00b710fb8e78bfaa73323b9deb1c43cc>` << "System shutdown due to critical error in " << component;



.. rubric:: Returns:

A :ref:`thread_stream_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__stream__logger__t>` instance configured for CRITICAL level


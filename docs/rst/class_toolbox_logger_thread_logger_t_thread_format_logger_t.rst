.. index:: pair: class; toolbox::logger::thread_logger_t::thread_format_logger_t
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t:

class toolbox::logger::thread_logger_t::thread_format_logger_t
==============================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Format-based logger for printf-style message formatting. :ref:`More...<details-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	class thread_format_logger_t {
	public:
		// construction
	
		:ref:`thread_format_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t_1a96d0edcf890c16cc5b28d230f0ac9d6f>`(
			:ref:`thread_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t>`& logger,
			:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level
		);

		// methods
	
		template <typename... Args>
		void
		:ref:`operator()<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t_1a8ac2b0715b8e5d657fa8c511754e852e>`(
			const char* format,
			Args&&... args
		);
	};
.. _details-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Format-based logger for printf-style message formatting.

The ``:ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t>``` class provides a thread-safe way to log messages using a printf-style format string. It supports format specifiers of the form ``{}`` which are replaced with the provided arguments.

.. note::

	Messages are only logged if their level is equal to or higher than the current logging level.



.. ref-code-block:: cpp

	thread_logger_t::thread_format_logger_t logger(
	    :ref:`thread_logger_t::instance <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`(),
	    :ref:`thread_logger_t::Level::INFO <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a551b723eafd6a31d444fcb2f5920fbd3>`);
	logger("Processing {} of {} items", processed_count, total_items);

Construction
------------

.. index:: pair: function; thread_format_logger_t
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t_1a96d0edcf890c16cc5b28d230f0ac9d6f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	thread_format_logger_t(
		:ref:`thread_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t>`& logger,
		:ref:`Level<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>` level
	)

Construct a format logger with a specific logging level.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- logger

		- Reference to the parent :ref:`thread_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t>` instance

	*
		- level

		- The logging level for messages from this logger

Methods
-------

.. index:: pair: function; operator()
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t_1a8ac2b0715b8e5d657fa8c511754e852e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename... Args>
	void
	operator()(
		const char* format,
		Args&&... args
	)

Log a formatted message.

.. ref-code-block:: cpp

	:ref:`thread_format_logger_t <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1_1thread__format__logger__t_1a96d0edcf890c16cc5b28d230f0ac9d6f>` logger(:ref:`thread_logger_t::instance <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1acc6c92f4fe66ee6d17852eb6b6f393bb>`(),
	    :ref:`thread_logger_t::Level::DEBUG <doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1adc30ec20708ef7b0f641ef78b7880a15>`);
	logger("Processing time: {} ms", elapsed_time);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Args

		- Types of the format arguments

	*
		- format

		- The format string containing ``{}`` placeholders

	*
		- args

		- The values to insert into the format string


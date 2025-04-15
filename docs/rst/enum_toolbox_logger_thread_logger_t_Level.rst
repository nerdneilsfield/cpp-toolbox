.. index:: pair: enum; Level
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1:

enum toolbox::logger::thread_logger_t::Level
============================================

Overview
~~~~~~~~

Enumeration of logging levels. :ref:`More...<details-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1>`

.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>

	enum Level: uint8_t {
	    :ref:`TRACE<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a2d3e4144aa384b18849ab9a8abad74d6>`,
	    :ref:`DEBUG<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1adc30ec20708ef7b0f641ef78b7880a15>`,
	    :ref:`INFO<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a551b723eafd6a31d444fcb2f5920fbd3>`,
	    :ref:`WARN<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a32bd8a1db2275458673903bdb84cb277>`,
	    :ref:`ERROR<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1abb1ca97ec761fc37101737ba0aa2e7c5>`,
	    :ref:`CRITICAL<doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a99cd1c61610c76a57cb8d10d6df6b870>`,
	};

.. _details-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Enumeration of logging levels.

Defines the severity levels for log messages, ordered from least severe (TRACE) to most severe (CRITICAL). Messages with a level below the current logging level will be ignored.

Enum Values
-----------

.. index:: pair: enumvalue; TRACE
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a2d3e4144aa384b18849ab9a8abad74d6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	TRACE

Detailed trace information for debugging.

.. index:: pair: enumvalue; DEBUG
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1adc30ec20708ef7b0f641ef78b7880a15:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	DEBUG

Debug-level information for development.

.. index:: pair: enumvalue; INFO
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a551b723eafd6a31d444fcb2f5920fbd3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	INFO

General operational messages.

.. index:: pair: enumvalue; WARN
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a32bd8a1db2275458673903bdb84cb277:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	WARN

Warning conditions that might need attention.

.. index:: pair: enumvalue; ERROR
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1abb1ca97ec761fc37101737ba0aa2e7c5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	ERROR

Error conditions that need investigation.

.. index:: pair: enumvalue; CRITICAL
.. _doxid-classtoolbox_1_1logger_1_1thread__logger__t_1aac8a891f00498ff98550a300d99f3fa1a99cd1c61610c76a57cb8d10d6df6b870:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CRITICAL

Critical conditions requiring immediate action.


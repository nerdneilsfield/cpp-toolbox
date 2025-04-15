.. index:: pair: struct; toolbox::logger::has_stream_operator
.. _doxid-structtoolbox_1_1logger_1_1has__stream__operator:

template struct toolbox::logger::has_stream_operator
====================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type trait to check if a type has stream insertion operator (<<). :ref:`More...<details-structtoolbox_1_1logger_1_1has__stream__operator>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	template <typename T>
	struct has_stream_operator {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a7fd2e9e81dc24f6c22f29b2d55cf5746>` = decltype(:ref:`test<doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a61c85c408e77c1c79112cbdc2080d01c>`<T>(0))::value;

		// methods
	
		template <typename U>
		static
		auto
		:target:`test<doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a61c85c408e77c1c79112cbdc2080d01c>`(int);
	
		template <typename>
		static
		auto
		:target:`test<doxid-structtoolbox_1_1logger_1_1has__stream__operator_1ab671a9205e8778d4fc969a0871c4d031>`(...);
	};
.. _details-structtoolbox_1_1logger_1_1has__stream__operator:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type trait to check if a type has stream insertion operator (<<).

This template checks if a type can be inserted into an ostream using the stream insertion operator.

.. ref-code-block:: cpp

	// Check if int has stream operator
	static_assert(:ref:`has_stream_operator\<int>::value <doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a7fd2e9e81dc24f6c22f29b2d55cf5746>`, "int has stream operator");
	
	// Check if custom type has stream operator
	struct MyType {};
	static_assert(!:ref:`has_stream_operator\<MyType>::value <doxid-structtoolbox_1_1logger_1_1has__stream__operator_1a7fd2e9e81dc24f6c22f29b2d55cf5746>`, "MyType lacks stream
	operator");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for stream insertion capability.


.. index:: pair: struct; toolbox::logger::has_ostream_method
.. _doxid-structtoolbox_1_1logger_1_1has__ostream__method:

template struct toolbox::logger::has_ostream_method
===================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type trait to check if a type has an ostream method. :ref:`More...<details-structtoolbox_1_1logger_1_1has__ostream__method>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	template <typename T>
	struct has_ostream_method {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1logger_1_1has__ostream__method_1ac4b2319fca9f3e19506d9202c9f7ea69>` = decltype(:ref:`test<doxid-structtoolbox_1_1logger_1_1has__ostream__method_1a7b5596181af724f6c73a7e8285dea8d9>`<T>(0))::value;

		// methods
	
		template <typename U>
		static
		auto
		:target:`test<doxid-structtoolbox_1_1logger_1_1has__ostream__method_1a7b5596181af724f6c73a7e8285dea8d9>`(int);
	
		template <typename>
		static
		auto
		:target:`test<doxid-structtoolbox_1_1logger_1_1has__ostream__method_1a7e0668be09b82991021df38dc8bfe5b0>`(...);
	};
.. _details-structtoolbox_1_1logger_1_1has__ostream__method:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type trait to check if a type has an ostream method.

This template checks if a type has a member operator<< method that can accept an ostream reference.

.. ref-code-block:: cpp

	struct MyType {
	  std::ostream& :ref:`operator\<\< <doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(std::ostream& os) const { return os; }
	};
	static_assert(:ref:`has_ostream_method\<MyType>::value <doxid-structtoolbox_1_1logger_1_1has__ostream__method_1ac4b2319fca9f3e19506d9202c9f7ea69>`, "MyType has ostream
	method");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for ostream method.


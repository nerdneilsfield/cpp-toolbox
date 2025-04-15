.. index:: pair: struct; toolbox::traits::is_callable
.. _doxid-structtoolbox_1_1traits_1_1is__callable:

template struct toolbox::traits::is_callable
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is callable. :ref:`More...<details-structtoolbox_1_1traits_1_1is__callable>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct is_callable: public false_type {
	};
.. _details-structtoolbox_1_1traits_1_1is__callable:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is callable.

Example:

.. ref-code-block:: cpp

	auto lambda = []() { return 42; };
	static_assert(is_callable<decltype(lambda)>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check

	*
		- void

		- SFINAE parameter


.. index:: pair: struct; toolbox::traits::has_size
.. _doxid-structtoolbox_1_1traits_1_1has__size:

template struct toolbox::traits::has_size
=========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type has size member. :ref:`More...<details-structtoolbox_1_1traits_1_1has__size>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct has_size: public false_type {
	};
.. _details-structtoolbox_1_1traits_1_1has__size:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type has size member.

Example:

.. ref-code-block:: cpp

	std::vector<int> v;
	static_assert(has_size<decltype(v)>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check

	*
		- void

		- SFINAE parameter


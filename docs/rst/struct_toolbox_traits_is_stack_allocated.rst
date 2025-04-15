.. index:: pair: struct; toolbox::traits::is_stack_allocated
.. _doxid-structtoolbox_1_1traits_1_1is__stack__allocated:

template struct toolbox::traits::is_stack_allocated
===================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type can be stack allocated. :ref:`More...<details-structtoolbox_1_1traits_1_1is__stack__allocated>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_stack_allocated {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__stack__allocated_1af6b2e4a9533ba3725a95dc934475aca8>` =       :ref:`detail::storage_traits<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits>`<T>::may_be_stack_allocated;
	};
.. _details-structtoolbox_1_1traits_1_1is__stack__allocated:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type can be stack allocated.

Example:

.. ref-code-block:: cpp

	struct Simple {};
	static_assert(is_stack_allocated<Simple>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


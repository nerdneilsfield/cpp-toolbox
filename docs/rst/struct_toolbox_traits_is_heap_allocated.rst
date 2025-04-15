.. index:: pair: struct; toolbox::traits::is_heap_allocated
.. _doxid-structtoolbox_1_1traits_1_1is__heap__allocated:

template struct toolbox::traits::is_heap_allocated
==================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type must be heap allocated. :ref:`More...<details-structtoolbox_1_1traits_1_1is__heap__allocated>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_heap_allocated {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__heap__allocated_1a5b7b65a0c5c2baa9a4361ca4e3b8c9d2>` =       :ref:`detail::storage_traits<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits>`<T>::must_be_heap_allocated;
	};
.. _details-structtoolbox_1_1traits_1_1is__heap__allocated:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type must be heap allocated.

Example:

.. ref-code-block:: cpp

	class Abstract { virtual void foo() = 0; };
	static_assert(is_heap_allocated<Abstract>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


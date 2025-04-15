.. index:: pair: struct; toolbox::traits::is_printable
.. _doxid-structtoolbox_1_1traits_1_1is__printable:

template struct toolbox::traits::is_printable
=============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is printable. :ref:`More...<details-structtoolbox_1_1traits_1_1is__printable>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_printable {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__printable_1af442afce3974ca21174dd393485f88ab>` = decltype(test<T>(0))::value;
	};
.. _details-structtoolbox_1_1traits_1_1is__printable:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is printable.

Example:

.. ref-code-block:: cpp

	static_assert(is_printable<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


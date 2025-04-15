.. index:: pair: struct; toolbox::traits::is_array
.. _doxid-structtoolbox_1_1traits_1_1is__array:

template struct toolbox::traits::is_array
=========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is an array. :ref:`More...<details-structtoolbox_1_1traits_1_1is__array>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_array {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__array_1af7a9656aa37ce070fc295d91d1b8ac51>` = std::is_array_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__array:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is an array.

Example:

.. ref-code-block:: cpp

	static_assert(is_array<int[]>::value);
	static_assert(!is_array<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


.. index:: pair: struct; toolbox::traits::is_pointer
.. _doxid-structtoolbox_1_1traits_1_1is__pointer:

template struct toolbox::traits::is_pointer
===========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is a pointer. :ref:`More...<details-structtoolbox_1_1traits_1_1is__pointer>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_pointer {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__pointer_1a854f4718cba0e6d16e4c48595cb542bd>` = std::is_pointer_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__pointer:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is a pointer.

Example:

.. ref-code-block:: cpp

	static_assert(is_pointer<int*>::value);
	static_assert(!is_pointer<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


.. index:: pair: struct; toolbox::traits::is_reference
.. _doxid-structtoolbox_1_1traits_1_1is__reference:

template struct toolbox::traits::is_reference
=============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is a reference. :ref:`More...<details-structtoolbox_1_1traits_1_1is__reference>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_reference {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__reference_1a2a4e128aa5e70f0901f2b6bb979681c3>` = std::is_reference_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__reference:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is a reference.

Example:

.. ref-code-block:: cpp

	static_assert(is_reference<int&>::value);
	static_assert(!is_reference<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


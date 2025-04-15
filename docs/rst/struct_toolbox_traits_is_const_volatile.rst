.. index:: pair: struct; toolbox::traits::is_const_volatile
.. _doxid-structtoolbox_1_1traits_1_1is__const__volatile:

template struct toolbox::traits::is_const_volatile
==================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is const-volatile qualified. :ref:`More...<details-structtoolbox_1_1traits_1_1is__const__volatile>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_const_volatile {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__const__volatile_1a1c59aa8be1903f3c2ef0a4abead7ee56>` = is_const_v<T>&& is_volatile_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__const__volatile:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is const-volatile qualified.

Example:

.. ref-code-block:: cpp

	static_assert(is_const_volatile<const volatile int>::value);
	static_assert(!is_const_volatile<const int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


.. index:: pair: struct; toolbox::traits::is_const
.. _doxid-structtoolbox_1_1traits_1_1is__const:

template struct toolbox::traits::is_const
=========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is const-qualified. :ref:`More...<details-structtoolbox_1_1traits_1_1is__const>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_const {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__const_1a4d7d3417d66dacd60f9716ef94937a6c>` = std::is_const_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__const:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is const-qualified.

Example:

.. ref-code-block:: cpp

	static_assert(is_const<const int>::value);
	static_assert(!is_const<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


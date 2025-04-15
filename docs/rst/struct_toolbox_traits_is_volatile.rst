.. index:: pair: struct; toolbox::traits::is_volatile
.. _doxid-structtoolbox_1_1traits_1_1is__volatile:

template struct toolbox::traits::is_volatile
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is volatile-qualified. :ref:`More...<details-structtoolbox_1_1traits_1_1is__volatile>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_volatile {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__volatile_1a94bc1c9cd66eafbc0d4b8eb207701949>` = std::is_volatile_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__volatile:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is volatile-qualified.

Example:

.. ref-code-block:: cpp

	static_assert(is_volatile<volatile int>::value);
	static_assert(!is_volatile<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


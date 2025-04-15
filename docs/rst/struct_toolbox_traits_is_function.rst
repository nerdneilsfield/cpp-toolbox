.. index:: pair: struct; toolbox::traits::is_function
.. _doxid-structtoolbox_1_1traits_1_1is__function:

template struct toolbox::traits::is_function
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is a function. :ref:`More...<details-structtoolbox_1_1traits_1_1is__function>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_function {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__function_1af8bb26c6bf025074ee52c1db1e43e18b>` = std::is_function_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__function:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is a function.

Example:

.. ref-code-block:: cpp

	void func();
	static_assert(is_function<decltype(func)>::value);
	static_assert(!is_function<int>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


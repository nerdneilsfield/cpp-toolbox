.. index:: pair: struct; toolbox::traits::is_null_pointer
.. _doxid-structtoolbox_1_1traits_1_1is__null__pointer:

template struct toolbox::traits::is_null_pointer
================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is std::nullptr_t. :ref:`More...<details-structtoolbox_1_1traits_1_1is__null__pointer>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_null_pointer {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__null__pointer_1afacf125110ba5efccaf19915be7b1c1b>` = std::is_null_pointer_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__null__pointer:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is std::nullptr_t.

Example:

.. ref-code-block:: cpp

	static_assert(is_null_pointer<std::nullptr_t>::value);
	static_assert(!is_null_pointer<int*>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


.. index:: pair: struct; toolbox::traits::is_member_pointer
.. _doxid-structtoolbox_1_1traits_1_1is__member__pointer:

template struct toolbox::traits::is_member_pointer
==================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type is a member pointer. :ref:`More...<details-structtoolbox_1_1traits_1_1is__member__pointer>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_member_pointer {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1is__member__pointer_1a5cb7bb2f9cfbacb33dfdbacb6399fe9b>` = std::is_member_pointer_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1is__member__pointer:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type is a member pointer.

Example:

.. ref-code-block:: cpp

	struct S { int m; };
	static_assert(is_member_pointer<int S::*>::value);
	static_assert(!is_member_pointer<int*>::value);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


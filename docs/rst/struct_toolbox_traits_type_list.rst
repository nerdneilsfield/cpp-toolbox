.. index:: pair: struct; toolbox::traits::type_list
.. _doxid-structtoolbox_1_1traits_1_1type__list:

template struct toolbox::traits::type_list
==========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type list container. :ref:`More...<details-structtoolbox_1_1traits_1_1type__list>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename... Ts>
	struct type_list {
		// fields
	
		static constexpr :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` :target:`size<doxid-structtoolbox_1_1traits_1_1type__list_1a2af4cd1c55c598b01a148f6dd27fa2f6>` = sizeof...(Ts);
	};
.. _details-structtoolbox_1_1traits_1_1type__list:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type list container.

Example:

.. ref-code-block:: cpp

	using list = type_list<int, float, double>;
	static_assert(list::size == 3);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Parameter pack of types


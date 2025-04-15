.. index:: pair: struct; toolbox::traits::remove_reference
.. _doxid-structtoolbox_1_1traits_1_1remove__reference:

template struct toolbox::traits::remove_reference
=================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Remove reference from type. :ref:`More...<details-structtoolbox_1_1traits_1_1remove__reference>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct remove_reference {
		// typedefs
	
		typedef std::remove_reference_t<T> :target:`type<doxid-structtoolbox_1_1traits_1_1remove__reference_1a2401fc7fd855e8ee64efdeb8961d3a35>`;
	};
.. _details-structtoolbox_1_1traits_1_1remove__reference:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Remove reference from type.

Example:

.. ref-code-block:: cpp

	using type = remove_reference<int&>::type; // type is int



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to remove reference from


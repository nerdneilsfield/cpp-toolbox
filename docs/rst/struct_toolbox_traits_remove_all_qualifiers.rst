.. index:: pair: struct; toolbox::traits::remove_all_qualifiers
.. _doxid-structtoolbox_1_1traits_1_1remove__all__qualifiers:

template struct toolbox::traits::remove_all_qualifiers
======================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Remove all qualifiers from type. :ref:`More...<details-structtoolbox_1_1traits_1_1remove__all__qualifiers>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct remove_all_qualifiers {
		// typedefs
	
		typedef typename std::remove_cv<typename std::remove_reference<T>:::ref:`type<doxid-structtoolbox_1_1traits_1_1remove__all__qualifiers_1aee52fc7f2cfe82e97e65ebb912439cbb>`>:::ref:`type<doxid-structtoolbox_1_1traits_1_1remove__all__qualifiers_1aee52fc7f2cfe82e97e65ebb912439cbb>` :target:`type<doxid-structtoolbox_1_1traits_1_1remove__all__qualifiers_1aee52fc7f2cfe82e97e65ebb912439cbb>`;
	};
.. _details-structtoolbox_1_1traits_1_1remove__all__qualifiers:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Remove all qualifiers from type.

Example:

.. ref-code-block:: cpp

	using type = remove_all_qualifiers<const int&>::type; // type is int



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to remove qualifiers from


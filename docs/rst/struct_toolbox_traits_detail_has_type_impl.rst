.. index:: pair: struct; toolbox::traits::detail::has_type_impl
.. _doxid-structtoolbox_1_1traits_1_1detail_1_1has__type__impl:

template struct toolbox::traits::detail::has_type_impl
======================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type has a type member. :ref:`More...<details-structtoolbox_1_1traits_1_1detail_1_1has__type__impl>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct has_type_impl: public false_type {
	};
.. _details-structtoolbox_1_1traits_1_1detail_1_1has__type__impl:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type has a type member.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check

	*
		- void

		- SFINAE parameter


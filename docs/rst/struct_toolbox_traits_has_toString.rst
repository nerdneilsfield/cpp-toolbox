.. index:: pair: struct; toolbox::traits::has_toString
.. _doxid-structtoolbox_1_1traits_1_1has__toString:

template struct toolbox::traits::has_toString
=============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type has toString method (pre-C++17 version) :ref:`More...<details-structtoolbox_1_1traits_1_1has__toString>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct has_toString {
		// fields
	
		static constexpr bool :target:`value<doxid-structtoolbox_1_1traits_1_1has__toString_1a42f4c65618ab0ed2dbc2f1d0999acc9c>` = decltype(test<T>(0))::value;
	};
.. _details-structtoolbox_1_1traits_1_1has__toString:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type has toString method (pre-C++17 version)

Example:

.. ref-code-block:: cpp

	class A { std::string toString() { return "A"; } };
	static_assert(has_toString<A>::value, "A has toString");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


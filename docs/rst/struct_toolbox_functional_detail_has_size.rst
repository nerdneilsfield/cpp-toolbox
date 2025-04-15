.. index:: pair: struct; toolbox::functional::detail::has_size
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1has__size:

template struct toolbox::functional::detail::has_size
=====================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type trait to check if type has size() member function. :ref:`More...<details-structtoolbox_1_1functional_1_1detail_1_1has__size>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct has_size: public false_type {
	};
.. _details-structtoolbox_1_1functional_1_1detail_1_1has__size:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type trait to check if type has size() member function.

.. ref-code-block:: cpp

	static_assert(has_size<std::vector<int>>::value, "has size");
	static_assert(!has_size<int>::value, "no size");


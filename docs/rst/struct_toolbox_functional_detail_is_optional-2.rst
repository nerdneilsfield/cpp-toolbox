.. index:: pair: struct; toolbox::functional::detail::is_optional
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1is__optional:

template struct toolbox::functional::detail::is_optional
========================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type trait to check if T is a std::optional. :ref:`More...<details-structtoolbox_1_1functional_1_1detail_1_1is__optional>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <typename T>
	struct is_optional: public false_type {
	};
.. _details-structtoolbox_1_1functional_1_1detail_1_1is__optional:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type trait to check if T is a std::optional.

.. ref-code-block:: cpp

	static_assert(is_optional<std::optional<int>>::value, "is optional");
	static_assert(!is_optional<int>::value, "not optional");


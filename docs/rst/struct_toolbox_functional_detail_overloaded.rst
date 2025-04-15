.. index:: pair: struct; toolbox::functional::detail::overloaded
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1overloaded:

template struct toolbox::functional::detail::overloaded
=======================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Helper class for creating overload sets. :ref:`More...<details-structtoolbox_1_1functional_1_1detail_1_1overloaded>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <class... Fs>
	struct overloaded: public Fs {
	};
.. _details-structtoolbox_1_1functional_1_1detail_1_1overloaded:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Helper class for creating overload sets.

.. ref-code-block:: cpp

	// Create overloaded visitor for variant
	auto visitor = :ref:`overloaded <doxid-namespacetoolbox_1_1functional_1_1detail_1a76f8d30e29e4541774ab82f72157651a>`{
	  [](int x) { return x * 2; },
	  [](const std::string& s) { return s + s; }
	};



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Fs

		- Types of functions to overload


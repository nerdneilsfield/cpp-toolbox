.. index:: pair: struct; toolbox::logger::is_container
.. _doxid-structtoolbox_1_1logger_1_1is__container:

template struct toolbox::logger::is_container
=============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type trait to check if a type is a container. :ref:`More...<details-structtoolbox_1_1logger_1_1is__container>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct is_container: public false_type {
	};
.. _details-structtoolbox_1_1logger_1_1is__container:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type trait to check if a type is a container.

This template checks if a type meets the basic requirements of a container:

* Has a value_type typedef

* Has an iterator typedef

* Has begin() and end() methods

.. ref-code-block:: cpp

	// Check if std::vector is a container
	static_assert(is_container<std::vector<int>>::value, "vector is a
	container");
	
	// Check if int is a container
	static_assert(!is_container<int>::value, "int is not a container");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for container properties.

	*
		- \_

		- SFINAE helper parameter (default void).


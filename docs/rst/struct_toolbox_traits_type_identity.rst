.. index:: pair: struct; toolbox::traits::type_identity
.. _doxid-structtoolbox_1_1traits_1_1type__identity:

template struct toolbox::traits::type_identity
==============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type identity trait. :ref:`More...<details-structtoolbox_1_1traits_1_1type__identity>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct type_identity {
		// typedefs
	
		typedef T :target:`type<doxid-structtoolbox_1_1traits_1_1type__identity_1ad3aa17c897c9aa42de261ad21f61f6b9>`;
	};
.. _details-structtoolbox_1_1traits_1_1type__identity:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type identity trait.

Example:

.. ref-code-block:: cpp

	using type = type_identity<int>::type; // type is int



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to identify


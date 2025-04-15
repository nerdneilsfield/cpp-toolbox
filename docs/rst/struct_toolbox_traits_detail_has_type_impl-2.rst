.. index:: pair: struct; toolbox::traits::detail::has_type_impl<T, void_t<typename T::type>>
.. _doxid-structtoolbox_1_1traits_1_1detail_1_1has__type__impl_3_01T_00_01void__t_3_01typename_01T_1_1type_01_4_01_4:

template struct toolbox::traits::detail::has_type_impl<T, void_t<typename T::type>>
===================================================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Check if type has a type member (specialization) :ref:`More...<details-structtoolbox_1_1traits_1_1detail_1_1has__type__impl_3_01T_00_01void__t_3_01typename_01T_1_1type_01_4_01_4>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct has_type_impl<T, void_t<typename T::type>>: public true_type {
	};
.. _details-structtoolbox_1_1traits_1_1detail_1_1has__type__impl_3_01T_00_01void__t_3_01typename_01T_1_1type_01_4_01_4:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Check if type has a type member (specialization)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to check


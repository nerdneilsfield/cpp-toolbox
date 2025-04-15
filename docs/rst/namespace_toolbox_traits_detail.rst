.. index:: pair: namespace; toolbox::traits::detail
.. _doxid-namespacetoolbox_1_1traits_1_1detail:

namespace toolbox::traits::detail
=================================

.. toctree::
	:hidden:

	struct_toolbox_traits_detail_has_type_impl.rst
	struct_toolbox_traits_detail_has_type_impl-2.rst
	struct_toolbox_traits_detail_storage_traits.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace detail {

	// typedefs

	typedef void :ref:`void_t<doxid-namespacetoolbox_1_1traits_1_1detail_1af947ecd84c1315e86403e59d3508e4f3>`;

	// structs

	template <
		typename T,
		typename = void
	>
	struct :ref:`has_type_impl<doxid-structtoolbox_1_1traits_1_1detail_1_1has__type__impl>`;

	template <typename T>
	struct :ref:`has_type_impl<T, void_t<typename T::type>><doxid-structtoolbox_1_1traits_1_1detail_1_1has__type__impl_3_01T_00_01void__t_3_01typename_01T_1_1type_01_4_01_4>`;

	template <typename T>
	struct :ref:`storage_traits<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits>`;

	} // namespace detail
.. _details-namespacetoolbox_1_1traits_1_1detail:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Typedefs
--------

.. index:: pair: typedef; void_t
.. _doxid-namespacetoolbox_1_1traits_1_1detail_1af947ecd84c1315e86403e59d3508e4f3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef void void_t

Empty type for SFINAE.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ...

		- Variable number of template parameters


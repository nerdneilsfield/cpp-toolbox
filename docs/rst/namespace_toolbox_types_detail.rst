.. index:: pair: namespace; toolbox::types::detail
.. _doxid-namespacetoolbox_1_1types_1_1detail:

namespace toolbox::types::detail
================================

.. toctree::
	:hidden:

	struct_toolbox_types_detail_is_calculable_container.rst
	struct_toolbox_types_detail_is_calculable_container-2.rst




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace detail {

	// structs

	template <
		typename T,
		typename = void
	>
	struct :ref:`is_calculable_container<doxid-structtoolbox_1_1types_1_1detail_1_1is__calculable__container>`;

	template <typename T>
	struct :ref:`is_calculable_container<T, std::void_t<decltype(std::begin(std::declval<const T&>())), decltype(std::end(std::declval<const T&>())), typename T::value_type>><doxid-structtoolbox_1_1types_1_1detail_1_1is__calculable__container_3_01T_00_01std_1_1void__t_3_01decl3a816d9f5967cc0da0667f39f8569f04>`;

	// global variables

	constexpr bool :target:`is_calculable_container_v<doxid-namespacetoolbox_1_1types_1_1detail_1a3739405c0c768be67c2783b27681faf2>` =     :ref:`is_calculable_container<doxid-structtoolbox_1_1types_1_1detail_1_1is__calculable__container>`<T>::value;

	} // namespace detail

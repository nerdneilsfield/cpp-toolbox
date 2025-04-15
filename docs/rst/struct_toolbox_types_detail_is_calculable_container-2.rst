.. index:: pair: struct; toolbox::types::detail::is_calculable_container<T, std::void_t<decltype(std::begin(std::declval<const T&>())), decltype(std::end(std::declval<const T&>())), typename T::value_type>>
.. _doxid-structtoolbox_1_1types_1_1detail_1_1is__calculable__container_3_01T_00_01std_1_1void__t_3_01decl3a816d9f5967cc0da0667f39f8569f04:

template struct toolbox::types::detail::is_calculable_container<T, std::void_t<decltype(std::begin(std::declval<const T&>())), decltype(std::end(std::declval<const T&>())), typename T::value_type>>
=====================================================================================================================================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <minmax.hpp>
	
	template <typename T>
	struct is_calculable_container<T, std::void_t<decltype(std::begin(std::declval<const T&>())), decltype(std::end(std::declval<const T&>())), typename T::value_type>>: public std::bool_constant< !std::is_same_v< std::decay_t< T >, std::string > &&!std::is_same_v< std::decay_t< T >, std::wstring > > {
	};

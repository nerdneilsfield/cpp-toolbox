.. index:: pair: struct; toolbox::utils::detail::has_istream_operator std::declval<T&>
.. _doxid-structtoolbox_1_1utils_1_1detail_1_1has__istream__operator_01std_1_1declval_3_01T_01_6_01_4_07_08_08_4_01_4:

template struct toolbox::utils::detail::has_istream_operator std::declval<T&>
=============================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click_impl.hpp>
	
	template <typename T>
	struct declval<T&>: public std::is_base_of< std::istream, std::remove_reference_t< decltype(std::declval< std::istream & >() >> std::declval< T & >())> > {
	};

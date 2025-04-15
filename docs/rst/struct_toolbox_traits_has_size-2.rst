.. index:: pair: struct; toolbox::traits::has_size<T, std::void_t<decltype(std::declval<T>().size())>>
.. _doxid-structtoolbox_1_1traits_1_1has__size_3_01T_00_01std_1_1void__t_3_01decltype_07std_1_1declval_3_0170b26d93b8d72e0ef44bc54f90f0abc:

template struct toolbox::traits::has_size<T, std::void_t<decltype(std::declval<T>().size())>>
=============================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>: public true_type {
	};

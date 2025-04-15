.. index:: pair: struct; toolbox::traits::is_callable<T, std::void_t<decltype(&T::operator())>>
.. _doxid-structtoolbox_1_1traits_1_1is__callable_3_01T_00_01std_1_1void__t_3_01decltype_07_6T_1_1operator_07_08_08_4_01_4:

template struct toolbox::traits::is_callable<T, std::void_t<decltype(&T::operator())>>
======================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct is_callable<T, std::void_t<decltype(&T::operator())>>: public true_type {
	};

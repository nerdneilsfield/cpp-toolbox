.. index:: pair: struct; toolbox::functional::detail::has_size<T, std::void_t<decltype(std::declval<T>().size())>>
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1has__size_3_01T_00_01std_1_1void__t_3_01decltype_07std_2790a8945b0a7b2e3e0cb919a03cdfe4:

template struct toolbox::functional::detail::has_size<T, std::void_t<decltype(std::declval<T>().size())>>
=========================================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <typename T>
	struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>: public true_type {
	};

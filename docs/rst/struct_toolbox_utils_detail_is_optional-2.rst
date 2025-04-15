.. index:: pair: struct; toolbox::utils::detail::is_optional<std::optional<U>>
.. _doxid-structtoolbox_1_1utils_1_1detail_1_1is__optional_3_01std_1_1optional_3_01U_01_4_01_4:

template struct toolbox::utils::detail::is_optional<std::optional<U>>
=====================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click_impl.hpp>
	
	template <typename U>
	struct is_optional<std::optional<U>>: public true_type {
	};

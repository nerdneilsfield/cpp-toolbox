.. index:: pair: struct; toolbox::functional::detail::is_optional<std::optional<T>>
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1is__optional_3_01std_1_1optional_3_01T_01_4_01_4:

template struct toolbox::functional::detail::is_optional<std::optional<T>>
==========================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <typename T>
	struct is_optional<std::optional<T>>: public true_type {
	};

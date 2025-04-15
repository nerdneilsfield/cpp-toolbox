.. index:: pair: struct; toolbox::types::detail::is_calculable_container
.. _doxid-structtoolbox_1_1types_1_1detail_1_1is__calculable__container:

template struct toolbox::types::detail::is_calculable_container
===============================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <minmax.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct is_calculable_container: public false_type {
	};

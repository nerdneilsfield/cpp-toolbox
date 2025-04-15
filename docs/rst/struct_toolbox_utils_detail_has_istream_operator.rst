.. index:: pair: struct; toolbox::utils::detail::has_istream_operator
.. _doxid-structtoolbox_1_1utils_1_1detail_1_1has__istream__operator:

template struct toolbox::utils::detail::has_istream_operator
============================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click_impl.hpp>
	
	template <
		typename T,
		typename = void
	>
	struct has_istream_operator: public false_type {
	};

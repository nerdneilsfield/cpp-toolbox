.. index:: pair: struct; toolbox::logger::is_container<T, std::void_t<T::value_type, T::iterator, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
.. _doxid-structtoolbox_1_1logger_1_1is__container_3_01T_00_01std_1_1void__t_3_01T_1_1value__type_00_01T_177a661787cdba84b0cbf27d4e0d73025:

template struct toolbox::logger::is_container<T, std::void_t<T::value_type, T::iterator, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
=================================================================================================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_logger.hpp>
	
	template <typename T>
	struct is_container<T, std::void_t<T::value_type, T::iterator, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>: public true_type {
	};

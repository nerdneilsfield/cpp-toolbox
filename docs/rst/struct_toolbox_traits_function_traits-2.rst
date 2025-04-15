.. index:: pair: struct; toolbox::traits::function_traits<R(Args...)>
.. _doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4:

template struct toolbox::traits::function_traits<R(Args...)>
============================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Function traits for regular functions. :ref:`More...<details-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <
		typename R,
		typename... Args
	>
	struct function_traits<R(Args...)> {
		// typedefs
	
		typedef R :target:`return_type<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1a90bf05fe7186e146d5c448f5bc49ad92>`;
		typedef typename std::tuple_element<N, std::tuple<Args...>>::type :target:`arg_type<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1ac1e5d57d1b1e20d04954cee3a7c13044>`;

		// fields
	
		static constexpr :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` :target:`arity<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1af4b192b6770f192d971013be7cd56a57>` = sizeof...(Args);
	};

	// direct descendants

	template <
		typename C,
		typename R,
		typename... Args
	>
	struct :ref:`function_traits<R(C::*)(Args...)><doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4>`;
.. _details-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Function traits for regular functions.

Example:

.. ref-code-block:: cpp

	int func(float, double);
	using traits = function_traits<decltype(func)>;
	static_assert(std::is_same_v<traits::return_type, int>);
	static_assert(traits::arity == 2);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- R

		- Return type

	*
		- Args

		- Function parameter types


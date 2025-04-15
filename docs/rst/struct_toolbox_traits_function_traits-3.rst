.. index:: pair: struct; toolbox::traits::function_traits<R(C::*)(Args...)>
.. _doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4:

template struct toolbox::traits::function_traits<R(C::*)(Args...)>
==================================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Function traits for member functions. :ref:`More...<details-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <
		typename C,
		typename R,
		typename... Args
	>
	struct function_traits<R(C::*)(Args...)>: public :ref:`toolbox::traits::function_traits<R(Args...)><doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4>` {
		// typedefs
	
		typedef C :target:`class_type<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4_1a4d8fb29fc2d916ec04ff4c834074dba9>`;
	};

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
		// typedefs
	
		typedef R :ref:`return_type<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1a90bf05fe7186e146d5c448f5bc49ad92>`;
		typedef typename std::tuple_element<N, std::tuple<Args...>>::type :ref:`arg_type<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1ac1e5d57d1b1e20d04954cee3a7c13044>`;

		// fields
	
		static constexpr :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` :ref:`arity<doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4_1af4b192b6770f192d971013be7cd56a57>` = sizeof...(Args);

.. _details-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Function traits for member functions.

Example:

.. ref-code-block:: cpp

	struct A {
	  int method(float, double);
	};
	using traits = function_traits<decltype(&A::method)>;
	static_assert(std::is_same_v<traits::class_type, A>);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- C

		- Class type

	*
		- R

		- Return type

	*
		- Args

		- Function parameter types


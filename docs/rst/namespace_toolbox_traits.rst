.. index:: pair: namespace; toolbox::traits
.. _doxid-namespacetoolbox_1_1traits:

namespace toolbox::traits
=========================

.. toctree::
	:hidden:

	namespace_toolbox_traits_detail.rst
	struct_toolbox_traits_function_traits.rst
	struct_toolbox_traits_function_traits-2.rst
	struct_toolbox_traits_function_traits-3.rst
	struct_toolbox_traits_has_size.rst
	struct_toolbox_traits_has_size-2.rst
	struct_toolbox_traits_has_toString.rst
	struct_toolbox_traits_is_array.rst
	struct_toolbox_traits_is_callable.rst
	struct_toolbox_traits_is_callable-2.rst
	struct_toolbox_traits_is_const.rst
	struct_toolbox_traits_is_const_volatile.rst
	struct_toolbox_traits_is_function.rst
	struct_toolbox_traits_is_heap_allocated.rst
	struct_toolbox_traits_is_member_pointer.rst
	struct_toolbox_traits_is_null_pointer.rst
	struct_toolbox_traits_is_pointer.rst
	struct_toolbox_traits_is_printable.rst
	struct_toolbox_traits_is_reference.rst
	struct_toolbox_traits_is_stack_allocated.rst
	struct_toolbox_traits_is_volatile.rst
	struct_toolbox_traits_remove_all_qualifiers.rst
	struct_toolbox_traits_remove_reference.rst
	struct_toolbox_traits_type_identity.rst
	struct_toolbox_traits_type_list.rst
	class_toolbox_traits_enum_wrapper.rst




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace traits {

	// namespaces

	namespace :ref:`toolbox::traits::detail<doxid-namespacetoolbox_1_1traits_1_1detail>`;

	// structs

	template <typename T>
	struct :ref:`function_traits<doxid-structtoolbox_1_1traits_1_1function__traits>`;

	template <
		typename R,
		typename... Args
	>
	struct :ref:`function_traits<R(Args...)><doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07Args_8_8_8_08_4>`;

	template <
		typename C,
		typename R,
		typename... Args
	>
	struct :ref:`function_traits<R(C::*)(Args...)><doxid-structtoolbox_1_1traits_1_1function__traits_3_01R_07C_1_1_5_08_07Args_8_8_8_08_4>`;

	template <
		typename T,
		typename = void
	>
	struct :ref:`has_size<doxid-structtoolbox_1_1traits_1_1has__size>`;

	template <typename T>
	struct :ref:`has_size<T, std::void_t<decltype(std::declval<T>().size())>><doxid-structtoolbox_1_1traits_1_1has__size_3_01T_00_01std_1_1void__t_3_01decltype_07std_1_1declval_3_0170b26d93b8d72e0ef44bc54f90f0abc>`;

	template <typename T>
	struct :ref:`has_toString<doxid-structtoolbox_1_1traits_1_1has__toString>`;

	template <typename T>
	struct :ref:`is_array<doxid-structtoolbox_1_1traits_1_1is__array>`;

	template <
		typename T,
		typename = void
	>
	struct :ref:`is_callable<doxid-structtoolbox_1_1traits_1_1is__callable>`;

	template <typename T>
	struct :ref:`is_callable<T, std::void_t<decltype(&T::operator())>><doxid-structtoolbox_1_1traits_1_1is__callable_3_01T_00_01std_1_1void__t_3_01decltype_07_6T_1_1operator_07_08_08_4_01_4>`;

	template <typename T>
	struct :ref:`is_const<doxid-structtoolbox_1_1traits_1_1is__const>`;

	template <typename T>
	struct :ref:`is_const_volatile<doxid-structtoolbox_1_1traits_1_1is__const__volatile>`;

	template <typename T>
	struct :ref:`is_function<doxid-structtoolbox_1_1traits_1_1is__function>`;

	template <typename T>
	struct :ref:`is_heap_allocated<doxid-structtoolbox_1_1traits_1_1is__heap__allocated>`;

	template <typename T>
	struct :ref:`is_member_pointer<doxid-structtoolbox_1_1traits_1_1is__member__pointer>`;

	template <typename T>
	struct :ref:`is_null_pointer<doxid-structtoolbox_1_1traits_1_1is__null__pointer>`;

	template <typename T>
	struct :ref:`is_pointer<doxid-structtoolbox_1_1traits_1_1is__pointer>`;

	template <typename T>
	struct :ref:`is_printable<doxid-structtoolbox_1_1traits_1_1is__printable>`;

	template <typename T>
	struct :ref:`is_reference<doxid-structtoolbox_1_1traits_1_1is__reference>`;

	template <typename T>
	struct :ref:`is_stack_allocated<doxid-structtoolbox_1_1traits_1_1is__stack__allocated>`;

	template <typename T>
	struct :ref:`is_volatile<doxid-structtoolbox_1_1traits_1_1is__volatile>`;

	template <typename T>
	struct :ref:`remove_all_qualifiers<doxid-structtoolbox_1_1traits_1_1remove__all__qualifiers>`;

	template <typename T>
	struct :ref:`remove_reference<doxid-structtoolbox_1_1traits_1_1remove__reference>`;

	template <typename T>
	struct :ref:`type_identity<doxid-structtoolbox_1_1traits_1_1type__identity>`;

	template <typename... Ts>
	struct :ref:`type_list<doxid-structtoolbox_1_1traits_1_1type__list>`;

	// classes

	template <typename EnumType>
	class :ref:`enum_wrapper<doxid-classtoolbox_1_1traits_1_1enum__wrapper>`;

	} // namespace traits

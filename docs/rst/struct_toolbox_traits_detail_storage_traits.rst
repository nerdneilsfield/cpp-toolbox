.. index:: pair: struct; toolbox::traits::detail::storage_traits
.. _doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits:

template struct toolbox::traits::detail::storage_traits
=======================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Storage traits for types. :ref:`More...<details-structtoolbox_1_1traits_1_1detail_1_1storage__traits>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename T>
	struct storage_traits {
		// fields
	
		static constexpr bool :target:`may_be_heap_allocated<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits_1aecf29f202af47a09288819cfdee89dba>` = !std::is_array_v<T>&&(sizeof(T)> 1024 ||             std::has_virtual_destructor_v<T>           ||             std::is_polymorphic_v<T>);
		static constexpr bool :target:`must_be_heap_allocated<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits_1a96a7f89c14d09c3250bfabd60d3f974d>` =       std::is_abstract_v<T>;
		static constexpr bool :target:`may_be_stack_allocated<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits_1a23b09d5db923794336b115df09f4a576>` =       !:ref:`must_be_heap_allocated<doxid-structtoolbox_1_1traits_1_1detail_1_1storage__traits_1a96a7f89c14d09c3250bfabd60d3f974d>`&& std::is_object_v<T>&& !std::is_abstract_v<T>;
	};
.. _details-structtoolbox_1_1traits_1_1detail_1_1storage__traits:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Storage traits for types.

Provides information about storage characteristics of a type:

* Whether it may be heap allocated

* Whether it must be heap allocated

* Whether it may be stack allocated

Example:

.. ref-code-block:: cpp

	class MyClass {};
	bool mayBeHeap = storage_traits<MyClass>::may_be_heap_allocated;
	bool mustBeHeap = storage_traits<MyClass>::must_be_heap_allocated;
	bool mayBeStack = storage_traits<MyClass>::may_be_stack_allocated;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type to analyze


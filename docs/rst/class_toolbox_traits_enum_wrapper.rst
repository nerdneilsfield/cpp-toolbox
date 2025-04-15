.. index:: pair: class; toolbox::traits::enum_wrapper
.. _doxid-classtoolbox_1_1traits_1_1enum__wrapper:

template class toolbox::traits::enum_wrapper
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Type-safe enum wrapper. :ref:`More...<details-classtoolbox_1_1traits_1_1enum__wrapper>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <type_traits.hpp>
	
	template <typename EnumType>
	class enum_wrapper {
	public:
		// typedefs
	
		typedef typename std::underlying_type<EnumType>::type :target:`underlying_type<doxid-classtoolbox_1_1traits_1_1enum__wrapper_1ad4b3d9de5cdfc3f337389c125be7105e>`;

		// construction
	
		:target:`enum_wrapper<doxid-classtoolbox_1_1traits_1_1enum__wrapper_1a9f66051a827fe9001db5d8868f5ac799>`(EnumType value);

		// methods
	
		constexpr
		:target:`operator EnumType<doxid-classtoolbox_1_1traits_1_1enum__wrapper_1aa9d8e09207c4276457f93c532460ce4e>`() const;
	
		constexpr auto
		:target:`to_underlying<doxid-classtoolbox_1_1traits_1_1enum__wrapper_1a7f2e9f8c650a8f5fbcf5562b4179f697>`() const;
	};
.. _details-classtoolbox_1_1traits_1_1enum__wrapper:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Type-safe enum wrapper.

Example:

.. ref-code-block:: cpp

	enum class Color { Red, Green, Blue };
	enum_wrapper<Color> wrapped(Color::Red);
	auto value = wrapped.to_underlying(); // Get underlying value



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- EnumType

		- Enum type to wrap


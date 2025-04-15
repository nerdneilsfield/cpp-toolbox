.. index:: pair: namespace; toolbox::functional::detail
.. _doxid-namespacetoolbox_1_1functional_1_1detail:

namespace toolbox::functional::detail
=====================================

.. toctree::
	:hidden:

	struct_toolbox_functional_detail_MemoizeState.rst
	struct_toolbox_functional_detail_has_size.rst
	struct_toolbox_functional_detail_has_size-2.rst
	struct_toolbox_functional_detail_is_optional.rst
	struct_toolbox_functional_detail_is_optional-2.rst
	struct_toolbox_functional_detail_overloaded.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace detail {

	// structs

	template <
		typename Func,
		typename R,
		typename... Args
	>
	struct :ref:`MemoizeState<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState>`;

	template <
		typename T,
		typename = void
	>
	struct :ref:`has_size<doxid-structtoolbox_1_1functional_1_1detail_1_1has__size>`;

	template <typename T>
	struct :ref:`has_size<T, std::void_t<decltype(std::declval<T>().size())>><doxid-structtoolbox_1_1functional_1_1detail_1_1has__size_3_01T_00_01std_1_1void__t_3_01decltype_07std_2790a8945b0a7b2e3e0cb919a03cdfe4>`;

	template <typename T>
	struct :ref:`is_optional<std::optional<T>><doxid-structtoolbox_1_1functional_1_1detail_1_1is__optional_3_01std_1_1optional_3_01T_01_4_01_4>`;

	template <typename T>
	struct :ref:`is_optional<doxid-structtoolbox_1_1functional_1_1detail_1_1is__optional>`;

	template <class... Fs>
	struct :ref:`overloaded<doxid-structtoolbox_1_1functional_1_1detail_1_1overloaded>`;

	// global variables

	constexpr bool :target:`is_optional_v<doxid-namespacetoolbox_1_1functional_1_1detail_1abb4a0b8e936f718da8749cb8ee3f8f19>` = :ref:`is_optional<doxid-structtoolbox_1_1functional_1_1detail_1_1is__optional>`<std::decay_t<T>>::value;

	// global functions

	template <class... Fs>
	:target:`overloaded<doxid-namespacetoolbox_1_1functional_1_1detail_1a76f8d30e29e4541774ab82f72157651a>`(Fs...);

	template <typename... Containers>
	auto
	:ref:`get_min_size<doxid-namespacetoolbox_1_1functional_1_1detail_1a9f6a4fa1f113a7732b0938e9d173e5ca>`(const Containers&... containers);

	template <
		typename Tuple,
		std::size_t... Is
	>
	void
	:ref:`increment_iterators<doxid-namespacetoolbox_1_1functional_1_1detail_1adbc9b7975526ed027d36367ed670a3c1>`(
		Tuple& iter_tuple,
		std::index_sequence<Is...>
	);

	template <
		typename Tuple,
		std::size_t... Is
	>
	auto
	:ref:`dereference_iterators_as_tuple<doxid-namespacetoolbox_1_1functional_1_1detail_1a9ab9aa30d6377b47498cc0fc70ab0946>`(
		Tuple& iter_tuple,
		std::index_sequence<Is...>
	);

	} // namespace detail
.. _details-namespacetoolbox_1_1functional_1_1detail:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; get_min_size
.. _doxid-namespacetoolbox_1_1functional_1_1detail_1a9f6a4fa1f113a7732b0938e9d173e5ca:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename... Containers>
	auto
	get_min_size(const Containers&... containers)

Helper function to get minimum size from multiple containers.

.. ref-code-block:: cpp

	std::vector<int> v1{1,2,3};
	std::list<int> l1{1,2};
	auto min_size = :ref:`get_min_size <doxid-namespacetoolbox_1_1functional_1_1detail_1a9f6a4fa1f113a7732b0938e9d173e5ca>`(v1, l1); // Returns 2



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Containers

		- Container types



.. rubric:: Returns:

Minimum size among all containers, or 0 if no containers

.. index:: pair: function; increment_iterators
.. _doxid-namespacetoolbox_1_1functional_1_1detail_1adbc9b7975526ed027d36367ed670a3c1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Tuple,
		std::size_t... Is
	>
	void
	increment_iterators(
		Tuple& iter_tuple,
		std::index_sequence<Is...>
	)

Helper function to increment all iterators in a tuple.

.. ref-code-block:: cpp

	auto iters = std::make_tuple(v.begin(), l.begin());
	:ref:`increment_iterators <doxid-namespacetoolbox_1_1functional_1_1detail_1adbc9b7975526ed027d36367ed670a3c1>`(iters,
	std::index_sequence_for<decltype(v),decltype(l)>{});

.. index:: pair: function; dereference_iterators_as_tuple
.. _doxid-namespacetoolbox_1_1functional_1_1detail_1a9ab9aa30d6377b47498cc0fc70ab0946:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Tuple,
		std::size_t... Is
	>
	auto
	dereference_iterators_as_tuple(
		Tuple& iter_tuple,
		std::index_sequence<Is...>
	)

Helper function to dereference iterators and create tuple of references.

.. ref-code-block:: cpp

	auto iters = std::make_tuple(v.begin(), l.begin());
	auto refs = :ref:`dereference_iterators_as_tuple <doxid-namespacetoolbox_1_1functional_1_1detail_1a9ab9aa30d6377b47498cc0fc70ab0946>`(iters,
	std::index_sequence_for<decltype(v),decltype(l)>{});


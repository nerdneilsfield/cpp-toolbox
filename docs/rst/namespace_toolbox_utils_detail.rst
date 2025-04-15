.. index:: pair: namespace; toolbox::utils::detail
.. _doxid-namespacetoolbox_1_1utils_1_1detail:

namespace toolbox::utils::detail
================================

.. toctree::
	:hidden:

	namespace_toolbox_utils_detail_has_istream_operator std.rst
	struct_toolbox_utils_detail_has_istream_operator.rst
	struct_toolbox_utils_detail_is_optional.rst
	struct_toolbox_utils_detail_is_optional-2.rst
	struct_toolbox_utils_detail_optional_value_type.rst
	struct_toolbox_utils_detail_optional_value_type-2.rst




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace detail {

	// namespaces

	namespace :ref:`toolbox::utils::detail::has_istream_operator std<doxid-namespacetoolbox_1_1utils_1_1detail_1_1has__istream__operator_01std>`;

	// typedefs

	typedef typename :ref:`optional_value_type<doxid-structtoolbox_1_1utils_1_1detail_1_1optional__value__type>`<std::remove_cv_t<std::remove_reference_t<T>>>::type :target:`optional_value_type_t<doxid-namespacetoolbox_1_1utils_1_1detail_1a8b4df47a7ef69ad8f4ffde5287318d59>`;

	// structs

	template <
		typename T,
		typename = void
	>
	struct :ref:`has_istream_operator<doxid-structtoolbox_1_1utils_1_1detail_1_1has__istream__operator>`;

	template <typename T>
	struct :ref:`is_optional<doxid-structtoolbox_1_1utils_1_1detail_1_1is__optional>`;

	template <typename U>
	struct :ref:`is_optional<std::optional<U>><doxid-structtoolbox_1_1utils_1_1detail_1_1is__optional_3_01std_1_1optional_3_01U_01_4_01_4>`;

	template <typename T>
	struct :ref:`optional_value_type<doxid-structtoolbox_1_1utils_1_1detail_1_1optional__value__type>`;

	template <typename U>
	struct :ref:`optional_value_type<std::optional<U>><doxid-structtoolbox_1_1utils_1_1detail_1_1optional__value__type_3_01std_1_1optional_3_01U_01_4_01_4>`;

	// global variables

	constexpr bool :target:`is_optional_v<doxid-namespacetoolbox_1_1utils_1_1detail_1ad0ee65cdef7fb964723c52188ece1ee8>` =     :ref:`is_optional<doxid-structtoolbox_1_1utils_1_1detail_1_1is__optional>`<std::remove_cv_t<std::remove_reference_t<T>>>::value;
	constexpr bool :target:`has_istream_operator_v<doxid-namespacetoolbox_1_1utils_1_1detail_1ab6fdd0a876078616752987da5b9c1dfe>` =     :ref:`has_istream_operator<doxid-structtoolbox_1_1utils_1_1detail_1_1has__istream__operator>`<std::remove_cv_t<std::remove_reference_t<T>>>::value;

	} // namespace detail

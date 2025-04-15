.. index:: pair: namespace; toolbox::logger
.. _doxid-namespacetoolbox_1_1logger:

namespace toolbox::logger
=========================

.. toctree::
	:hidden:

	struct_toolbox_logger_has_ostream_method.rst
	struct_toolbox_logger_has_stream_operator.rst
	struct_toolbox_logger_is_container.rst
	struct_toolbox_logger_is_container-2.rst
	class_toolbox_logger_thread_logger_t.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace logger {

	// structs

	template <typename T>
	struct :ref:`has_ostream_method<doxid-structtoolbox_1_1logger_1_1has__ostream__method>`;

	template <typename T>
	struct :ref:`has_stream_operator<doxid-structtoolbox_1_1logger_1_1has__stream__operator>`;

	template <
		typename T,
		typename = void
	>
	struct :ref:`is_container<doxid-structtoolbox_1_1logger_1_1is__container>`;

	template <typename T>
	struct :ref:`is_container<T, std::void_t<T::value_type, T::iterator, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>><doxid-structtoolbox_1_1logger_1_1is__container_3_01T_00_01std_1_1void__t_3_01T_1_1value__type_00_01T_177a661787cdba84b0cbf27d4e0d73025>`;

	// classes

	class :ref:`thread_logger_t<doxid-classtoolbox_1_1logger_1_1thread__logger__t>`;

	// global variables

	constexpr bool :ref:`is_container_v<doxid-namespacetoolbox_1_1logger_1ae92163dce5c70e6094943043c3c76b49>` = :ref:`is_container<doxid-structtoolbox_1_1logger_1_1is__container>`<T>::value;
	constexpr bool :ref:`has_stream_operator_v<doxid-namespacetoolbox_1_1logger_1aaf82a52d321593496c80328501f31622>` = :ref:`has_stream_operator<doxid-structtoolbox_1_1logger_1_1has__stream__operator>`<T>::value;
	constexpr bool :ref:`has_ostream_method_v<doxid-namespacetoolbox_1_1logger_1a69ac1b9f2684929bd3b9f229e15fb322>` = :ref:`has_ostream_method<doxid-structtoolbox_1_1logger_1_1has__ostream__method>`<T>::value;

	} // namespace logger
.. _details-namespacetoolbox_1_1logger:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Variables
----------------

.. index:: pair: variable; is_container_v
.. _doxid-namespacetoolbox_1_1logger_1ae92163dce5c70e6094943043c3c76b49:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	constexpr bool is_container_v = :ref:`is_container<doxid-structtoolbox_1_1logger_1_1is__container>`<T>::value

Helper variable template for :ref:`is_container <doxid-structtoolbox_1_1logger_1_1is__container>` trait.

This provides a more convenient way to access the :ref:`is_container <doxid-structtoolbox_1_1logger_1_1is__container>` trait value.

.. ref-code-block:: cpp

	// Check if std::map is a container
	constexpr bool is_map_container = is_container_v<std::map<int, int>>;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for container properties.

.. index:: pair: variable; has_stream_operator_v
.. _doxid-namespacetoolbox_1_1logger_1aaf82a52d321593496c80328501f31622:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	constexpr bool has_stream_operator_v = :ref:`has_stream_operator<doxid-structtoolbox_1_1logger_1_1has__stream__operator>`<T>::value

Helper variable template for :ref:`has_stream_operator <doxid-structtoolbox_1_1logger_1_1has__stream__operator>` trait.

This provides a more convenient way to access the :ref:`has_stream_operator <doxid-structtoolbox_1_1logger_1_1has__stream__operator>` trait value.

.. ref-code-block:: cpp

	// Check if std::string has stream operator
	constexpr bool can_stream_string = has_stream_operator_v<std::string>;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for stream insertion capability.

.. index:: pair: variable; has_ostream_method_v
.. _doxid-namespacetoolbox_1_1logger_1a69ac1b9f2684929bd3b9f229e15fb322:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	constexpr bool has_ostream_method_v = :ref:`has_ostream_method<doxid-structtoolbox_1_1logger_1_1has__ostream__method>`<T>::value

Helper variable template for :ref:`has_ostream_method <doxid-structtoolbox_1_1logger_1_1has__ostream__method>` trait.

This provides a more convenient way to access the :ref:`has_ostream_method <doxid-structtoolbox_1_1logger_1_1has__ostream__method>` trait value.

.. ref-code-block:: cpp

	struct MyType {};
	constexpr bool has_ostream = has_ostream_method_v<MyType>;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type to check for ostream method.


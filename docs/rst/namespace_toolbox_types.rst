.. index:: pair: namespace; toolbox::types
.. _doxid-namespacetoolbox_1_1types:

namespace toolbox::types
========================

.. toctree::
	:hidden:

	namespace_toolbox_types_detail.rst
	struct_toolbox_types_minmax_t.rst
	struct_toolbox_types_minmax_t-2.rst
	struct_toolbox_types_point_t.rst
	class_toolbox_types_point_cloud_t.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace types {

	// namespaces

	namespace :ref:`toolbox::types::detail<doxid-namespacetoolbox_1_1types_1_1detail>`;

	// structs

	template <typename T>
	struct :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`;

	template <typename T>
	struct :ref:`minmax_t<point_t<T>><doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4>`;

	template <typename T>
	struct :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`;

	// classes

	template <typename T>
	class :ref:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t>`;

	// global functions

	template <typename T>
	:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<T>
	:target:`combine_minmax<doxid-namespacetoolbox_1_1types_1ac0264d34a572654b498427746f2afd8f>`(
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<T>& a,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<T>& b
	);

	template <typename CoordT>
	:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<CoordT>>
	:target:`combine_minmax<doxid-namespacetoolbox_1_1types_1a16eedfcd38f8796afa2fa9f2049578c5>`(
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<CoordT>>& a,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<CoordT>>& b
	);

	template <typename InputType>
	auto
	:target:`calculate_minmax<doxid-namespacetoolbox_1_1types_1ae98c941755e6e52c1054a31008d659d0>`(const InputType& input);

	template <typename T>
	auto
	:target:`calculate_minmax<doxid-namespacetoolbox_1_1types_1af3114944804f22c3a87726419240f074>`(const :ref:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t>`<T>& input);

	template <typename InputType>
	auto
	:target:`calculate_minmax_parallel<doxid-namespacetoolbox_1_1types_1a8e120ade857e793467344ade835c5107>`(const InputType& input);

	template <typename T>
	auto
	:target:`calculate_minmax_parallel<doxid-namespacetoolbox_1_1types_1ae29424ad2561c23ed520d8b921830b8a>`(const :ref:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t>`<T>& input);

	template <typename T>
	auto
	:ref:`operator<<<doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2>`(
		std::ostream& output_stream,
		const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& pt
	);

	template <typename T>
	CPP_TOOLBOX_EXPORT auto
	:ref:`generate_random_points<doxid-namespacetoolbox_1_1types_1a4c77f04fb47f7803330524d59ebff97c>`(
		const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& num_points,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>>& minmax
	);

	template <typename T>
	CPP_TOOLBOX_EXPORT auto
	:ref:`generate_random_points_parallel<doxid-namespacetoolbox_1_1types_1af9fb2b47eed132436e0507ff6dcf73b5>`(
		const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& num_points,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>>& minmax
	);

	} // namespace types
.. _details-namespacetoolbox_1_1types:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; operator<<
.. _doxid-namespacetoolbox_1_1types_1a1249a479c6547ab56be6a1907421ccf2:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	auto
	operator<<(
		std::ostream& output_stream,
		const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& pt
	)

Stream output operator for :ref:`point_t <doxid-structtoolbox_1_1types_1_1point__t>` - Declaration Only.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- output_stream

		- Output stream

	*
		- pt

		- Point to output



.. rubric:: Returns:

Reference to output stream

.. index:: pair: function; generate_random_points
.. _doxid-namespacetoolbox_1_1types_1a4c77f04fb47f7803330524d59ebff97c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	CPP_TOOLBOX_EXPORT auto
	generate_random_points(
		const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& num_points,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>>& minmax
	)

Generates random points within given bounds sequentially.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The coordinate type (e.g., float, double).

	*
		- num_points

		- Total number of points to generate.

	*
		- minmax

		- The minimum and maximum bounds for coordinates.



.. rubric:: Returns:

std::vector<point_t<T>> A vector containing the generated points.

.. index:: pair: function; generate_random_points_parallel
.. _doxid-namespacetoolbox_1_1types_1af9fb2b47eed132436e0507ff6dcf73b5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename T>
	CPP_TOOLBOX_EXPORT auto
	generate_random_points_parallel(
		const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& num_points,
		const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>>& minmax
	)

Generates random points within given bounds in parallel.

Divides the work among threads in the default thread pool. Each thread uses its own random number generator seeded uniquely.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The coordinate type (e.g., float, double).

	*
		- num_points

		- Total number of points to generate.

	*
		- minmax

		- The minimum and maximum bounds for coordinates.



.. rubric:: Returns:

std::vector<point_t<T>> A vector containing the generated points.


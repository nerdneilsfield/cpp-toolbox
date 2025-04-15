.. index:: pair: class; toolbox::types::point_cloud_t
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t:

template class toolbox::types::point_cloud_t
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A point cloud class containing points and associated data. :ref:`More...<details-classtoolbox_1_1types_1_1point__cloud__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <point.hpp>
	
	template <typename T>
	class point_cloud_t: public :ref:`toolbox::io::base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>` {
	public:
		// fields
	
		std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> :ref:`points<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a4b4825f28913d3404e4bbe52164671ff>`;
		std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> :ref:`normals<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a51a93c21f4e4ac87d6356e687047326a>`;
		std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> :ref:`colors<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a190ddb2bc0ad9a8c96e660e0863fe3b8>`;
		T :ref:`intensity<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a705226af021f0d079adc4669de37f073>`;

		// construction
	
		:target:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a478dfcbe07eb70d052519522aad5d03d>`();
		:target:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t_1ab04e06c7252010bd49ca9d3b8fd0e684>`(const point_cloud_t& other);
		:target:`point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a4a48def5ba0ec3fe0bfef3c039cd65b2>`(point_cloud_t&& other);
		:target:`~point_cloud_t<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a766b7179b312f34670a20ceadb1568a1>`();

		// methods
	
		point_cloud_t&
		:target:`operator=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a6f724d45d43a5907207147c52d7d13b2>`(const point_cloud_t& other);
	
		point_cloud_t&
		:target:`operator=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a13942ff83b297a424204d93bf6044f09>`(point_cloud_t&& other);
	
		auto
		:ref:`size<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a929bfd294a28a1f344bf4a6183ccb664>`() const;
	
		auto
		:ref:`empty<doxid-classtoolbox_1_1types_1_1point__cloud__t_1ac8ea56dc4d88223facc3a0fc77e51f0d>`() const;
	
		void
		:ref:`clear<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a41475bb191d4ed9ded2031e64e00b107>`();
	
		void
		:ref:`reserve<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a8059d1b9775fe09ba3d09e81c20deded>`(const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& required_size);
	
		auto
		:ref:`operator+<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a2f4502c2c35feb7484c25a576b8be4d5>`(const point_cloud_t& other) const;
	
		auto
		:ref:`operator+<doxid-classtoolbox_1_1types_1_1point__cloud__t_1ae6db32b8a4fa631904cadb47fceac9de>`(const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& point) const;
	
		auto
		:ref:`operator+<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a8ab660617664fca538f637c4e6f21704>`(:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>&& point) const;
	
		auto
		:ref:`operator+<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a224ee764d8a644f41ee13065a32d8ef6>`(point_cloud_t&& other) const;
	
		point_cloud_t&
		:ref:`operator+=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a4ed32e45c473a7406172aa3ac058989f>`(const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& point);
	
		point_cloud_t&
		:ref:`operator+=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a59d06c4e89cb6acd12ff1160cea74bc1>`(:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>&& point);
	
		point_cloud_t&
		:ref:`operator+=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1a9eed7008d41253e042096bde64a945e7>`(const point_cloud_t& other);
	
		point_cloud_t&
		:ref:`operator+=<doxid-classtoolbox_1_1types_1_1point__cloud__t_1ae5d6a9daaea864ec62c36aec55b3a1f3>`(point_cloud_t&& other);
	};

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
		// methods
	
		:ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`&
		:ref:`operator=<doxid-classtoolbox_1_1io_1_1base__file__data__t_1a0b2dff17aadbdae8d4b390ea5c5d6a00>`(const :ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`&);
	
		:ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`&
		:ref:`operator=<doxid-classtoolbox_1_1io_1_1base__file__data__t_1a7f30dcbe14083eb759409d1ed13f7112>`(:ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`&&);

.. _details-classtoolbox_1_1types_1_1point__cloud__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A point cloud class containing points and associated data.

This class represents a collection of 3D points with optional normals, colors and intensity data.

.. ref-code-block:: cpp

	// Create point cloud
	point_cloud_t<double> cloud;
	
	// Add points
	cloud += point_t<double>(1.0, 2.0, 3.0);
	cloud += point_t<double>(4.0, 5.0, 6.0);
	
	// Access data
	:ref:`std::size_t <doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` num_points = cloud.size();
	point_t<double> first_point = cloud.points[0];
	double :ref:`intensity <doxid-classtoolbox_1_1types_1_1point__cloud__t_1a705226af021f0d079adc4669de37f073>` = cloud.intensity;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The numeric type for point coordinates

Fields
------

.. index:: pair: variable; points
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a4b4825f28913d3404e4bbe52164671ff:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> points

Point coordinates.

.. index:: pair: variable; normals
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a51a93c21f4e4ac87d6356e687047326a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> normals

Point normals (optional)

.. index:: pair: variable; colors
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a190ddb2bc0ad9a8c96e660e0863fe3b8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	std::vector<:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>> colors

Point colors (optional)

.. index:: pair: variable; intensity
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a705226af021f0d079adc4669de37f073:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	T intensity

Global intensity value.

Methods
-------

.. index:: pair: function; size
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a929bfd294a28a1f344bf4a6183ccb664:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	size() const

Get number of points in cloud.



.. rubric:: Returns:

Number of points

.. index:: pair: function; empty
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1ac8ea56dc4d88223facc3a0fc77e51f0d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	empty() const

Check if cloud is empty.



.. rubric:: Returns:

true if empty, false otherwise

.. index:: pair: function; clear
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a41475bb191d4ed9ded2031e64e00b107:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	clear()

Clear all data from cloud.

.. index:: pair: function; reserve
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a8059d1b9775fe09ba3d09e81c20deded:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	reserve(const :ref:`std::size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`& required_size)

Reserve memory for points.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- required_size

		- Number of points to reserve space for

.. index:: pair: function; operator+
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a2f4502c2c35feb7484c25a576b8be4d5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator+(const point_cloud_t& other) const

Add two point clouds.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Cloud to add



.. rubric:: Returns:

New combined cloud

.. index:: pair: function; operator+
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1ae6db32b8a4fa631904cadb47fceac9de:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator+(const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& point) const

Add point to cloud.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- point

		- Point to add



.. rubric:: Returns:

New cloud with added point

.. index:: pair: function; operator+
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a8ab660617664fca538f637c4e6f21704:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator+(:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>&& point) const

Add point to cloud (move version)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- point

		- Point to add



.. rubric:: Returns:

New cloud with added point

.. index:: pair: function; operator+
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a224ee764d8a644f41ee13065a32d8ef6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	operator+(point_cloud_t&& other) const

Add two point clouds (move version)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Cloud to add



.. rubric:: Returns:

New combined cloud

.. index:: pair: function; operator+=
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a4ed32e45c473a7406172aa3ac058989f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_cloud_t&
	operator+=(const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& point)

Add point to cloud.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- point

		- Point to add



.. rubric:: Returns:

Reference to this cloud

.. index:: pair: function; operator+=
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a59d06c4e89cb6acd12ff1160cea74bc1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_cloud_t&
	operator+=(:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>&& point)

Add point to cloud (move version)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- point

		- Point to add



.. rubric:: Returns:

Reference to this cloud

.. index:: pair: function; operator+=
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1a9eed7008d41253e042096bde64a945e7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_cloud_t&
	operator+=(const point_cloud_t& other)

Add another cloud to this one.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Cloud to add



.. rubric:: Returns:

Reference to this cloud

.. index:: pair: function; operator+=
.. _doxid-classtoolbox_1_1types_1_1point__cloud__t_1ae5d6a9daaea864ec62c36aec55b3a1f3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_cloud_t&
	operator+=(point_cloud_t&& other)

Add another cloud to this one (move version)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Cloud to add



.. rubric:: Returns:

Reference to this cloud


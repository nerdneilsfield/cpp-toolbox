.. index:: pair: struct; toolbox::types::point_t
.. _doxid-structtoolbox_1_1types_1_1point__t:

template struct toolbox::types::point_t
=======================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A 3D point/vector template class. :ref:`More...<details-structtoolbox_1_1types_1_1point__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <point.hpp>
	
	template <typename T>
	struct point_t {
		// fields
	
		T :ref:`x<doxid-structtoolbox_1_1types_1_1point__t_1a171efb0a8686791dd6b042babab79ad4>`;
		T :ref:`y<doxid-structtoolbox_1_1types_1_1point__t_1ae84eeb1bacaf6f2aa28321f152f38624>`;
		T :ref:`z<doxid-structtoolbox_1_1types_1_1point__t_1a5697097cf3fe9bed97c2ebc8e9544db6>`;

		// construction
	
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t_1a562974be138dbf613f340cab7b6d37ac>`(
			T x_coord,
			T y_coord,
			T z_coord
		);
	
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t_1a09f13a0dc148b311f1617c36521fd812>`();
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t_1a12ff9bf8c55cc0ec8f15870737ff95f4>`(const point_t& other);
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t_1a0c8d73beda63939951ab7566c6f4e24a>`(point_t&& other);
		:target:`~point_t<doxid-structtoolbox_1_1types_1_1point__t_1a3b6a72a74b2f562ce835d451cdb6a1ac>`();

		// methods
	
		point_t&
		:ref:`operator=<doxid-structtoolbox_1_1types_1_1point__t_1abfbc80bc72eb5e4f9d84129cffaa0269>`(const point_t& other);
	
		point_t&
		:ref:`operator=<doxid-structtoolbox_1_1types_1_1point__t_1a002bac1a6e6cf698e373d2a8ebb2877e>`(point_t&& other);
	
		point_t&
		:ref:`operator+=<doxid-structtoolbox_1_1types_1_1point__t_1a97fb8282a628b0a34838596ccc2e94af>`(const point_t& other);
	
		point_t&
		:ref:`operator-=<doxid-structtoolbox_1_1types_1_1point__t_1a732ad3dcf6da0e98bbf54d65efdb8e56>`(const point_t& other);
	
		point_t&
		:ref:`operator*=<doxid-structtoolbox_1_1types_1_1point__t_1ae688a2e416a93fb2e9c5bd51b21ec37e>`(const T& scalar);
	
		point_t&
		:ref:`operator/=<doxid-structtoolbox_1_1types_1_1point__t_1ab13d63f2c969ca7f9ad14441cd87418e>`(const T& scalar);
	
		auto
		:ref:`dot<doxid-structtoolbox_1_1types_1_1point__t_1abfcdd416b11a3a2292184805782b768f>`(const point_t& other) const;
	
		auto
		:ref:`cross<doxid-structtoolbox_1_1types_1_1point__t_1a1d705d8b2e6e7b688802c2e7894b77dd>`(const point_t& other) const;
	
		auto
		:ref:`norm<doxid-structtoolbox_1_1types_1_1point__t_1a58a06ce4deffa10cbad6747095360c2a>`() const;
	
		auto
		:ref:`normalize<doxid-structtoolbox_1_1types_1_1point__t_1a4fd70b4134e57d8361cc2633f3aeeb71>`() const;
	
		auto
		:ref:`p_norm<doxid-structtoolbox_1_1types_1_1point__t_1a124666311659f8bc42794d2d6bf461d3>`(const double& p_value) const;
	
		auto
		:ref:`p_normalize<doxid-structtoolbox_1_1types_1_1point__t_1a059bc727620c0bb056e80658471e3768>`(const double& p_value) const;
	
		auto
		:ref:`distance<doxid-structtoolbox_1_1types_1_1point__t_1a74fdf288fba57e8ef0f7b598054c98bd>`(const point_t& other) const;
	
		auto
		:ref:`distance_p<doxid-structtoolbox_1_1types_1_1point__t_1acdba49ccc737841c4daf5db008888b91>`(
			const point_t& other,
			const double& p_value
		) const;
	
		bool
		:target:`operator==<doxid-structtoolbox_1_1types_1_1point__t_1a84ffd5da7296d926dadb6a9f72da48e6>`(const point_t& other) const;
	
		bool
		:target:`operator!=<doxid-structtoolbox_1_1types_1_1point__t_1ace9ac2e0e060e0f3073d04b873267b6d>`(const point_t& other) const;
	
		bool
		:target:`operator<<doxid-structtoolbox_1_1types_1_1point__t_1a8e97be2e82223a9967e35f0469c2fa98>`(const point_t& other) const;
	
		auto
		:target:`min_value<doxid-structtoolbox_1_1types_1_1point__t_1ad70da3aa13e710636e217356a7015b85>`();
	
		auto
		:target:`max_value<doxid-structtoolbox_1_1types_1_1point__t_1ab715ac706852e5cafafb2960b6433ffb>`();
	
		auto
		:target:`min_value<doxid-structtoolbox_1_1types_1_1point__t_1a8040f6c5eebe2bc2a1ec128dad701312>`();
	
		auto
		:target:`max_value<doxid-structtoolbox_1_1types_1_1point__t_1a1bb8ea9686c80f160d9e2c903f63395e>`();
	
		static
		auto
		:ref:`min_value<doxid-structtoolbox_1_1types_1_1point__t_1a10b8e4cd3dc68c2772a4cb35e22e5a4d>`();
	
		static
		auto
		:ref:`max_value<doxid-structtoolbox_1_1types_1_1point__t_1aa3b0a411dff3c72d4143a798e3652360>`();
	};
.. _details-structtoolbox_1_1types_1_1point__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A 3D point/vector template class.

This class represents a 3D point or vector with x, y, z coordinates. It provides common vector operations and supports arithmetic operators.

.. ref-code-block:: cpp

	// Create points
	point_t<double> p1(1.0, 2.0, 3.0);
	point_t<double> p2(4.0, 5.0, 6.0);
	
	// Vector operations
	double dot_product = p1.dot(p2);
	point_t<double> cross_product = p1.cross(p2);
	double :ref:`norm <doxid-structtoolbox_1_1types_1_1point__t_1a58a06ce4deffa10cbad6747095360c2a>` = p1.norm();
	
	// Arithmetic
	point_t<double> sum = p1 + p2;
	point_t<double> scaled = p1 * 2.0;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The numeric type for coordinates (e.g. float, double, int)

Fields
------

.. index:: pair: variable; x
.. _doxid-structtoolbox_1_1types_1_1point__t_1a171efb0a8686791dd6b042babab79ad4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	T x

X coordinate.

.. index:: pair: variable; y
.. _doxid-structtoolbox_1_1types_1_1point__t_1ae84eeb1bacaf6f2aa28321f152f38624:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	T y

Y coordinate.

.. index:: pair: variable; z
.. _doxid-structtoolbox_1_1types_1_1point__t_1a5697097cf3fe9bed97c2ebc8e9544db6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	T z

Z coordinate.

Construction
------------

.. index:: pair: function; point_t
.. _doxid-structtoolbox_1_1types_1_1point__t_1a562974be138dbf613f340cab7b6d37ac:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t(
		T x_coord,
		T y_coord,
		T z_coord
	)

Constructs a point with given coordinates.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x_coord

		- X coordinate

	*
		- y_coord

		- Y coordinate

	*
		- z_coord

		- Z coordinate

.. index:: pair: function; point_t
.. _doxid-structtoolbox_1_1types_1_1point__t_1a09f13a0dc148b311f1617c36521fd812:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t()

Default constructor, initializes to origin (0,0,0)

.. index:: pair: function; point_t
.. _doxid-structtoolbox_1_1types_1_1point__t_1a12ff9bf8c55cc0ec8f15870737ff95f4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t(const point_t& other)

Copy constructor.

.. index:: pair: function; point_t
.. _doxid-structtoolbox_1_1types_1_1point__t_1a0c8d73beda63939951ab7566c6f4e24a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t(point_t&& other)

Move constructor.

Methods
-------

.. index:: pair: function; operator=
.. _doxid-structtoolbox_1_1types_1_1point__t_1abfbc80bc72eb5e4f9d84129cffaa0269:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator=(const point_t& other)

Copy assignment.

.. index:: pair: function; operator=
.. _doxid-structtoolbox_1_1types_1_1point__t_1a002bac1a6e6cf698e373d2a8ebb2877e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator=(point_t&& other)

Move assignment.

.. index:: pair: function; operator+=
.. _doxid-structtoolbox_1_1types_1_1point__t_1a97fb8282a628b0a34838596ccc2e94af:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator+=(const point_t& other)

Add another point component-wise.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to add



.. rubric:: Returns:

Reference to this point

.. index:: pair: function; operator-=
.. _doxid-structtoolbox_1_1types_1_1point__t_1a732ad3dcf6da0e98bbf54d65efdb8e56:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator-=(const point_t& other)

Subtract another point component-wise.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to subtract



.. rubric:: Returns:

Reference to this point

.. index:: pair: function; operator*=
.. _doxid-structtoolbox_1_1types_1_1point__t_1ae688a2e416a93fb2e9c5bd51b21ec37e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator*=(const T& scalar)

Multiply coordinates by a scalar.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- scalar

		- Value to multiply by



.. rubric:: Returns:

Reference to this point

.. index:: pair: function; operator/=
.. _doxid-structtoolbox_1_1types_1_1point__t_1ab13d63f2c969ca7f9ad14441cd87418e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	point_t&
	operator/=(const T& scalar)

Divide coordinates by a scalar.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- scalar

		- Value to divide by

	*
		- std::runtime_error

		- if scalar is zero



.. rubric:: Returns:

Reference to this point

.. index:: pair: function; dot
.. _doxid-structtoolbox_1_1types_1_1point__t_1abfcdd416b11a3a2292184805782b768f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	dot(const point_t& other) const

Calculate dot product with another point.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to calculate dot product with



.. rubric:: Returns:

Dot product value

.. index:: pair: function; cross
.. _doxid-structtoolbox_1_1types_1_1point__t_1a1d705d8b2e6e7b688802c2e7894b77dd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	cross(const point_t& other) const

Calculate cross product with another point.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to calculate cross product with



.. rubric:: Returns:

New point representing cross product

.. index:: pair: function; norm
.. _doxid-structtoolbox_1_1types_1_1point__t_1a58a06ce4deffa10cbad6747095360c2a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	norm() const

Calculate Euclidean norm (length) of vector.



.. rubric:: Returns:

Norm as double

.. index:: pair: function; normalize
.. _doxid-structtoolbox_1_1types_1_1point__t_1a4fd70b4134e57d8361cc2633f3aeeb71:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	normalize() const

Return normalized vector (unit length)



.. rubric:: Returns:

Normalized vector as point_t<double>

.. index:: pair: function; p_norm
.. _doxid-structtoolbox_1_1types_1_1point__t_1a124666311659f8bc42794d2d6bf461d3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	p_norm(const double& p_value) const

Calculate p-norm of vector.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- p_value

		- The p value for norm calculation



.. rubric:: Returns:

p-norm as double

.. index:: pair: function; p_normalize
.. _doxid-structtoolbox_1_1types_1_1point__t_1a059bc727620c0bb056e80658471e3768:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	p_normalize(const double& p_value) const

Return p-normalized vector.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- p_value

		- The p value for normalization



.. rubric:: Returns:

p-normalized vector as point_t<double>

.. index:: pair: function; distance
.. _doxid-structtoolbox_1_1types_1_1point__t_1a74fdf288fba57e8ef0f7b598054c98bd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	distance(const point_t& other) const

Calculate Euclidean distance to another point.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to calculate distance to



.. rubric:: Returns:

Distance as double

.. index:: pair: function; distance_p
.. _doxid-structtoolbox_1_1types_1_1point__t_1acdba49ccc737841c4daf5db008888b91:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	distance_p(
		const point_t& other,
		const double& p_value
	) const

Calculate p-distance to another point.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- other

		- Point to calculate distance to

	*
		- p_value

		- The p value for distance calculation



.. rubric:: Returns:

p-distance as double

.. index:: pair: function; min_value
.. _doxid-structtoolbox_1_1types_1_1point__t_1a10b8e4cd3dc68c2772a4cb35e22e5a4d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	auto
	min_value()

Get point with minimum possible values.



.. rubric:: Returns:

Point with minimum coordinates

.. index:: pair: function; max_value
.. _doxid-structtoolbox_1_1types_1_1point__t_1aa3b0a411dff3c72d4143a798e3652360:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	auto
	max_value()

Get point with maximum possible values.



.. rubric:: Returns:

Point with maximum coordinates


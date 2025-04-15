.. index:: pair: class; toolbox::utils::simple_timer_interface_t
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t:

class toolbox::utils::simple_timer_interface_t
==============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Interface for managing a collection of timers. :ref:`More...<details-classtoolbox_1_1utils_1_1simple__timer__interface__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <timer.hpp>
	
	class simple_timer_interface_t {
	public:
		// construction
	
		virtual
		:ref:`~simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a0657a252aa6957989848bc75003e361f>`();

		// methods
	
		virtual
		void
		:ref:`start<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a951cdf306a9e873127dd57d625167ef2>`(int index) = 0;
	
		virtual
		void
		:ref:`stop<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a1c212e40346a2b6bfce50931221fdf0c>`(int index) = 0;
	
		virtual
		void
		:ref:`reset<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ab9ed36ad8db6becea87a78a509c4303c>`(int index) = 0;
	
		virtual
		void
		:ref:`print_stats<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a58d2fadcbf568b35683a66b5806b65b9>`(int index) const = 0;
	
		virtual
		void
		:ref:`print_all_stats<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ad21e5f1ada6f12eb6f0836e985097e4c>`() const = 0;
	
		virtual
		auto
		:ref:`elapsed_time_ms<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ac0a6f61768c354b5e64dd8fd6072c8a5>`(int index) const = 0;
	
		virtual
		auto
		:ref:`elapsed_time<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ae3eae860a75aab4625fd1fc856810b5c>`(int index) const = 0;
	
		virtual
		auto
		:ref:`size<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a4775c89d3a5bf6815d80ccfc5fea8637>`() const = 0;
	};

	// direct descendants

	class :ref:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t>`;
.. _details-classtoolbox_1_1utils_1_1simple__timer__interface__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Interface for managing a collection of timers.

This interface provides a unified way to manage multiple timers through a single interface. It supports starting, stopping, resetting, and querying timers by index.

.. ref-code-block:: cpp

	// Basic usage
	simple_timer_interface_t* timers = create_timer(3);
	timers->start(0);
	// ... perform operation ...
	timers->stop(0);
	double elapsed = timers->elapsed_time_ms(0);
	timers->print_all_stats();
	delete_timer(&timers);

Construction
------------

.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a0657a252aa6957989848bc75003e361f:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	~simple_timer_interface_t()

Virtual destructor to ensure proper cleanup.

Methods
-------

.. index:: pair: function; start
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a951cdf306a9e873127dd57d625167ef2:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	start(int index) = 0

Start the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to start

.. index:: pair: function; stop
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a1c212e40346a2b6bfce50931221fdf0c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	stop(int index) = 0

Stop the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to stop

.. index:: pair: function; reset
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ab9ed36ad8db6becea87a78a509c4303c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	reset(int index) = 0

Reset the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to reset

.. index:: pair: function; print_stats
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a58d2fadcbf568b35683a66b5806b65b9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	print_stats(int index) const = 0

Print statistics for the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to print

.. index:: pair: function; print_all_stats
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ad21e5f1ada6f12eb6f0836e985097e4c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	print_all_stats() const = 0

Print statistics for all timers.

.. index:: pair: function; elapsed_time_ms
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ac0a6f61768c354b5e64dd8fd6072c8a5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	elapsed_time_ms(int index) const = 0

Get elapsed time in milliseconds for specified timer.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to query



.. rubric:: Returns:

Elapsed time in milliseconds

.. index:: pair: function; elapsed_time
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1ae3eae860a75aab4625fd1fc856810b5c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	elapsed_time(int index) const = 0

Get elapsed time in seconds for specified timer.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to query



.. rubric:: Returns:

Elapsed time in seconds

.. index:: pair: function; size
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t_1a4775c89d3a5bf6815d80ccfc5fea8637:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	size() const = 0

Get the number of timers in the collection.



.. rubric:: Returns:

Number of timers


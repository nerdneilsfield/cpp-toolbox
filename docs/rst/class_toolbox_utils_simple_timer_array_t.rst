.. index:: pair: class; toolbox::utils::simple_timer_array_t
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t:

class toolbox::utils::simple_timer_array_t
==========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Concrete implementation of :ref:`simple_timer_interface_t <doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>` using an array of timers. :ref:`More...<details-classtoolbox_1_1utils_1_1simple__timer__array__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <timer.hpp>
	
	class simple_timer_array_t: public :ref:`toolbox::utils::simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>` {
	public:
		// construction
	
		:ref:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a4ff7b3e1baadeb8633aab9cfdef38bb4>`(int size);
		:ref:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a9001933e544e95aa937ff62b6667ad76>`(const std::vector<std::string>& names);
		:target:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a0b0bb53316db65a37b1dbed68896cb1b>`(const simple_timer_array_t&);
		:target:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1aaca1c423e52d9aa62ad4845a6c8bbcd3>`(simple_timer_array_t&&);
		:target:`~simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a893b2b09e37af05f8750680aee0d6546>`();

		// methods
	
		auto
		:target:`operator=<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1aa9f62a89e934f3c82c20fc393631e13e>`(const simple_timer_array_t&);
	
		auto
		:target:`operator=<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a2a752f211001abdcb48278b7e593ef2e>`(simple_timer_array_t&&);
	
		virtual
		void
		:ref:`start<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1ab66c6eef78372b7ec8593bb48512ce9c>`(int index);
	
		virtual
		void
		:ref:`stop<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a5bc4381c9055816bcc3846cc0b1a124d>`(int index);
	
		virtual
		void
		:ref:`reset<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a98bcb5ccd773bf01438c9fe9d5345edb>`(int index);
	
		virtual
		auto
		:ref:`elapsed_time_ms<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a5bfdd8935616beb1edfd57172b418a51>`(int index) const;
	
		virtual
		auto
		:ref:`elapsed_time<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a9f37c360c2a1965b1bebb817570ddcde>`(int index) const;
	
		virtual
		void
		:ref:`print_stats<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1af18d3dac5fd50aebf1f6611166dcfcaa>`(int index) const;
	
		virtual
		void
		:ref:`print_all_stats<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a2f62036ebec41a0d6a635ac371495fa9>`() const;
	
		virtual
		auto
		:ref:`size<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1aa55c1ca9b04efdd34d924d85b889d397>`() const;
	};

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
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

.. _details-classtoolbox_1_1utils_1_1simple__timer__array__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Concrete implementation of :ref:`simple_timer_interface_t <doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>` using an array of timers.

This class implements the :ref:`simple_timer_interface_t <doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>` using a vector of :ref:`stop_watch_timer_t <doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t>` objects. It provides thread-safe operations on multiple timers.

.. ref-code-block:: cpp

	// Basic usage
	simple_timer_array_t timers(3);
	timers.start(0);
	// ... perform operation ...
	timers.stop(0);
	double elapsed = timers.elapsed_time_ms(0);
	timers.print_all_stats();

Construction
------------

.. index:: pair: function; simple_timer_array_t
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a4ff7b3e1baadeb8633aab9cfdef38bb4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	simple_timer_array_t(int size)

Construct a :ref:`simple_timer_array_t <doxid-classtoolbox_1_1utils_1_1simple__timer__array__t>` with specified size.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- size

		- Number of timers to create

.. index:: pair: function; simple_timer_array_t
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a9001933e544e95aa937ff62b6667ad76:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	simple_timer_array_t(const std::vector<std::string>& names)

Construct a :ref:`simple_timer_array_t <doxid-classtoolbox_1_1utils_1_1simple__timer__array__t>` with specified names.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- names

		- Vector of names for each timer

Methods
-------

.. index:: pair: function; start
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1ab66c6eef78372b7ec8593bb48512ce9c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	start(int index)

Start the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to start

.. index:: pair: function; stop
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a5bc4381c9055816bcc3846cc0b1a124d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	stop(int index)

Stop the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to stop

.. index:: pair: function; reset
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a98bcb5ccd773bf01438c9fe9d5345edb:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	reset(int index)

Reset the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to reset

.. index:: pair: function; elapsed_time_ms
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a5bfdd8935616beb1edfd57172b418a51:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	elapsed_time_ms(int index) const

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
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a9f37c360c2a1965b1bebb817570ddcde:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	elapsed_time(int index) const

Get elapsed time in seconds for specified timer.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to query



.. rubric:: Returns:

Elapsed time in seconds

.. index:: pair: function; print_stats
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1af18d3dac5fd50aebf1f6611166dcfcaa:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	print_stats(int index) const

Print statistics for the timer at specified index.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- index

		- The index of the timer to print

.. index:: pair: function; print_all_stats
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1a2f62036ebec41a0d6a635ac371495fa9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	void
	print_all_stats() const

Print statistics for all timers.

.. index:: pair: function; size
.. _doxid-classtoolbox_1_1utils_1_1simple__timer__array__t_1aa55c1ca9b04efdd34d924d85b889d397:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	virtual
	auto
	size() const

Get the number of timers in the collection.



.. rubric:: Returns:

Number of timers


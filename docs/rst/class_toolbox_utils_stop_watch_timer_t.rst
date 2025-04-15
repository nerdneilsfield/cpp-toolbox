.. index:: pair: class; toolbox::utils::stop_watch_timer_t
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t:

class toolbox::utils::stop_watch_timer_t
========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A high-resolution stopwatch timer for measuring elapsed time. :ref:`More...<details-classtoolbox_1_1utils_1_1stop__watch__timer__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <timer.hpp>
	
	class stop_watch_timer_t {
	public:
		// construction
	
		:ref:`stop_watch_timer_t<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1ab869b3ea0d3f52a0589b848fb67c2a10>`(std::string name = "default_timer");

		// methods
	
		void
		:ref:`set_name<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1abcc82e718432abb8d70fd26af4999a61>`(const std::string& name);
	
		auto
		:ref:`get_name<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a6d1e2b7f98c31a20a980316f79cd7fdf>`() const;
	
		void
		:ref:`start<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1adf71d3b113d638712ca9373f0d01c5fd>`();
	
		void
		:ref:`stop<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a9dfddbc28505fdd2d4cd413daec64728>`();
	
		void
		:ref:`reset<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1af18d3c15931cba7c7312c89c6a166f62>`();
	
		auto
		:ref:`elapsed_time_ms<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a0c0cae7d727ebd26ad2912e2c6c28cbe>`() const;
	
		auto
		:ref:`elapsed_time<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1aba0a1f15bef0d2b71a2cc467b434ca6e>`() const;
	
		void
		:ref:`print_stats<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a07dc549398de34042a0e0429560140da>`() const;
	};
.. _details-classtoolbox_1_1utils_1_1stop__watch__timer__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A high-resolution stopwatch timer for measuring elapsed time.

This class provides a simple interface for measuring elapsed time with nanosecond precision. It supports starting, stopping, and resetting the timer, as well as retrieving elapsed time in milliseconds or seconds.

.. ref-code-block:: cpp

	// Basic usage
	stop_watch_timer_t timer("MyTimer");
	timer.start();
	// ... perform some operations ...
	timer.stop();
	double elapsed_ms = timer.elapsed_time_ms();
	timer.print_stats();
	
	// Multiple start/stop cycles
	stop_watch_timer_t timer;
	timer.start();
	// ... first operation ...
	timer.stop();
	timer.start();
	// ... second operation ...
	timer.stop();
	double total_seconds = timer.elapsed_time();

Construction
------------

.. index:: pair: function; stop_watch_timer_t
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1ab869b3ea0d3f52a0589b848fb67c2a10:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	stop_watch_timer_t(std::string name = "default_timer")

Construct a new :ref:`stop_watch_timer_t <doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t>` object.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- name

		- The name of the timer (default: "default_timer")

Methods
-------

.. index:: pair: function; set_name
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1abcc82e718432abb8d70fd26af4999a61:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	set_name(const std::string& name)

Set the name of the timer.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- name

		- The new name for the timer

.. index:: pair: function; get_name
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a6d1e2b7f98c31a20a980316f79cd7fdf:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	get_name() const

Get the name of the timer.



.. rubric:: Returns:

const std::string& The current name of the timer

.. index:: pair: function; start
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1adf71d3b113d638712ca9373f0d01c5fd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	start()

Start or resume the timer.

.. note::

	If the timer is already running, this will restart the timing

.. index:: pair: function; stop
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a9dfddbc28505fdd2d4cd413daec64728:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	stop()

Stop (pause) the timer and accumulate the duration.

.. note::

	If the timer is not running, this function does nothing

.. index:: pair: function; reset
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1af18d3c15931cba7c7312c89c6a166f62:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	reset()

Reset the timer to zero.

.. note::

	This also stops the timer if it was running

.. index:: pair: function; elapsed_time_ms
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a0c0cae7d727ebd26ad2912e2c6c28cbe:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	elapsed_time_ms() const

Get the total elapsed time in milliseconds.

.. note::

	This includes all accumulated time between start/stop cycles



.. rubric:: Returns:

double The elapsed time in milliseconds

.. index:: pair: function; elapsed_time
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1aba0a1f15bef0d2b71a2cc467b434ca6e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	elapsed_time() const

Get the total elapsed time in seconds.

.. note::

	This includes all accumulated time between start/stop cycles



.. rubric:: Returns:

double The elapsed time in seconds

.. index:: pair: function; print_stats
.. _doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t_1a07dc549398de34042a0e0429560140da:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	print_stats() const

Print the timer statistics to standard output.

Prints the timer name and elapsed time in a human-readable format


.. index:: pair: class; toolbox::container::concurrent_queue_t
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t:

template class toolbox::container::concurrent_queue_t
=====================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A wrapper around a high-performance MPMC concurrent queue. :ref:`More...<details-classtoolbox_1_1container_1_1concurrent__queue__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <concurrent_queue.hpp>
	
	template <typename T>
	class concurrent_queue_t {
	public:
		// construction
	
		:ref:`~concurrent_queue_t<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a94525706908c0f9cf4c5821a1a591533>`();

		// methods
	
		void
		:ref:`enqueue<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1ad8b857daac08b48651621c00feff4a3d>`(T&& value);
	
		bool
		:ref:`try_dequeue<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a6a845607470339dbca7081c0cbe626e0>`(T& item);
	
		std::optional<T>
		:ref:`try_dequeue<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1abbfd8f2281592767cb7ce1b6116818b7>`();
	
		bool
		:ref:`wait_dequeue_timed<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a35bc2569336e065de2197b568e0d041e>`(
			T& item,
			std::chrono::microseconds timeout
		);
	
		:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`
		:ref:`size_approx<doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1ac2ad05e78bebe8ad3738e7a7f70cbe7c>`() const;
	};
.. _details-classtoolbox_1_1container_1_1concurrent__queue__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A wrapper around a high-performance MPMC concurrent queue.

This class provides a stable interface while hiding the underlying third-party library implementation details using the Pimpl idiom.

.. ref-code-block:: cpp

	// Create a concurrent queue for integers
	concurrent_queue_t<int> queue;
	
	// Producer thread
	queue.enqueue(42);
	
	// Consumer thread
	int value;
	if(queue.try_dequeue(value)) {
	  // Process value
	}
	
	// Alternative consumer using optional
	if(auto opt = queue.try_dequeue()) {
	  int value = *opt;
	  // Process value
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of elements stored in the queue. Must be movable.

Construction
------------

.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a94525706908c0f9cf4c5821a1a591533:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	~concurrent_queue_t()

Destroys the queue wrapper and cleans up resources.

.. note::

	Must be defined in the .cpp file due to Pimpl idiom with unique_ptr.

Methods
-------

.. index:: pair: function; enqueue
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1ad8b857daac08b48651621c00feff4a3d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	enqueue(T&& value)

Enqueues an item into the queue. Thread-safe for multiple producers.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- value

		- The value to enqueue (will be moved).

.. index:: pair: function; try_dequeue
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a6a845607470339dbca7081c0cbe626e0:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	bool
	try_dequeue(T& item)

Attempts to dequeue an item from the queue (non-blocking). Thread-safe for multiple consumers.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- item

		- Reference to store the dequeued value if successful.



.. rubric:: Returns:

True if an item was successfully dequeued, false if the queue was empty.

.. index:: pair: function; try_dequeue
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1abbfd8f2281592767cb7ce1b6116818b7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	std::optional<T>
	try_dequeue()

Attempts to dequeue an item, returning it in an std::optional (non-blocking). Thread-safe for multiple consumers.



.. rubric:: Returns:

std::optional<T> containing the dequeued value if successful, or std::nullopt if the queue was empty.

.. index:: pair: function; wait_dequeue_timed
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1a35bc2569336e065de2197b568e0d041e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	bool
	wait_dequeue_timed(
		T& item,
		std::chrono::microseconds timeout
	)

Attempts to dequeue an item, blocking until an item is available or the specified timeout elapses (non-blocking). Thread-safe for multiple consumers.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- item

		- Reference to store the dequeued value if successful.

	*
		- timeout

		- The maximum duration to wait.



.. rubric:: Returns:

True if an item was successfully dequeued within the timeout, false otherwise.

.. index:: pair: function; size_approx
.. _doxid-classtoolbox_1_1container_1_1concurrent__queue__t_1ac2ad05e78bebe8ad3738e7a7f70cbe7c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`
	size_approx() const

Returns an approximate count of items in the queue. Useful for heuristics but may not be exact in a highly concurrent scenario.



.. rubric:: Returns:

Approximate number of items in the queue.


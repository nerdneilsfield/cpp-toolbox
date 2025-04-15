.. index:: pair: class; toolbox::container::lock_free_queue_t
.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t:

template class toolbox::container::lock_free_queue_t
====================================================

.. toctree::
	:hidden:

	struct_toolbox_container_lock_free_queue_t_Node.rst

Overview
~~~~~~~~

An MPMC lock-free unbounded queue using Hazard Pointers for memory safety. :ref:`More...<details-classtoolbox_1_1container_1_1lock__free__queue__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <lock_free_queue.hpp>
	
	template <typename T>
	class lock_free_queue_t {
	public:
		// structs
	
		struct :ref:`Node<doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node>`;

		// construction
	
		:ref:`~lock_free_queue_t<doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a88f67f01833759462e4145109491137b>`();

		// methods
	
		void
		:ref:`enqueue<doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a04fe5aaa9bf0cead6b5541ddcbeff786>`(T value);
	
		bool
		:ref:`try_dequeue<doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a1afe385f50759fd6bf94b47749440153>`(T& result);
	
		std::optional<T>
		:ref:`try_dequeue<doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1aa773c3783f00fdb4de841e7c62cfb588>`();
	
		static
		void
		:ref:`cleanup_this_thread_retired_nodes<doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1abf1d4659e14a669d5f571c453e7a6d66>`();
	};
.. _details-classtoolbox_1_1container_1_1lock__free__queue__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

An MPMC lock-free unbounded queue using Hazard Pointers for memory safety.

Based on the Michael & Scott algorithm. Uses raw pointers, atomic operations, and Hazard Pointers for safe memory reclamation.

.. note::

	This implementation uses a simplified Hazard Pointer scheme. Production use might require a more robust and optimized HP library. The HP registration part might involve locks, slightly impacting the "pure" lock-free nature for thread joining/leaving, but core queue operations remain lock-free.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of elements stored in the queue. Must be movable and default constructible.

Construction
------------

.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a88f67f01833759462e4145109491137b:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	~lock_free_queue_t()

Destroys the lock-free queue.

Deletes remaining nodes. Assumes no concurrent access during destruction. Attempts to clean up nodes retired by the calling thread.

.. warning::

	Proper distributed cleanup of retired nodes across all threads is complex and not fully handled here. May leak memory if other threads exit uncleanly or don't call cleanup routines.

Methods
-------

.. index:: pair: function; enqueue
.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a04fe5aaa9bf0cead6b5541ddcbeff786:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	enqueue(T value)

Enqueues an item into the queue.

Thread-safe for multiple producers. Uses Hazard Pointers implicitly if needed for tail reads (though M&S enqueue typically doesn't require HPs).



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- value

		- The value to enqueue (will be moved).

.. index:: pair: function; try_dequeue
.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1a1afe385f50759fd6bf94b47749440153:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	bool
	try_dequeue(T& result)

Attempts to dequeue an item from the queue using Hazard Pointers.

Thread-safe for multiple consumers. Uses Hazard Pointers to protect access to head and head->next nodes before potential retirement. Non-blocking operation.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- result

		- Reference to store the dequeued value if successful.



.. rubric:: Returns:

True if an item was successfully dequeued, false if the queue was empty.

.. index:: pair: function; try_dequeue
.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1aa773c3783f00fdb4de841e7c62cfb588:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	std::optional<T>
	try_dequeue()

Attempts to dequeue an item, returning it in an std::optional.

Uses the Hazard Pointer based try_dequeue implementation.



.. rubric:: Returns:

std::optional<T> containing the dequeued value if successful, or std::nullopt if the queue was empty.

.. index:: pair: function; cleanup_this_thread_retired_nodes
.. _doxid-classtoolbox_1_1container_1_1lock__free__queue__t_1abf1d4659e14a669d5f571c453e7a6d66:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	void
	cleanup_this_thread_retired_nodes()

Performs cleanup of retired nodes for the calling thread.

This function should ideally be called by each thread that used the queue before the thread exits, to ensure timely reclamation of memory retired by that thread. It attempts to scan and delete any nodes in the calling thread's retired list that are no longer protected by any hazard pointers.

.. note::

	This only cleans the calling thread's list. Memory retired by other threads that have exited without cleaning their lists might not be reclaimed until this thread (or another active thread) retires enough nodes to trigger a scan that happens to collect enough hazard pointers globally. A more robust global cleanup mechanism (e.g., dedicated cleanup thread, global retired list) is not implemented in this simplified version.


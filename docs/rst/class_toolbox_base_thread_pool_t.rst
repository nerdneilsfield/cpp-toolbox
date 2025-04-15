.. index:: pair: class; toolbox::base::thread_pool_t
.. _doxid-classtoolbox_1_1base_1_1thread__pool__t:

class toolbox::base::thread_pool_t
==================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A high-performance C++17 thread pool implementation using moodycamel::ConcurrentQueue (via wrapper). :ref:`More...<details-classtoolbox_1_1base_1_1thread__pool__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_pool.hpp>
	
	class thread_pool_t {
	public:
		// construction
	
		:ref:`thread_pool_t<doxid-classtoolbox_1_1base_1_1thread__pool__t_1ae08a2a6cdf82861586256335e54c5991>`(:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` threads = 0);
		:ref:`~thread_pool_t<doxid-classtoolbox_1_1base_1_1thread__pool__t_1a95a99126dbc2f85da807b08e7ab5566d>`();

		// methods
	
		template <
			class F,
			class... Args
		>
		auto
		:ref:`submit<doxid-classtoolbox_1_1base_1_1thread__pool__t_1a4d11cc8664186d7ddb3fd5cc8124aa53>`(
			F&& f,
			Args&&... args
		);
	};
.. _details-classtoolbox_1_1base_1_1thread__pool__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A high-performance C++17 thread pool implementation using moodycamel::ConcurrentQueue (via wrapper).

This thread pool allows submitting tasks and asynchronously retrieving results. It creates a fixed number of worker threads upon construction and gracefully stops them during destruction. Uses ``:ref:`toolbox::container::concurrent_queue_t <doxid-classtoolbox_1_1container_1_1concurrent__queue__t>``` as the underlying task queue.

.. ref-code-block:: cpp

	// Create a thread pool with default number of threads
	:ref:`thread_pool_t <doxid-classtoolbox_1_1base_1_1thread__pool__t_1ae08a2a6cdf82861586256335e54c5991>` pool;
	
	// Submit a simple task
	auto future = pool.submit([]() { return 42; });
	int result = future.get(); // result will be 42
	
	// Submit a task with arguments
	auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
	int result2 = future2.get(); // result2 will be 30

Construction
------------

.. index:: pair: function; thread_pool_t
.. _doxid-classtoolbox_1_1base_1_1thread__pool__t_1ae08a2a6cdf82861586256335e54c5991:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	thread_pool_t(:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` threads = 0)

Constructs and initializes the thread pool.

.. ref-code-block:: cpp

	// Create pool with 4 threads
	:ref:`thread_pool_t <doxid-classtoolbox_1_1base_1_1thread__pool__t_1ae08a2a6cdf82861586256335e54c5991>` pool(4);
	
	// Create pool with default number of threads
	:ref:`thread_pool_t <doxid-classtoolbox_1_1base_1_1thread__pool__t_1ae08a2a6cdf82861586256335e54c5991>` :ref:`default_pool <doxid-namespacetoolbox_1_1concurrent_1ad83682a69d167e3b7f8712566033e180>`;



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- threads

		- Number of worker threads to create. Defaults to hardware concurrency. If hardware concurrency cannot be determined or is 0, creates at least 1 thread.

	*
		- std::invalid_argument

		- if thread count parameter is 0.

.. _doxid-classtoolbox_1_1base_1_1thread__pool__t_1a95a99126dbc2f85da807b08e7ab5566d:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	~thread_pool_t()

Destructor that stops the thread pool and waits for all worker threads to finish.

Waits for tasks already dequeued by threads to complete. After destruction, the thread pool becomes unusable.

Methods
-------

.. index:: pair: function; submit
.. _doxid-classtoolbox_1_1base_1_1thread__pool__t_1a4d11cc8664186d7ddb3fd5cc8124aa53:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		class F,
		class... Args
	>
	auto
	submit(
		F&& f,
		Args&&... args
	)

Submits a task to the thread pool for execution.

.. ref-code-block:: cpp

	// Submit a lambda task
	auto future = pool.submit([]() { return 42; });
	
	// Submit a task with arguments
	auto future2 = pool.submit([](int a, int b) { return a + b; }, 10, 20);
	
	// Submit a task that throws an exception
	auto future3 = pool.submit([]() { throw std::runtime_error("error"); });
	try {
	    future3.get();
	} catch (const std::exception& e) {
	    // Handle exception
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F

		- Type of the task function.

	*
		- Args

		- Types of the task function arguments.

	*
		- f

		- Task function (can be function pointer, lambda, std::function, or any callable object).

	*
		- args

		- Arguments to pass to the task function.

	*
		- std::runtime_error

		- if the thread pool has been stopped and cannot accept new tasks.



.. rubric:: Returns:

std::future object that can be used to get the task's return value or catch its exceptions.


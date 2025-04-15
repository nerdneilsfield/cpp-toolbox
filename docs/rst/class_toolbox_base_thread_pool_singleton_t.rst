.. index:: pair: class; toolbox::base::thread_pool_singleton_t
.. _doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t:

class toolbox::base::thread_pool_singleton_t
============================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A singleton thread pool implementation that manages a pool of worker threads. :ref:`More...<details-classtoolbox_1_1base_1_1thread__pool__singleton__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <thread_pool_singleton.hpp>
	
	class thread_pool_singleton_t {
	public:
		// construction
	
		:target:`thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aa65e50131198b59c1840bca9dcbb8fb9>`(const thread_pool_singleton_t&);
		:target:`thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1a797a6ce91fd3f9f9e2b32d14d218354b>`(thread_pool_singleton_t&&);
		:ref:`~thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aa5041f3e9ee0c6b6f982fa77c53a6ee1>`();

		// methods
	
		static
		thread_pool_singleton_t&
		:ref:`instance<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aae07bd8c92d5250118e6fac71da8bbfc>`();
	
		thread_pool_singleton_t&
		:target:`operator=<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1a819571d65765b44e5ffca61c1f20f339>`(const thread_pool_singleton_t&);
	
		thread_pool_singleton_t&
		:target:`operator=<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1a6f0f263ce928dccf8e20fdec8b72c547>`(thread_pool_singleton_t&&);
	
		template <
			class F,
			class... Args
		>
		auto
		:ref:`submit<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1a7df92875a7403035819bc5c134d420ea>`(
			F&& f,
			Args&&... args
		);
	
		:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`
		:ref:`get_thread_count<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1ad214d4edef3d6ed542c4b408a5969ab4>`() const;
	};
.. _details-classtoolbox_1_1base_1_1thread__pool__singleton__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A singleton thread pool implementation that manages a pool of worker threads.

This class provides a thread-safe singleton thread pool that can execute tasks asynchronously. Tasks are submitted as functions/lambdas and are queued for execution by worker threads.

.. ref-code-block:: cpp

	// Get thread pool instance
	auto& pool = :ref:`thread_pool_singleton_t::instance <doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aae07bd8c92d5250118e6fac71da8bbfc>`();
	
	// Submit a task and get future
	auto future = pool.submit([](int x) { return x * 2; }, 42);
	
	// Get result
	int result = future.get(); // result = 84

Construction
------------

.. _doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aa5041f3e9ee0c6b6f982fa77c53a6ee1:
.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	~thread_pool_singleton_t()

Destructor that stops the thread pool and waits for all worker threads to finish.

Methods
-------

.. index:: pair: function; instance
.. _doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1aae07bd8c92d5250118e6fac71da8bbfc:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	static
	thread_pool_singleton_t&
	instance()

Get the singleton instance of the thread pool.



.. rubric:: Returns:

Reference to the singleton thread pool instance

.. index:: pair: function; submit
.. _doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1a7df92875a7403035819bc5c134d420ea:

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

Submit a task to be executed by the thread pool.

This method queues a task for asynchronous execution by one of the worker threads. The task can be a function, lambda, or any callable object.

.. ref-code-block:: cpp

	// Submit a lambda
	auto future1 = pool.submit([]() { return "Hello"; });
	
	// Submit a function with arguments
	auto future2 = pool.submit([](int x, int y) { return x + y; }, 2, 3);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F

		- Type of the callable

	*
		- Args

		- Types of the arguments

	*
		- f

		- The callable to execute

	*
		- args

		- Arguments to pass to the callable

	*
		- std::runtime_error

		- if the thread pool has been stopped



.. rubric:: Returns:

std::future containing the eventual result

.. index:: pair: function; get_thread_count
.. _doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t_1ad214d4edef3d6ed542c4b408a5969ab4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`
	get_thread_count() const

Get the number of worker threads in the pool.



.. rubric:: Returns:

The number of worker threads


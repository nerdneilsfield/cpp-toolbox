.. index:: pair: class; toolbox::base::object_pool_t
.. _doxid-classtoolbox_1_1base_1_1object__pool__t:

template class toolbox::base::object_pool_t
===========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

A thread-safe object pool template. :ref:`More...<details-classtoolbox_1_1base_1_1object__pool__t>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <object_pool.hpp>
	
	template <typename T>
	class object_pool_t {
	public:
		// typedefs
	
		typedef std::unique_ptr<T, :ref:`PoolDeleter<doxid-classtoolbox_1_1base_1_1PoolDeleter>`<T>> :ref:`PooledObjectPtr<doxid-classtoolbox_1_1base_1_1object__pool__t_1ae721bc932699daa4c5e521583b54eb5f>`;

		// construction
	
		:ref:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t_1a76ed5cc37a23b4b4c05e7035c844b44e>`(
			:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` initial_size = 0,
			std::function<void(T&)> resetter = nullptr
		);
	
		:target:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t_1ade913fc522724ec24d9e670856c9f248>`(const object_pool_t&);
		:target:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t_1ad80f3c326e026fde449d11560d12c4ce>`(object_pool_t&&);

		// methods
	
		object_pool_t&
		:target:`operator=<doxid-classtoolbox_1_1base_1_1object__pool__t_1a787f41fa3fa731b4d5176a67f016227e>`(const object_pool_t&);
	
		object_pool_t&
		:target:`operator=<doxid-classtoolbox_1_1base_1_1object__pool__t_1a306fd4fc15c652b42768cad593cf6a32>`(object_pool_t&&);
	
		:ref:`PooledObjectPtr<doxid-classtoolbox_1_1base_1_1object__pool__t_1ae721bc932699daa4c5e521583b54eb5f>`
		:ref:`acquire<doxid-classtoolbox_1_1base_1_1object__pool__t_1aee1365b915dd94ff289623f8b8e3818f>`();
	
		void
		:ref:`release<doxid-classtoolbox_1_1base_1_1object__pool__t_1ad760b438b3168f37bfe95d2bf16c1735>`(T* ptr);
	};
.. _details-classtoolbox_1_1base_1_1object__pool__t:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

A thread-safe object pool template.

Manages a pool of reusable objects of type T to reduce allocation overhead. Uses RAII via std::unique_ptr with a custom deleter to manage object lifetimes.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of object to pool. Must be default-constructible.

Typedefs
--------

.. index:: pair: typedef; PooledObjectPtr
.. _doxid-classtoolbox_1_1base_1_1object__pool__t_1ae721bc932699daa4c5e521583b54eb5f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::unique_ptr<T, :ref:`PoolDeleter<doxid-classtoolbox_1_1base_1_1PoolDeleter>`<T>> PooledObjectPtr

Alias for a unique_ptr managing a pooled object. Automatically returns the object to the pool when it goes out of scope.

Construction
------------

.. index:: pair: function; object_pool_t
.. _doxid-classtoolbox_1_1base_1_1object__pool__t_1a76ed5cc37a23b4b4c05e7035c844b44e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	object_pool_t(
		:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` initial_size = 0,
		std::function<void(T&)> resetter = nullptr
	)

Construct an object pool.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- initial_size

		- Number of objects to create initially. Defaults to 0.

	*
		- resetter

		- Optional function to call on an object to reset its state before returning it to the pool.

Methods
-------

.. index:: pair: function; acquire
.. _doxid-classtoolbox_1_1base_1_1object__pool__t_1aee1365b915dd94ff289623f8b8e3818f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	:ref:`PooledObjectPtr<doxid-classtoolbox_1_1base_1_1object__pool__t_1ae721bc932699daa4c5e521583b54eb5f>`
	acquire()

Acquire an object from the pool.

If the pool is empty, a new object is created. The returned unique_ptr will automatically release the object back to the pool upon destruction.



.. rubric:: Returns:

A std::unique_ptr managing the acquired object.

.. index:: pair: function; release
.. _doxid-classtoolbox_1_1base_1_1object__pool__t_1ad760b438b3168f37bfe95d2bf16c1735:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	release(T* ptr)

Release an object back to the pool.

Called automatically by the :ref:`PoolDeleter <doxid-classtoolbox_1_1base_1_1PoolDeleter>` when the unique_ptr goes out of scope. Resets the object's state if a reset function was provided.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ptr

		- Raw pointer to the object to release. Ownership is taken by the pool.


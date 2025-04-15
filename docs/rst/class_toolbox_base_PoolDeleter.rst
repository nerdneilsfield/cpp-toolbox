.. index:: pair: class; toolbox::base::PoolDeleter
.. _doxid-classtoolbox_1_1base_1_1PoolDeleter:

template class toolbox::base::PoolDeleter
=========================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Custom deleter for std::unique_ptr to return objects to an :ref:`object_pool_t <doxid-classtoolbox_1_1base_1_1object__pool__t>`. :ref:`More...<details-classtoolbox_1_1base_1_1PoolDeleter>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <object_pool.hpp>
	
	template <typename T>
	class PoolDeleter {
	public:
		// construction
	
		:ref:`PoolDeleter<doxid-classtoolbox_1_1base_1_1PoolDeleter_1a68dcde0408c7f633cdf81773f3896806>`(:ref:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t>`<T>* pool = nullptr);

		// methods
	
		void
		:ref:`operator()<doxid-classtoolbox_1_1base_1_1PoolDeleter_1a85aa08132d5284793d3399919cb98bbe>`(T* ptr) const;
	};
.. _details-classtoolbox_1_1base_1_1PoolDeleter:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Custom deleter for std::unique_ptr to return objects to an :ref:`object_pool_t <doxid-classtoolbox_1_1base_1_1object__pool__t>`.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- The type of object managed by the pool.

Construction
------------

.. index:: pair: function; PoolDeleter
.. _doxid-classtoolbox_1_1base_1_1PoolDeleter_1a68dcde0408c7f633cdf81773f3896806:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	PoolDeleter(:ref:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t>`<T>* pool = nullptr)

Construct a :ref:`PoolDeleter <doxid-classtoolbox_1_1base_1_1PoolDeleter>`.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- pool

		- Pointer to the object pool to release objects back to.

Methods
-------

.. index:: pair: function; operator()
.. _doxid-classtoolbox_1_1base_1_1PoolDeleter_1a85aa08132d5284793d3399919cb98bbe:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	void
	operator()(T* ptr) const

The function called by std::unique_ptr instead of delete. Releases the object back to the pool if the pool pointer is valid.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ptr

		- Raw pointer to the object being managed.


.. index:: pair: namespace; toolbox::concurrent
.. _doxid-namespacetoolbox_1_1concurrent:

namespace toolbox::concurrent
=============================

.. toctree::
	:hidden:

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace concurrent {

	// global functions

	:ref:`base::thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t>`&
	:ref:`default_pool<doxid-namespacetoolbox_1_1concurrent_1ad83682a69d167e3b7f8712566033e180>`();

	template <
		typename Iterator,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_for_each<doxid-namespacetoolbox_1_1concurrent_1acae673a7f47c092841ef7aab1894a71f>`(
		Iterator begin,
		Iterator end,
		Function func
	);

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_for_each<doxid-namespacetoolbox_1_1concurrent_1aa0a37c5ad71c53905b1c8db374cd4d43>`(
		std::vector<T, Alloc>& vec,
		Function func
	);

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_for_each<doxid-namespacetoolbox_1_1concurrent_1ab9f7beaaac6b99bcd570652522778550>`(
		const std::vector<T, Alloc>& vec,
		Function func
	);

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_for_each<doxid-namespacetoolbox_1_1concurrent_1aa7bd6ddb7ab2da26b9705ca3ed2f9c94>`(
		std::array<T, N>& arr,
		Function func
	);

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_for_each<doxid-namespacetoolbox_1_1concurrent_1a3445a42f0d7ae47387555bc5b4576648>`(
		const std::array<T, N>& arr,
		Function func
	);

	template <
		typename InputIt,
		typename OutputIt,
		typename UnaryOperation
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_transform<doxid-namespacetoolbox_1_1concurrent_1adc129cfed7d6ea2f9bd3cffde739ce9d>`(
		InputIt first1,
		InputIt last1,
		OutputIt d_first,
		UnaryOperation unary_op
	);

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_transform<doxid-namespacetoolbox_1_1concurrent_1aebf0bab748ed9ab65042ad8700411a9e>`(
		std::vector<T, Alloc>& vec,
		Function func
	);

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_transform<doxid-namespacetoolbox_1_1concurrent_1a778ee00b731c893ba86f150deae148b4>`(
		const std::vector<T, Alloc>& vec,
		Function func
	);

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_transform<doxid-namespacetoolbox_1_1concurrent_1aab4fda981d54019a30712394d7747cc4>`(
		std::array<T, N>& arr,
		Function func
	);

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	:ref:`parallel_transform<doxid-namespacetoolbox_1_1concurrent_1aa1e6d8ab2286c5d96dd9e0de3fc13ad1>`(
		const std::array<T, N>& arr,
		Function func
	);

	template <
		typename Iterator,
		typename T,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	:ref:`parallel_reduce<doxid-namespacetoolbox_1_1concurrent_1a6ea1b3789d0e60da0d852d4d2b1b0587>`(
		Iterator begin,
		Iterator end,
		T identity,
		BinaryOperation reduce_op
	);

	template <
		typename T,
		typename Alloc,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	:ref:`parallel_reduce<doxid-namespacetoolbox_1_1concurrent_1a901bcfda7e90ddf49a34af8665b20564>`(
		std::vector<T, Alloc>& vec,
		T identity,
		BinaryOperation reduce_op
	);

	template <
		typename T,
		typename Alloc,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	:ref:`parallel_reduce<doxid-namespacetoolbox_1_1concurrent_1ad88cfa90539431b271421f750576bc0f>`(
		const std::vector<T, Alloc>& vec,
		T identity,
		BinaryOperation reduce_op
	);

	template <
		typename T,
		size_t N,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	:ref:`parallel_reduce<doxid-namespacetoolbox_1_1concurrent_1aa11d099f6070651f3af1f243019118b8>`(
		std::array<T, N>& arr,
		T identity,
		BinaryOperation reduce_op
	);

	template <
		typename T,
		size_t N,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	:ref:`parallel_reduce<doxid-namespacetoolbox_1_1concurrent_1abb403005ec151d5f1ce67d3986733d4d>`(
		const std::array<T, N>& arr,
		T identity,
		BinaryOperation reduce_op
	);

	} // namespace concurrent
.. _details-namespacetoolbox_1_1concurrent:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; default_pool
.. _doxid-namespacetoolbox_1_1concurrent_1ad83682a69d167e3b7f8712566033e180:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	:ref:`base::thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t>`&
	default_pool()

Get the default thread pool instance.



.. rubric:: Returns:

Reference to the singleton thread pool instance

.. index:: pair: function; parallel_for_each
.. _doxid-namespacetoolbox_1_1concurrent_1acae673a7f47c092841ef7aab1894a71f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Iterator,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_for_each(
		Iterator begin,
		Iterator end,
		Function func
	)

Applies a function to each element in range [begin, end) in parallel.

Uses default thread pool for manual task division and submission

.. ref-code-block:: cpp

	std::vector<int> vec = {1, 2, 3, 4, 5};
	
	// Double each element in parallel
	:ref:`parallel_for_each <doxid-namespacetoolbox_1_1concurrent_1acae673a7f47c092841ef7aab1894a71f>`(vec.begin(), vec.end(), [](int& x) {
	  x *= 2;
	});
	// vec now contains {2, 4, 6, 8, 10}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Iterator

		- Input iterator type (must support random access for efficient chunking)

	*
		- Function

		- Unary function object type

	*
		- begin

		- Start iterator of range

	*
		- end

		- End iterator of range

	*
		- func

		- Function object to apply to each element

.. index:: pair: function; parallel_for_each
.. _doxid-namespacetoolbox_1_1concurrent_1aa0a37c5ad71c53905b1c8db374cd4d43:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_for_each(
		std::vector<T, Alloc>& vec,
		Function func
	)

Convenience overload for vectors.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Vector element type

	*
		- Alloc

		- Vector allocator type

	*
		- Function

		- Function object type

	*
		- vec

		- Vector to process

	*
		- func

		- Function to apply

.. index:: pair: function; parallel_for_each
.. _doxid-namespacetoolbox_1_1concurrent_1ab9f7beaaac6b99bcd570652522778550:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_for_each(
		const std::vector<T, Alloc>& vec,
		Function func
	)

Convenience overload for const vectors.

.. index:: pair: function; parallel_for_each
.. _doxid-namespacetoolbox_1_1concurrent_1aa7bd6ddb7ab2da26b9705ca3ed2f9c94:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_for_each(
		std::array<T, N>& arr,
		Function func
	)

Convenience overload for arrays.

.. index:: pair: function; parallel_for_each
.. _doxid-namespacetoolbox_1_1concurrent_1a3445a42f0d7ae47387555bc5b4576648:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_for_each(
		const std::array<T, N>& arr,
		Function func
	)

Convenience overload for const arrays.

.. index:: pair: function; parallel_transform
.. _doxid-namespacetoolbox_1_1concurrent_1adc129cfed7d6ea2f9bd3cffde739ce9d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename InputIt,
		typename OutputIt,
		typename UnaryOperation
	>
	CPP_TOOLBOX_EXPORT void
	parallel_transform(
		InputIt first1,
		InputIt last1,
		OutputIt d_first,
		UnaryOperation unary_op
	)

Transforms elements from input range to output range in parallel.

Applies unary_op to [first1, last1) storing results in [d_first,...)

.. warning::

	Input and output iterators must be random access iterators



.. warning::

	Output range [d_first, d_first + distance(first1, last1)) must have sufficient allocated capacity



.. ref-code-block:: cpp

	std::vector<int> input = {1, 2, 3, 4, 5};
	std::vector<int> output(input.size());
	
	// Square each element in parallel
	:ref:`parallel_transform <doxid-namespacetoolbox_1_1concurrent_1adc129cfed7d6ea2f9bd3cffde739ce9d>`(input.begin(), input.end(), output.begin(),
	                  [](int x) { return x * x; });
	// output now contains {1, 4, 9, 16, 25}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- InputIt

		- Input iterator type (std::random_access_iterator)

	*
		- OutputIt

		- Output iterator type (std::random_access_iterator)

	*
		- UnaryOperation

		- Unary operation type, signature should be OutType(const InType&)

	*
		- first1

		- Start of input range

	*
		- last1

		- End of input range

	*
		- d_first

		- Start of output range

	*
		- unary_op

		- Operation to apply to each element

.. index:: pair: function; parallel_transform
.. _doxid-namespacetoolbox_1_1concurrent_1aebf0bab748ed9ab65042ad8700411a9e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_transform(
		std::vector<T, Alloc>& vec,
		Function func
	)

Convenience overload for vectors.

.. index:: pair: function; parallel_transform
.. _doxid-namespacetoolbox_1_1concurrent_1a778ee00b731c893ba86f150deae148b4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_transform(
		const std::vector<T, Alloc>& vec,
		Function func
	)

Convenience overload for const vectors.

.. index:: pair: function; parallel_transform
.. _doxid-namespacetoolbox_1_1concurrent_1aab4fda981d54019a30712394d7747cc4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_transform(
		std::array<T, N>& arr,
		Function func
	)

Convenience overload for arrays.

.. index:: pair: function; parallel_transform
.. _doxid-namespacetoolbox_1_1concurrent_1aa1e6d8ab2286c5d96dd9e0de3fc13ad1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename Function
	>
	CPP_TOOLBOX_EXPORT void
	parallel_transform(
		const std::array<T, N>& arr,
		Function func
	)

Convenience overload for const arrays.

.. index:: pair: function; parallel_reduce
.. _doxid-namespacetoolbox_1_1concurrent_1a6ea1b3789d0e60da0d852d4d2b1b0587:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Iterator,
		typename T,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	parallel_reduce(
		Iterator begin,
		Iterator end,
		T identity,
		BinaryOperation reduce_op
	)

Performs parallel reduction on range [begin, end)

Uses default thread pool for task division. Performs local reduction then merges results. reduce_op should be associative. identity must be identity element for reduce_op.

.. ref-code-block:: cpp

	std::vector<int> vec = {1, 2, 3, 4, 5};
	
	// Sum elements in parallel
	int sum = :ref:`parallel_reduce <doxid-namespacetoolbox_1_1concurrent_1a6ea1b3789d0e60da0d852d4d2b1b0587>`(vec.begin(), vec.end(), 0,
	                         std::plus<int>());
	// sum == 15
	
	// Find maximum element
	int max = :ref:`parallel_reduce <doxid-namespacetoolbox_1_1concurrent_1a6ea1b3789d0e60da0d852d4d2b1b0587>`(vec.begin(), vec.end(),
	                         std::numeric_limits<int>::min(),
	                         [](int a, int b) { return std::max(a,b); });
	// max == 5



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Iterator

		- Input iterator type (must support random access)

	*
		- T

		- Type of reduction result and identity element

	*
		- BinaryOperation

		- Binary operation type, signature should be T(const T&, const ElementType&) or T(T,T)

	*
		- begin

		- Start iterator

	*
		- end

		- End iterator

	*
		- identity

		- Identity element for reduction

	*
		- reduce_op

		- Binary operation to merge two T values or T with element type



.. rubric:: Returns:

Result of parallel reduction

.. index:: pair: function; parallel_reduce
.. _doxid-namespacetoolbox_1_1concurrent_1a901bcfda7e90ddf49a34af8665b20564:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	parallel_reduce(
		std::vector<T, Alloc>& vec,
		T identity,
		BinaryOperation reduce_op
	)

Convenience overload for vectors.

.. index:: pair: function; parallel_reduce
.. _doxid-namespacetoolbox_1_1concurrent_1ad88cfa90539431b271421f750576bc0f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename Alloc,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	parallel_reduce(
		const std::vector<T, Alloc>& vec,
		T identity,
		BinaryOperation reduce_op
	)

Convenience overload for const vectors.

.. index:: pair: function; parallel_reduce
.. _doxid-namespacetoolbox_1_1concurrent_1aa11d099f6070651f3af1f243019118b8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	parallel_reduce(
		std::array<T, N>& arr,
		T identity,
		BinaryOperation reduce_op
	)

Convenience overload for arrays.

.. index:: pair: function; parallel_reduce
.. _doxid-namespacetoolbox_1_1concurrent_1abb403005ec151d5f1ce67d3986733d4d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		size_t N,
		typename BinaryOperation
	>
	CPP_TOOLBOX_EXPORT T
	parallel_reduce(
		const std::array<T, N>& arr,
		T identity,
		BinaryOperation reduce_op
	)

Convenience overload for const arrays.


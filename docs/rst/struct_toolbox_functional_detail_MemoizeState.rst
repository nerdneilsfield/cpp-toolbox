.. index:: pair: struct; toolbox::functional::detail::MemoizeState
.. _doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState:

template struct toolbox::functional::detail::MemoizeState
=========================================================

.. toctree::
	:hidden:

Overview
~~~~~~~~

Internal state for memoization. :ref:`More...<details-structtoolbox_1_1functional_1_1detail_1_1MemoizeState>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <
		typename Func,
		typename R,
		typename... Args
	>
	struct MemoizeState {
		// typedefs
	
		typedef std::tuple<std::decay_t<Args>...> :target:`KeyType<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1ad6ad3485a98e162c328816616145caa5>`;
		typedef R :target:`ResultType<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1a0c3bb92ef50f79d303fe75ed1e7eb178>`;

		// fields
	
		Func :target:`original_func<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1a1bafd0fec40682cca4825d6af6810b3d>`;
		std::map<:ref:`KeyType<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1ad6ad3485a98e162c328816616145caa5>`, :ref:`ResultType<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1a0c3bb92ef50f79d303fe75ed1e7eb178>`> :target:`cache<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1ae1f1475b804e61955db9a60e564e42f8>`;
		std::mutex :target:`cache_mutex<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1a3ab5f6a25db8402dfb45fca8ae6248a3>`;

		// construction
	
		:target:`MemoizeState<doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1acd2869e33a4b1e6d052d66c0d8dba349>`(Func&& f);
	};
.. _details-structtoolbox_1_1functional_1_1detail_1_1MemoizeState:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Internal state for memoization.

.. ref-code-block:: cpp

	:ref:`MemoizeState <doxid-structtoolbox_1_1functional_1_1detail_1_1MemoizeState_1acd2869e33a4b1e6d052d66c0d8dba349>`<int(int), int, int> state{[](int x) { return x*x; }};
	auto result = state.original_func(5); // Returns 25


.. index:: pair: struct; toolbox::functional::detail_impl::MemoizeHelperState
.. _doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState:

template struct toolbox::functional::detail_impl::MemoizeHelperState
====================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional_impl.hpp>
	
	template <
		typename R,
		typename... Args
	>
	struct MemoizeHelperState {
		// typedefs
	
		typedef std::function<R(Args...)> :target:`FuncType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a99e23b563ebf71980e6905a54227656a>`;
		typedef std::tuple<std::decay_t<Args>...> :target:`KeyType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1ad462de8b02cd8c504cf4ed92fecf8bae>`;
		typedef R :target:`ResultType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a586f3942e8568eef056525a07f751f3f>`;

		// fields
	
		:ref:`FuncType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a99e23b563ebf71980e6905a54227656a>` :target:`original_func_<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a2ed390cdab3fd7a5c04280931fc8f5a7>`;
		std::map<:ref:`KeyType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1ad462de8b02cd8c504cf4ed92fecf8bae>`, :ref:`ResultType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a586f3942e8568eef056525a07f751f3f>`> :target:`cache_<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a97845c1f6a31ef13d4c2b9d65bd58653>`;
		std::mutex :target:`cache_mutex_<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1ab35405d89ac8d3be9fad226dcf68a537>`;

		// construction
	
		:target:`MemoizeHelperState<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a140e730fd3f7fe1c2a5f593629a9c6dc>`(:ref:`FuncType<doxid-structtoolbox_1_1functional_1_1detail__impl_1_1MemoizeHelperState_1a99e23b563ebf71980e6905a54227656a>` f);
	};

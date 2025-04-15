.. index:: pair: struct; toolbox::functional::MemoizedFunction::MemoizedFunction<R(Args...)>
.. _doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State:

template struct toolbox::functional::MemoizedFunction::MemoizedFunction<R(Args...)>
===================================================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional_impl.hpp>
	
	template <>
	struct MemoizedFunction<R(Args...)> {
		// typedefs
	
		typedef std::function<R(Args...)> :target:`FuncType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a68de85dd7b44f677d95bff1422745933>`;
		typedef std::tuple<std::decay_t<Args>...> :target:`KeyType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a93a053ab87b4e47889c39a8fa943d211>`;
		typedef R :target:`ResultType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a5e26374c6dadf7bceadb10e20011cf88>`;

		// fields
	
		:ref:`FuncType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a68de85dd7b44f677d95bff1422745933>` :target:`original_func_<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1ac513ebd72ad7ea183dd4579b02edb24a>`;
		std::map<:ref:`KeyType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a93a053ab87b4e47889c39a8fa943d211>`, :ref:`ResultType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a5e26374c6dadf7bceadb10e20011cf88>`> :target:`cache_<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a64cc4210f27a2bd58d37c48ac9e3b9a7>`;
		std::mutex :target:`cache_mutex_<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a46bccb5b1f9f6edf22608e660d5ab7e8>`;

		// methods
	
		:target:`State<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a79c13186c583c6358ea8b5a39a7cd020>`(:ref:`FuncType<doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State_1a68de85dd7b44f677d95bff1422745933>` f);
	};

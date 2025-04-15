.. index:: pair: class; toolbox::functional::MemoizedFunction<R(Args...)>
.. _doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4:

template class toolbox::functional::MemoizedFunction<R(Args...)>
================================================================

.. toctree::
	:hidden:

	struct_toolbox_functional_MemoizedFunction_MemoizedFunction.rst

Specialization of MemoizedFunction for specific function signature Declaration only, definition in impl.


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <functional.hpp>
	
	template <
		typename R,
		typename... Args
	>
	class MemoizedFunction<R(Args...)> {
	public:
		// structs
	
		template <>
		struct :ref:`MemoizedFunction<R(Args...)><doxid-structtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1_1State>`;

		// construction
	
		:target:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a0e2209ac8f5f89adb0a6038bf7604f10>`(FuncType f);
		:target:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1acae96bbec0e5ef9defe196a121767b61>`(const MemoizedFunction&);
		:target:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1ae139566d0d34a86210d6c13f5f77a286>`(MemoizedFunction&&);

		// methods
	
		:ref:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a0e2209ac8f5f89adb0a6038bf7604f10>`&
		:target:`operator=<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a3f53e4f6027af288cb537a6a04ac025f>`(const :ref:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a0e2209ac8f5f89adb0a6038bf7604f10>`&);
	
		:ref:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a0e2209ac8f5f89adb0a6038bf7604f10>`&
		:target:`operator=<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a489dd036ec959c2d501c49b68b6fdbe1>`(:ref:`MemoizedFunction<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a0e2209ac8f5f89adb0a6038bf7604f10>`&&);
	
		auto
		:target:`operator()<doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4_1a27ef5ae7912753e200ffc53131f47c8d>`(Args... args);
	};

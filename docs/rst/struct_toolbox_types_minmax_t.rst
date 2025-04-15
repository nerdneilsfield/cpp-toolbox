.. index:: pair: struct; toolbox::types::minmax_t
.. _doxid-structtoolbox_1_1types_1_1minmax__t:

template struct toolbox::types::minmax_t
========================================

.. toctree::
	:hidden:

Primary template for :ref:`minmax_t <doxid-structtoolbox_1_1types_1_1minmax__t>`.


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <minmax.hpp>
	
	template <typename T>
	struct minmax_t {
		// fields
	
		T :target:`min<doxid-structtoolbox_1_1types_1_1minmax__t_1afb9bf412ac5bcba091faf576f52ab346>`;
		T :target:`max<doxid-structtoolbox_1_1types_1_1minmax__t_1a4b0a43eedd016e2a3a9760dce0c345e3>`;
		bool :target:`initialized_<doxid-structtoolbox_1_1types_1_1minmax__t_1a41b7ceedddbfedf61bf911c44faeb189>` = false;

		// construction
	
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1aec5e824ad0918d24bed4fb51f26c95ee>`(
			T initial_min,
			T initial_max
		);
	
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a4d00569e74a3057cb147ef2720268b69>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a16a35d289fe3e8ce5c58b54f8da7dda7>`(const minmax_t& other);
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1ac8d7e056dbd5f3b803eafc3e2980004a>`(minmax_t&& other);
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a71591d15ef0f3fe4f2e441d56d3ba225>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a634d4f2685a477d374a620df0ee305a6>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a7ddfe77698049a667fbb091f6a29d9e3>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1af5f405a434b6e629230bf306a2325646>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a301b4325ef4200f0abfb39badfe56a6a>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a0bfb0c0a80be3012a115a5693fa4fc83>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1aed41c69e2ad9e6bf1f996f71c903da69>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1a142baa972a7519f1df7b26d063a3d765>`();
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1af4e5b4d9c8447066339466df816a9aa0>`();
		:target:`~minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_1aa5756b1220b5fc608e9c4c2b8715f332>`();

		// methods
	
		minmax_t&
		:target:`operator=<doxid-structtoolbox_1_1types_1_1minmax__t_1a4c2ddf1fb741a8ebe0ece51f612120e5>`(const minmax_t& other);
	
		minmax_t&
		:target:`operator=<doxid-structtoolbox_1_1types_1_1minmax__t_1ae68eb52d8d8d2e52d09a928c281d236f>`(minmax_t&& other);
	
		minmax_t&
		:target:`operator+=<doxid-structtoolbox_1_1types_1_1minmax__t_1abfe2e732d7c6107077d09a70a6f00532>`(const T& value);
	};

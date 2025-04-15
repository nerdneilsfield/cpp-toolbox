.. index:: pair: namespace; toolbox::container::detail
.. _doxid-namespacetoolbox_1_1container_1_1detail:

namespace toolbox::container::detail
====================================

.. toctree::
	:hidden:

	struct_toolbox_container_detail_HPRec.rst
	class_toolbox_container_detail_HazardPointerGuard.rst




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace detail {

	// structs

	struct :ref:`HPRec<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec>`;

	// classes

	class :ref:`HazardPointerGuard<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard>`;

	// global variables

	std::vector<:ref:`HPRec<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec>`*> :target:`g_hp_list<doxid-namespacetoolbox_1_1container_1_1detail_1a9de94ce41458a5376a03b31314f658ba>`;
	std::mutex :target:`g_hp_list_mutex<doxid-namespacetoolbox_1_1container_1_1detail_1ac81750d8ccc240d65f0fc39996322203>`;
	thread_local std::vector<std::pair<void*, std::function<void(void*)>>> :target:`t_retired_list<doxid-namespacetoolbox_1_1container_1_1detail_1a20709ef73a40d04b12059915f687eac1>`;
	constexpr :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` :target:`MAX_HAZARD_POINTERS_PER_THREAD<doxid-namespacetoolbox_1_1container_1_1detail_1a9d8e85c8cd4612108a0b0c4fb7433b2d>` =     2;
	constexpr :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` :target:`RETIRE_SCAN_THRESHOLD<doxid-namespacetoolbox_1_1container_1_1detail_1a0294c9b505f4827357db54e2fb25a3b1>` =     100;

	// global functions

	:ref:`HPRec<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec>`*
	:target:`acquire_hp_record<doxid-namespacetoolbox_1_1container_1_1detail_1ab5fc1f73f81210682b262477d5ab8e20>`();

	void
	:target:`release_hp_record<doxid-namespacetoolbox_1_1container_1_1detail_1ad24b531d08aaf5d51fd24cbb974f1c9f>`(:ref:`HPRec<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec>`* rec);

	void
	:target:`set_hazard_pointer<doxid-namespacetoolbox_1_1container_1_1detail_1aec8de8fead358ddea2ed692abaa6b9cb>`(
		:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` index,
		void* ptr
	);

	void
	:target:`clear_hazard_pointer<doxid-namespacetoolbox_1_1container_1_1detail_1a8e740e605178300fe2fdef7b47781f99>`(:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` index);

	void
	:target:`scan_retired_nodes<doxid-namespacetoolbox_1_1container_1_1detail_1a184105b295c8774ca00c31a04500b697>`();

	void
	:target:`retire_node<doxid-namespacetoolbox_1_1container_1_1detail_1af5d2eed5614b3bb0b45572016ba013b7>`(
		void* node,
		std::function<void(void*)> deleter
	);

	void
	:target:`cleanup_retired_nodes<doxid-namespacetoolbox_1_1container_1_1detail_1ada646727df8d265477dbc1873452fced>`();

	} // namespace detail

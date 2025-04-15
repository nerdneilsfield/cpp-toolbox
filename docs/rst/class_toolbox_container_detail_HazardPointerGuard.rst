.. index:: pair: class; toolbox::container::detail::HazardPointerGuard
.. _doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard:

class toolbox::container::detail::HazardPointerGuard
====================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <lock_free_queue.hpp>
	
	class HazardPointerGuard {
	public:
		// construction
	
		:target:`HazardPointerGuard<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1a96e27b06f82ed4414804c8866d8a1b2b>`(
			:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>` index,
			void* node
		);
	
		:target:`HazardPointerGuard<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1aeec7e3baf29bfb7635d5f3e2cb26d4f5>`(const HazardPointerGuard&);
		:target:`HazardPointerGuard<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1a820dad21bca9b661c8f05246eafb7af3>`(HazardPointerGuard&&);
		:target:`~HazardPointerGuard<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1a7a5ee85fd667ed998740d6bf3aa29c6c>`();

		// methods
	
		HazardPointerGuard&
		:target:`operator=<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1a60c88b451e9103e1aca9a58dd8b8871f>`(const HazardPointerGuard&);
	
		HazardPointerGuard&
		:target:`operator=<doxid-classtoolbox_1_1container_1_1detail_1_1HazardPointerGuard_1ab7431aac59cfe48cd48bb11123d6af55>`(HazardPointerGuard&&);
	};

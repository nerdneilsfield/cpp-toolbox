.. index:: pair: class; cpp_toolbox::io::pcd_file_data_t
.. _doxid-classcpp__toolbox_1_1io_1_1pcd__file__data__t:

class cpp_toolbox::io::pcd_file_data_t
======================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <pcd.hpp>
	
	class pcd_file_data_t: public base_file_data_t {
	public:
		// fields
	
		std::vector<Eigen::Vector3d> :target:`points<doxid-classcpp__toolbox_1_1io_1_1pcd__file__data__t_1a6fa5bb1a50b39ded1128849fdd89f33d>`;
		std::vector<Eigen::Vector3d> :target:`normals<doxid-classcpp__toolbox_1_1io_1_1pcd__file__data__t_1abe15a31a94099f19f14ac28491dc39d1>`;
		std::vector<Eigen::Vector3d> :target:`colors<doxid-classcpp__toolbox_1_1io_1_1pcd__file__data__t_1acbeb17ac81ebd61cf2c993094ed60919>`;
	};

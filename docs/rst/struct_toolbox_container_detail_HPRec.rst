.. index:: pair: struct; toolbox::container::detail::HPRec
.. _doxid-structtoolbox_1_1container_1_1detail_1_1HPRec:

struct toolbox::container::detail::HPRec
========================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <lock_free_queue.hpp>
	
	struct HPRec {
		// fields
	
		std::atomic<std::thread::id> :target:`owner_thread_id<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec_1abe71391f27efb23a714a19aa1004c0c2>`;
		std::atomic<void*> :target:`hazard_pointers<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec_1ab06cf32ff1a37a02a8a63ae97413ba67>`[MAX_HAZARD_POINTERS_PER_THREAD];
		HPRec* :target:`next<doxid-structtoolbox_1_1container_1_1detail_1_1HPRec_1a5b22c1279122a0099c553b583b3878d5>` = nullptr;
	};

.. index:: pair: struct; toolbox::container::lock_free_queue_t::Node
.. _doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node:

struct toolbox::container::lock_free_queue_t::Node
==================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	struct Node {
		// fields
	
		T :target:`data<doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node_1aeaad1b68047f208f545d254e102290a3>`;
		std::atomic<Node*> :target:`next<doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node_1acc4dfd3a9f0ca5cc98a17ad9c3551ac4>`;

		// construction
	
		:target:`Node<doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node_1a721801f1b73a39c914cc65fa6de451ef>`(T&& d);
		:target:`Node<doxid-structtoolbox_1_1container_1_1lock__free__queue__t_1_1Node_1a3fe74946f09e11d35fa8566ff18499b0>`();
	};

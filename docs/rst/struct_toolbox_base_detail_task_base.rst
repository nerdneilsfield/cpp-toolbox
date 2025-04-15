.. index:: pair: struct; toolbox::base::detail::task_base
.. _doxid-structtoolbox_1_1base_1_1detail_1_1task__base:

struct toolbox::base::detail::task_base
=======================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <task_base.hpp>
	
	struct task_base {
		// construction
	
		virtual
		:target:`~task_base<doxid-structtoolbox_1_1base_1_1detail_1_1task__base_1a9f9975f993f9eaffd342217ea2a43cbb>`();

		// methods
	
		virtual
		void
		:target:`execute<doxid-structtoolbox_1_1base_1_1detail_1_1task__base_1a5d67d2b405ff1fa3b2e87504e0f20558>`() = 0;
	};

	// direct descendants

	template <typename F>
	struct :ref:`task_derived<doxid-structtoolbox_1_1base_1_1detail_1_1task__derived>`;

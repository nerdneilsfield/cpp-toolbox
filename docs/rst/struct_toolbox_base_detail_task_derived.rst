.. index:: pair: struct; toolbox::base::detail::task_derived
.. _doxid-structtoolbox_1_1base_1_1detail_1_1task__derived:

template struct toolbox::base::detail::task_derived
===================================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <task_base.hpp>
	
	template <typename F>
	struct task_derived: public :ref:`toolbox::base::detail::task_base<doxid-structtoolbox_1_1base_1_1detail_1_1task__base>` {
		// fields
	
		F :target:`func<doxid-structtoolbox_1_1base_1_1detail_1_1task__derived_1af82174f39885333022b3723e383bddb6>`;

		// construction
	
		:target:`task_derived<doxid-structtoolbox_1_1base_1_1detail_1_1task__derived_1a98256125f90d63c81e1179e80a6ddffd>`(F&& f);

		// methods
	
		virtual
		void
		:target:`execute<doxid-structtoolbox_1_1base_1_1detail_1_1task__derived_1a9841dd999b4ae77161fe62c3fb648e98>`();
	};

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
		// methods
	
		virtual
		void
		:ref:`execute<doxid-structtoolbox_1_1base_1_1detail_1_1task__base_1a5d67d2b405ff1fa3b2e87504e0f20558>`() = 0;


.. index:: pair: class; toolbox::utils::ClickException
.. _doxid-classtoolbox_1_1utils_1_1ClickException:

class toolbox::utils::ClickException
====================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click.hpp>
	
	class ClickException: public runtime_error {
	public:
		// construction
	
		:target:`ClickException<doxid-classtoolbox_1_1utils_1_1ClickException_1ae481f5cd96e3220700cdfcb903ebc138>`(const std::string& message);

		// methods
	
		virtual
		void
		:target:`print<doxid-classtoolbox_1_1utils_1_1ClickException_1afa3913129f0afdee261cc1ff24b485ac>`() const;
	};

	// direct descendants

	class :ref:`ParameterError<doxid-classtoolbox_1_1utils_1_1ParameterError>`;
	class :ref:`UsageError<doxid-classtoolbox_1_1utils_1_1UsageError>`;

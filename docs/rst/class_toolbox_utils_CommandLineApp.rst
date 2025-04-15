.. index:: pair: class; toolbox::utils::CommandLineApp
.. _doxid-classtoolbox_1_1utils_1_1CommandLineApp:

class toolbox::utils::CommandLineApp
====================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click.hpp>
	
	class CommandLineApp: public :ref:`toolbox::utils::command_t<doxid-classtoolbox_1_1utils_1_1command__t>` {
	public:
		// construction
	
		:target:`CommandLineApp<doxid-classtoolbox_1_1utils_1_1CommandLineApp_1a09918951950dda1103381cb7f79ba8e3>`(
			const std::string& name,
			const std::string& description
		);

		// methods
	
		int
		:target:`run<doxid-classtoolbox_1_1utils_1_1CommandLineApp_1acfb31491292ea6676b7361d766e1575e>`(
			int argc,
			char** argv
		);
	};

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
		// methods
	
		std::string
		:ref:`get_name<doxid-classtoolbox_1_1utils_1_1command__t_1a16d7229223462d27a26d24b1ee64e0ef>`() const;
	
		std::string
		:ref:`get_description<doxid-classtoolbox_1_1utils_1_1command__t_1ac5aed7d524d06c35f488155471d4eff7>`() const;
	
		template <typename T>
		:ref:`option_t<doxid-classtoolbox_1_1utils_1_1option__t>`<T>&
		:ref:`add_option<doxid-classtoolbox_1_1utils_1_1command__t_1aec9f0c285f02c46a544245f83bb2bf65>`(
			const std::string& name,
			const std::string& short_name,
			const std::string& description,
			bool required = false
		);
	
		:ref:`flag_t<doxid-classtoolbox_1_1utils_1_1flag__t>`&
		:ref:`add_flag<doxid-classtoolbox_1_1utils_1_1command__t_1a6b8a9b93d1125d860705020767f45dc5>`(
			const std::string& name,
			const std::string& short_name,
			const std::string& description
		);
	
		template <typename T>
		:ref:`argument_t<doxid-classtoolbox_1_1utils_1_1argument__t>`<T>&
		:ref:`add_argument<doxid-classtoolbox_1_1utils_1_1command__t_1a51b4b7380c98f978a19ce5f29cd5623c>`(
			const std::string& name,
			const std::string& description,
			bool required = true
		);
	
		:ref:`command_t<doxid-classtoolbox_1_1utils_1_1command__t>`&
		:ref:`add_command<doxid-classtoolbox_1_1utils_1_1command__t_1a432f45bb1f0470b26384ce9038d13f1b>`(
			const std::string& name,
			const std::string& description
		);
	
		:ref:`command_t<doxid-classtoolbox_1_1utils_1_1command__t>`&
		:ref:`set_callback<doxid-classtoolbox_1_1utils_1_1command__t_1a5e0ac7dff09dbc431645850040f137fa>`(std::function<int()> callback);
	
		int
		:ref:`parse_and_execute<doxid-classtoolbox_1_1utils_1_1command__t_1ae194baf1919e341f39e58aee4e5b448d>`(const std::vector<std::string>& args);
	
		std::string
		:ref:`format_help<doxid-classtoolbox_1_1utils_1_1command__t_1a5dfa0b0cd8add7049b092cb626e321ae>`() const;


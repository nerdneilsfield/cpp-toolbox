.. index:: pair: class; toolbox::utils::option_t
.. _doxid-classtoolbox_1_1utils_1_1option__t:

template class toolbox::utils::option_t
=======================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click.hpp>
	
	template <typename T>
	class option_t: public :ref:`toolbox::utils::parameter_t<doxid-classtoolbox_1_1utils_1_1parameter__t>` {
	public:
		// construction
	
		:target:`option_t<doxid-classtoolbox_1_1utils_1_1option__t_1ac3a446c6b879711daa15780e73e75253>`(
			const std::string& name,
			const std::string& short_name,
			const std::string& description,
			bool required = false
		);

		// methods
	
		option_t&
		:target:`set_default<doxid-classtoolbox_1_1utils_1_1option__t_1a2ba182db603d6f07a3380f09cea7430a>`(const T& default_value);
	
		option_t&
		:target:`set_parser<doxid-classtoolbox_1_1utils_1_1option__t_1a5552a69de0a9fa11311c049c7a61c072>`(std::function<bool(const std::string&, T&)> parser);
	
		virtual
		std::string
		:target:`get_short_name<doxid-classtoolbox_1_1utils_1_1option__t_1a7bbb86c7dd96b473c86b5412df2a35c8>`() const;
	
		T
		:target:`get<doxid-classtoolbox_1_1utils_1_1option__t_1ae87b226fece8b4189b12d27a5fdfe308>`() const;
	
		virtual
		bool
		:target:`parse<doxid-classtoolbox_1_1utils_1_1option__t_1a2adf77f412836ea9ac9e1b5b4887ed9d>`(const std::string& value);
	
		virtual
		bool
		:target:`accepts_missing_value<doxid-classtoolbox_1_1utils_1_1option__t_1afa56eaf9813d1d93373ad83d45e8e068>`() const;
	
		virtual
		bool
		:target:`is_option<doxid-classtoolbox_1_1utils_1_1option__t_1aed2676dcaa4e469230bf28040dda2456>`() const;
	};

	// direct descendants

	class :ref:`flag_t<doxid-classtoolbox_1_1utils_1_1flag__t>`;

Inherited Members
-----------------

.. ref-code-block:: cpp
	:class: doxyrest-overview-inherited-code-block

	public:
		// methods
	
		std::string
		:ref:`get_name<doxid-classtoolbox_1_1utils_1_1parameter__t_1ad295aeea05f5ecdf8a1e95a23629c050>`() const;
	
		std::string
		:ref:`get_description<doxid-classtoolbox_1_1utils_1_1parameter__t_1add04a6d135dca61cca2a558209fca301>`() const;
	
		bool
		:ref:`is_required<doxid-classtoolbox_1_1utils_1_1parameter__t_1ad2604d826c76ce17f0abda5a09ed8c69>`() const;
	
		bool
		:ref:`is_set<doxid-classtoolbox_1_1utils_1_1parameter__t_1a218823735f8a81af7bf2cbb00c7f1f1e>`() const;
	
		virtual
		bool
		:ref:`parse<doxid-classtoolbox_1_1utils_1_1parameter__t_1a9d30f04fd949c09a712d2564acf5eb39>`(const std::string& value) = 0;
	
		virtual
		bool
		:ref:`accepts_missing_value<doxid-classtoolbox_1_1utils_1_1parameter__t_1aca3a378b667d23cd94e879ae51204de4>`() const;
	
		virtual
		bool
		:ref:`is_option<doxid-classtoolbox_1_1utils_1_1parameter__t_1a9ddf9fd9104e7ae26fcac73499a1b453>`() const;
	
		virtual
		bool
		:ref:`is_argument<doxid-classtoolbox_1_1utils_1_1parameter__t_1a68091176259a3811efa190bf6158d4a7>`() const;
	
		virtual
		bool
		:ref:`is_flag<doxid-classtoolbox_1_1utils_1_1parameter__t_1aa29360aa4741572bbf3757830e2e379f>`() const;
	
		virtual
		std::string
		:ref:`get_short_name<doxid-classtoolbox_1_1utils_1_1parameter__t_1a01d024edc8d73ed0bf1dbc415f3154ff>`() const;

	protected:
		// fields
	
		std::string :ref:`name_<doxid-classtoolbox_1_1utils_1_1parameter__t_1a5450c4b69af915e04a45431b6e3f23a8>`;
		std::string :ref:`description_<doxid-classtoolbox_1_1utils_1_1parameter__t_1a82d7153e79b8b3f66c72e7c7713f108d>`;
		bool :ref:`required_<doxid-classtoolbox_1_1utils_1_1parameter__t_1aedd279b88390ca7fa43ffa307c32914c>`;
		bool :ref:`is_set_<doxid-classtoolbox_1_1utils_1_1parameter__t_1abed65262a722d13905b0eb3f0dee8399>`;


.. index:: pair: class; toolbox::utils::parameter_t
.. _doxid-classtoolbox_1_1utils_1_1parameter__t:

class toolbox::utils::parameter_t
=================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click.hpp>
	
	class parameter_t {
	public:
		// construction
	
		:target:`parameter_t<doxid-classtoolbox_1_1utils_1_1parameter__t_1a216dba492f5cb3b26be8218142883a57>`(
			const std::string& name,
			const std::string& description,
			bool required = false
		);
		virtual
		:target:`~parameter_t<doxid-classtoolbox_1_1utils_1_1parameter__t_1a77df61793825459d52e84566e4ee381f>`();

		// methods
	
		std::string
		:target:`get_name<doxid-classtoolbox_1_1utils_1_1parameter__t_1ad295aeea05f5ecdf8a1e95a23629c050>`() const;
	
		std::string
		:target:`get_description<doxid-classtoolbox_1_1utils_1_1parameter__t_1add04a6d135dca61cca2a558209fca301>`() const;
	
		bool
		:target:`is_required<doxid-classtoolbox_1_1utils_1_1parameter__t_1ad2604d826c76ce17f0abda5a09ed8c69>`() const;
	
		bool
		:target:`is_set<doxid-classtoolbox_1_1utils_1_1parameter__t_1a218823735f8a81af7bf2cbb00c7f1f1e>`() const;
	
		virtual
		bool
		:target:`parse<doxid-classtoolbox_1_1utils_1_1parameter__t_1a9d30f04fd949c09a712d2564acf5eb39>`(const std::string& value) = 0;
	
		virtual
		bool
		:target:`accepts_missing_value<doxid-classtoolbox_1_1utils_1_1parameter__t_1aca3a378b667d23cd94e879ae51204de4>`() const;
	
		virtual
		bool
		:target:`is_option<doxid-classtoolbox_1_1utils_1_1parameter__t_1a9ddf9fd9104e7ae26fcac73499a1b453>`() const;
	
		virtual
		bool
		:target:`is_argument<doxid-classtoolbox_1_1utils_1_1parameter__t_1a68091176259a3811efa190bf6158d4a7>`() const;
	
		virtual
		bool
		:target:`is_flag<doxid-classtoolbox_1_1utils_1_1parameter__t_1aa29360aa4741572bbf3757830e2e379f>`() const;
	
		virtual
		std::string
		:target:`get_short_name<doxid-classtoolbox_1_1utils_1_1parameter__t_1a01d024edc8d73ed0bf1dbc415f3154ff>`() const;

	protected:
		// fields
	
		std::string :target:`name_<doxid-classtoolbox_1_1utils_1_1parameter__t_1a5450c4b69af915e04a45431b6e3f23a8>`;
		std::string :target:`description_<doxid-classtoolbox_1_1utils_1_1parameter__t_1a82d7153e79b8b3f66c72e7c7713f108d>`;
		bool :target:`required_<doxid-classtoolbox_1_1utils_1_1parameter__t_1aedd279b88390ca7fa43ffa307c32914c>`;
		bool :target:`is_set_<doxid-classtoolbox_1_1utils_1_1parameter__t_1abed65262a722d13905b0eb3f0dee8399>`;
	};

	// direct descendants

	template <typename T>
	class :ref:`option_t<doxid-classtoolbox_1_1utils_1_1option__t>`;

	template <typename T>
	class :ref:`argument_t<doxid-classtoolbox_1_1utils_1_1argument__t>`;

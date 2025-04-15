.. index:: pair: class; toolbox::utils::argument_t
.. _doxid-classtoolbox_1_1utils_1_1argument__t:

template class toolbox::utils::argument_t
=========================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <click.hpp>
	
	template <typename T>
	class argument_t: public :ref:`toolbox::utils::parameter_t<doxid-classtoolbox_1_1utils_1_1parameter__t>` {
	public:
		// construction
	
		:target:`argument_t<doxid-classtoolbox_1_1utils_1_1argument__t_1abd4140876e1a410c138eb2bd9e0d0803>`(
			const std::string& name,
			const std::string& description,
			bool required = true
		);

		// methods
	
		argument_t&
		:target:`set_default<doxid-classtoolbox_1_1utils_1_1argument__t_1aec03aa91fb828e28d2672736db8a83e3>`(const T& default_value);
	
		T
		:target:`get<doxid-classtoolbox_1_1utils_1_1argument__t_1ac6843912d9a9648b625afd0556480d08>`() const;
	
		virtual
		bool
		:target:`parse<doxid-classtoolbox_1_1utils_1_1argument__t_1a9af939268ff1ea9d2dd4af8899f328ef>`(const std::string& value);
	
		virtual
		bool
		:target:`accepts_missing_value<doxid-classtoolbox_1_1utils_1_1argument__t_1ab1ae484b3befdc3c25759da1903ede6e>`() const;
	
		virtual
		bool
		:target:`is_argument<doxid-classtoolbox_1_1utils_1_1argument__t_1ac379f220df895f9e7ea4f72a8cea88f5>`() const;
	
		virtual
		std::string
		:target:`get_short_name<doxid-classtoolbox_1_1utils_1_1argument__t_1aed518a812b9d460b1a5c81081b91543b>`() const;
	};

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


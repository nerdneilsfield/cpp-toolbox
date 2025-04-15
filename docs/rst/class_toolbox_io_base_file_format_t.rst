.. index:: pair: class; toolbox::io::base_file_format_t
.. _doxid-classtoolbox_1_1io_1_1base__file__format__t:

class toolbox::io::base_file_format_t
=====================================

.. toctree::
	:hidden:




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <base.hpp>
	
	class base_file_format_t {
	public:
		// construction
	
		:target:`base_file_format_t<doxid-classtoolbox_1_1io_1_1base__file__format__t_1af52281ea5be78a8ff977ad15813aa162>`(const base_file_format_t&);
		:target:`base_file_format_t<doxid-classtoolbox_1_1io_1_1base__file__format__t_1a446aa5f85ed14c0d5b6d3b3c3da87769>`(base_file_format_t&&);
		virtual
		:target:`~base_file_format_t<doxid-classtoolbox_1_1io_1_1base__file__format__t_1a8b54328057f4f11cee8dc66aed628a25>`();

		// methods
	
		base_file_format_t&
		:target:`operator=<doxid-classtoolbox_1_1io_1_1base__file__format__t_1aa9f9b8c55073fb1224a5eb93caddfba4>`(const base_file_format_t&);
	
		base_file_format_t&
		:target:`operator=<doxid-classtoolbox_1_1io_1_1base__file__format__t_1a66fb94bed04a7c57e94d4fc9611cdccb>`(base_file_format_t&&);
	
		virtual
		auto
		:target:`can_read<doxid-classtoolbox_1_1io_1_1base__file__format__t_1ad43d755601268004591b811e6cdc340b>`(const std::string& path) const = 0;
	
		virtual
		auto
		:target:`get_supported_extensions<doxid-classtoolbox_1_1io_1_1base__file__format__t_1aa2a7dd3795a2d988923c3e18719feb46>`() const = 0;
	
		virtual
		auto
		:target:`read<doxid-classtoolbox_1_1io_1_1base__file__format__t_1afb34b2c84b1006dcdd095f997b852491>`(
			const std::string& path,
			std::unique_ptr<:ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`>& data
		) = 0;
	
		virtual
		auto
		:target:`write<doxid-classtoolbox_1_1io_1_1base__file__format__t_1ac32ebb3ec2dfa64ebf5ed9aaf71a17ff>`(
			const std::string& path,
			const std::unique_ptr<:ref:`base_file_data_t<doxid-classtoolbox_1_1io_1_1base__file__data__t>`>& data
		) const = 0;

	protected:
		// construction
	
		:target:`base_file_format_t<doxid-classtoolbox_1_1io_1_1base__file__format__t_1ab495e6e2cdc4802c6b7e015b145c1ac4>`();
	};

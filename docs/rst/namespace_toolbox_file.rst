.. index:: pair: namespace; toolbox::file
.. _doxid-namespacetoolbox_1_1file:

namespace toolbox::file
=======================

.. toctree::
	:hidden:

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace file {

	// typedefs

	typedef std::filesystem::file_time_type :target:`file_time_type<doxid-namespacetoolbox_1_1file_1a9d370d4f448be574533574f643bc1298>`;
	typedef std::filesystem::file_type :target:`file_type<doxid-namespacetoolbox_1_1file_1ab939fdda9a10c744810dcab68833b841>`;

	// global functions

	CPP_TOOLBOX_EXPORT auto
	:ref:`string_to_path<doxid-namespacetoolbox_1_1file_1a60f6004279fd475e4d1951e177b46c69>`(const std::string& str);

	CPP_TOOLBOX_EXPORT auto
	:ref:`path_to_string<doxid-namespacetoolbox_1_1file_1aa5ae9f96e511dc7feafcb507ad1f6c31>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_file_size<doxid-namespacetoolbox_1_1file_1a1137011f0ab9390578e201f5df1a5d4c>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_absolute_path<doxid-namespacetoolbox_1_1file_1a85dd8c020456e11f9e8217aa3941216e>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_relative_path<doxid-namespacetoolbox_1_1file_1aa193ebd895ae2c8602885121f1e8c72d>`(
		const std::filesystem::path& path,
		const std::filesystem::path& base
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_parent_path<doxid-namespacetoolbox_1_1file_1ab2785348e22ba0c2a203ccca4890aa00>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_file_name<doxid-namespacetoolbox_1_1file_1a94d0f2a1bed7286d6779f57c7a016781>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_file_extension<doxid-namespacetoolbox_1_1file_1ac2b9f2a5b7df4bfb3e2afc92486f02fc>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`copy_file<doxid-namespacetoolbox_1_1file_1aac210d89b1411dada1371a94f0b64ebd>`(
		const std::filesystem::path& src,
		const std::filesystem::path& dst
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`move_file<doxid-namespacetoolbox_1_1file_1a3f5444c60648ca97bfa4b21c0b7dc09b>`(
		const std::filesystem::path& src,
		const std::filesystem::path& dst
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`delete_file<doxid-namespacetoolbox_1_1file_1ac6e643e4658fd230eced397b5b6adeb0>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`file_exists<doxid-namespacetoolbox_1_1file_1a46034cf680e496550ad944f62ba49a64>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`create_directory<doxid-namespacetoolbox_1_1file_1ae4c9fa4b7d4d61845edf6f7270b597ab>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`directory_exists<doxid-namespacetoolbox_1_1file_1a3f58adf72e4c7d32fa0d4b1c86eb6eb1>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`delete_directory<doxid-namespacetoolbox_1_1file_1a433c83e6efcc1c9482e518a28dd47632>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_current_directory<doxid-namespacetoolbox_1_1file_1a18c5527a6ca0799d2bf7141f88fdf8ce>`();

	CPP_TOOLBOX_EXPORT auto
	:ref:`path_type<doxid-namespacetoolbox_1_1file_1ae9e38d49fe1aa7ebeef6139c908e5f15>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_regular_file<doxid-namespacetoolbox_1_1file_1a43823bbfcad90d86c042e3627791ffaa>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_directory<doxid-namespacetoolbox_1_1file_1aa25c437b3e41a9a5ef2860f9b7c2012f>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_symlink<doxid-namespacetoolbox_1_1file_1aebd2ce80a8b6a1e94b58b9b7ab0dc6a8>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_fifo<doxid-namespacetoolbox_1_1file_1a18d000663dbb8b2ddcbb88c613cdac2b>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_socket<doxid-namespacetoolbox_1_1file_1a9ac1ee2d33be226474ce83fb1d5df0c4>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_block_file<doxid-namespacetoolbox_1_1file_1a71e1f7c73c64935f5c3c1cdb0003da67>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_character_file<doxid-namespacetoolbox_1_1file_1ab07f7f5501955c1019ba3536ba0973e1>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_other<doxid-namespacetoolbox_1_1file_1a5baf94bae1c814008785e16d5925a93d>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`is_empty<doxid-namespacetoolbox_1_1file_1a72b627127638d1708e29fb365e4c1f1c>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_creation_time<doxid-namespacetoolbox_1_1file_1ad3979fb549eb4b46b46c71b829bab0c1>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_last_access_time<doxid-namespacetoolbox_1_1file_1a3833d3e8d3e54e2dadd40cbdbe412d3e>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_last_write_time<doxid-namespacetoolbox_1_1file_1a98028c1a5c72cc4c4b8b4c0f9e6d4af6>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_symlink_target<doxid-namespacetoolbox_1_1file_1a5edf9150543eb7a956ea0eee18c87f20>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`traverse_directory<doxid-namespacetoolbox_1_1file_1aa4085a2e77b2b2c55e81b3161960c55d>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`recursive_traverse_directory<doxid-namespacetoolbox_1_1file_1a131b4596d21d7af9c3a20625c2b84f7b>`(const std::filesystem::path& path);

	CPP_TOOLBOX_EXPORT auto
	:ref:`traverse_directory_files<doxid-namespacetoolbox_1_1file_1a77fa6ba4b9d31a41208fa4bf7b98ee0c>`(
		const std::filesystem::path& path,
		std::function<void(const std::filesystem::path&)>& callback
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`recursive_traverse_directory_files<doxid-namespacetoolbox_1_1file_1a61389af42cef158903ec3cdd1487837d>`(
		const std::filesystem::path& path,
		std::function<void(const std::filesystem::path&)>& callback
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_current_working_directory<doxid-namespacetoolbox_1_1file_1ab446410f8d2e28ef4709e79c6f3192c9>`();

	} // namespace file
.. _details-namespacetoolbox_1_1file:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; string_to_path
.. _doxid-namespacetoolbox_1_1file_1a60f6004279fd475e4d1951e177b46c69:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	string_to_path(const std::string& str)

Convert a string to a filesystem path.

.. ref-code-block:: cpp

	// Basic usage
	auto path = :ref:`string_to_path <doxid-namespacetoolbox_1_1file_1a60f6004279fd475e4d1951e177b46c69>`("/usr/local/bin");
	
	// Windows path conversion
	auto win_path = :ref:`string_to_path <doxid-namespacetoolbox_1_1file_1a60f6004279fd475e4d1951e177b46c69>`("C:\\Program Files\\MyApp");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- str

		- The string to convert to a path



.. rubric:: Returns:

std::filesystem::path The converted path object

.. index:: pair: function; path_to_string
.. _doxid-namespacetoolbox_1_1file_1aa5ae9f96e511dc7feafcb507ad1f6c31:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	path_to_string(const std::filesystem::path& path)

Convert a filesystem path to a string.

.. ref-code-block:: cpp

	// Basic usage
	std::filesystem::path p = "/usr/local/bin";
	auto str = :ref:`path_to_string <doxid-namespacetoolbox_1_1file_1aa5ae9f96e511dc7feafcb507ad1f6c31>`(p);  // Returns "/usr/local/bin"
	
	// Windows path conversion
	std::filesystem::path win_p = "C:\\Program Files\\MyApp";
	auto win_str = :ref:`path_to_string <doxid-namespacetoolbox_1_1file_1aa5ae9f96e511dc7feafcb507ad1f6c31>`(win_p);  // Returns "C:\\Program Files\\MyApp"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to convert to a string



.. rubric:: Returns:

std::string The string representation of the path

.. index:: pair: function; get_file_size
.. _doxid-namespacetoolbox_1_1file_1a1137011f0ab9390578e201f5df1a5d4c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_file_size(const std::filesystem::path& path)

Get the size of a file in bytes.

.. ref-code-block:: cpp

	// Get size of existing file
	try {
	    auto size = :ref:`get_file_size <doxid-namespacetoolbox_1_1file_1a1137011f0ab9390578e201f5df1a5d4c>`("/path/to/file.txt");
	    std::cout << "File size: " << size << " bytes\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to the file

	*
		- std::filesystem::filesystem_error

		- if the file doesn't exist or can't be accessed



.. rubric:: Returns:

std::size_t The size of the file in bytes

.. index:: pair: function; get_absolute_path
.. _doxid-namespacetoolbox_1_1file_1a85dd8c020456e11f9e8217aa3941216e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_absolute_path(const std::filesystem::path& path)

Get the absolute path of a file or directory.

.. ref-code-block:: cpp

	// Get absolute path of relative path
	auto abs_path = :ref:`get_absolute_path <doxid-namespacetoolbox_1_1file_1a85dd8c020456e11f9e8217aa3941216e>`("relative/path");
	
	// Get absolute path of current directory
	auto current_dir = :ref:`get_absolute_path <doxid-namespacetoolbox_1_1file_1a85dd8c020456e11f9e8217aa3941216e>`(".");



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to convert to absolute



.. rubric:: Returns:

std::filesystem::path The absolute path

.. index:: pair: function; get_relative_path
.. _doxid-namespacetoolbox_1_1file_1aa193ebd895ae2c8602885121f1e8c72d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_relative_path(
		const std::filesystem::path& path,
		const std::filesystem::path& base
	)

Get the relative path between two paths.

.. ref-code-block:: cpp

	// Get relative path between two directories
	auto rel_path = :ref:`get_relative_path <doxid-namespacetoolbox_1_1file_1aa193ebd895ae2c8602885121f1e8c72d>`("/usr/local/bin", "/usr");
	// rel_path will be "local/bin"
	
	// Get relative path between files
	auto file_rel = :ref:`get_relative_path <doxid-namespacetoolbox_1_1file_1aa193ebd895ae2c8602885121f1e8c72d>`("/a/b/c.txt", "/a/d/e.txt");
	// file_rel will be "../b/c.txt"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The target path

	*
		- base

		- The base path to compute relative path from



.. rubric:: Returns:

std::filesystem::path The relative path from base to target

.. index:: pair: function; get_parent_path
.. _doxid-namespacetoolbox_1_1file_1ab2785348e22ba0c2a203ccca4890aa00:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_parent_path(const std::filesystem::path& path)

Get the parent directory of a path.

.. ref-code-block:: cpp

	// Get parent directory of file
	auto parent = :ref:`get_parent_path <doxid-namespacetoolbox_1_1file_1ab2785348e22ba0c2a203ccca4890aa00>`("/usr/local/bin/gcc");
	// parent will be "/usr/local/bin"
	
	// Get parent directory of directory
	auto parent_dir = :ref:`get_parent_path <doxid-namespacetoolbox_1_1file_1ab2785348e22ba0c2a203ccca4890aa00>`("/usr/local/bin/");
	// parent_dir will be "/usr/local"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to get the parent of



.. rubric:: Returns:

std::filesystem::path The parent directory path

.. index:: pair: function; get_file_name
.. _doxid-namespacetoolbox_1_1file_1a94d0f2a1bed7286d6779f57c7a016781:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_file_name(const std::filesystem::path& path)

Get the file name component of a path.

.. ref-code-block:: cpp

	// Get file name from full path
	auto name = :ref:`get_file_name <doxid-namespacetoolbox_1_1file_1a94d0f2a1bed7286d6779f57c7a016781>`("/usr/local/bin/gcc");
	// name will be "gcc"
	
	// Get file name from relative path
	auto rel_name = :ref:`get_file_name <doxid-namespacetoolbox_1_1file_1a94d0f2a1bed7286d6779f57c7a016781>`("docs/readme.txt");
	// rel_name will be "readme.txt"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to extract the file name from



.. rubric:: Returns:

std::string The file name component

.. index:: pair: function; get_file_extension
.. _doxid-namespacetoolbox_1_1file_1ac2b9f2a5b7df4bfb3e2afc92486f02fc:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_file_extension(const std::filesystem::path& path)

Get the file extension of a path.

.. ref-code-block:: cpp

	// Get extension from file path
	auto ext = :ref:`get_file_extension <doxid-namespacetoolbox_1_1file_1ac2b9f2a5b7df4bfb3e2afc92486f02fc>`("/path/to/document.pdf");
	// ext will be ".pdf"
	
	// Get extension from file name
	auto ext2 = :ref:`get_file_extension <doxid-namespacetoolbox_1_1file_1ac2b9f2a5b7df4bfb3e2afc92486f02fc>`("archive.tar.gz");
	// ext2 will be ".gz"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to extract the extension from



.. rubric:: Returns:

std::string The file extension including the dot

.. index:: pair: function; copy_file
.. _doxid-namespacetoolbox_1_1file_1aac210d89b1411dada1371a94f0b64ebd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	copy_file(
		const std::filesystem::path& src,
		const std::filesystem::path& dst
	)

Copy a file.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- src

		- The source file

	*
		- dst

		- The destination file



.. rubric:: Returns:

True if the file was copied successfully

.. index:: pair: function; move_file
.. _doxid-namespacetoolbox_1_1file_1a3f5444c60648ca97bfa4b21c0b7dc09b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	move_file(
		const std::filesystem::path& src,
		const std::filesystem::path& dst
	)

Move a file.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- src

		- The source file

	*
		- dst

		- The destination file



.. rubric:: Returns:

True if the file was moved successfully

.. index:: pair: function; delete_file
.. _doxid-namespacetoolbox_1_1file_1ac6e643e4658fd230eced397b5b6adeb0:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	delete_file(const std::filesystem::path& path)

Delete a file at the specified path.

.. ref-code-block:: cpp

	// Delete an existing file
	if (:ref:`delete_file <doxid-namespacetoolbox_1_1file_1ac6e643e4658fd230eced397b5b6adeb0>`("/tmp/test.txt")) {
	    std::cout << "File deleted successfully\n";
	} else {
	    std::cerr << "Failed to delete file\n";
	}
	
	// Attempt to delete non-existent file
	bool result = :ref:`delete_file <doxid-namespacetoolbox_1_1file_1ac6e643e4658fd230eced397b5b6adeb0>`("/path/to/nonexistent.file");
	// result will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path to the file to delete



.. rubric:: Returns:

True if the file was successfully deleted, false otherwise

.. index:: pair: function; file_exists
.. _doxid-namespacetoolbox_1_1file_1a46034cf680e496550ad944f62ba49a64:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	file_exists(const std::filesystem::path& path)

Check if a file exists at the specified path.

.. ref-code-block:: cpp

	// Check if file exists
	if (:ref:`file_exists <doxid-namespacetoolbox_1_1file_1a46034cf680e496550ad944f62ba49a64>`("/etc/passwd")) {
	    std::cout << "File exists\n";
	} else {
	    std::cout << "File does not exist\n";
	}
	
	// Check non-existent file
	bool exists = :ref:`file_exists <doxid-namespacetoolbox_1_1file_1a46034cf680e496550ad944f62ba49a64>`("/tmp/nonexistent.file");
	// exists will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path to check



.. rubric:: Returns:

True if the file exists, false otherwise

.. index:: pair: function; create_directory
.. _doxid-namespacetoolbox_1_1file_1ae4c9fa4b7d4d61845edf6f7270b597ab:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	create_directory(const std::filesystem::path& path)

Create a directory at the specified path.

.. ref-code-block:: cpp

	// Create a new directory
	if (:ref:`create_directory <doxid-namespacetoolbox_1_1file_1ae4c9fa4b7d4d61845edf6f7270b597ab>`("/tmp/new_dir")) {
	    std::cout << "Directory created\n";
	} else {
	    std::cerr << "Failed to create directory\n";
	}
	
	// Create nested directories
	:ref:`create_directory <doxid-namespacetoolbox_1_1file_1ae4c9fa4b7d4d61845edf6f7270b597ab>`("/tmp/a/b/c");  // Creates all directories in path



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path where the directory should be created



.. rubric:: Returns:

True if the directory was created successfully, false otherwise

.. index:: pair: function; directory_exists
.. _doxid-namespacetoolbox_1_1file_1a3f58adf72e4c7d32fa0d4b1c86eb6eb1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	directory_exists(const std::filesystem::path& path)

Check if a directory exists at the specified path.

.. ref-code-block:: cpp

	// Check if directory exists
	if (:ref:`directory_exists <doxid-namespacetoolbox_1_1file_1a3f58adf72e4c7d32fa0d4b1c86eb6eb1>`("/usr/local/bin")) {
	    std::cout << "Directory exists\n";
	} else {
	    std::cout << "Directory does not exist\n";
	}
	
	// Check non-existent directory
	bool exists = :ref:`directory_exists <doxid-namespacetoolbox_1_1file_1a3f58adf72e4c7d32fa0d4b1c86eb6eb1>`("/tmp/nonexistent_dir");
	// exists will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path to check



.. rubric:: Returns:

True if the directory exists, false otherwise

.. index:: pair: function; delete_directory
.. _doxid-namespacetoolbox_1_1file_1a433c83e6efcc1c9482e518a28dd47632:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	delete_directory(const std::filesystem::path& path)

Delete a directory at the specified path.

.. ref-code-block:: cpp

	// Delete an empty directory
	if (:ref:`delete_directory <doxid-namespacetoolbox_1_1file_1a433c83e6efcc1c9482e518a28dd47632>`("/tmp/empty_dir")) {
	    std::cout << "Directory deleted\n";
	} else {
	    std::cerr << "Failed to delete directory\n";
	}
	
	// Delete non-empty directory
	:ref:`delete_directory <doxid-namespacetoolbox_1_1file_1a433c83e6efcc1c9482e518a28dd47632>`("/tmp/non_empty_dir");  // Will fail unless recursive



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path of the directory to delete



.. rubric:: Returns:

True if the directory was deleted successfully, false otherwise

.. index:: pair: function; get_current_directory
.. _doxid-namespacetoolbox_1_1file_1a18c5527a6ca0799d2bf7141f88fdf8ce:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_current_directory()

Get the current working directory.

.. ref-code-block:: cpp

	// Print current directory
	auto current_dir = :ref:`get_current_directory <doxid-namespacetoolbox_1_1file_1a18c5527a6ca0799d2bf7141f88fdf8ce>`();
	std::cout << "Current directory: " << current_dir << "\n";
	
	// Change and get current directory
	std::filesystem::current_path("/tmp");
	auto dir = :ref:`get_current_directory <doxid-namespacetoolbox_1_1file_1a18c5527a6ca0799d2bf7141f88fdf8ce>`();  // Returns "/tmp"



.. rubric:: Returns:

The current working directory as a filesystem path

.. index:: pair: function; path_type
.. _doxid-namespacetoolbox_1_1file_1ae9e38d49fe1aa7ebeef6139c908e5f15:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	path_type(const std::filesystem::path& path)

Get the type of the filesystem object at the specified path.

.. ref-code-block:: cpp

	// Check type of a file
	auto type = :ref:`path_type <doxid-namespacetoolbox_1_1file_1ae9e38d49fe1aa7ebeef6139c908e5f15>`("/etc/passwd");
	if (type == std::filesystem::file_type::regular) {
	    std::cout << "Regular file\n";
	}
	
	// Check type of a directory
	auto dir_type = :ref:`path_type <doxid-namespacetoolbox_1_1file_1ae9e38d49fe1aa7ebeef6139c908e5f15>`("/usr/local/bin");
	if (dir_type == std::filesystem::file_type::directory) {
	    std::cout << "Directory\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The filesystem path to check



.. rubric:: Returns:

The type of the filesystem object (file, directory, etc.)

.. index:: pair: function; is_regular_file
.. _doxid-namespacetoolbox_1_1file_1a43823bbfcad90d86c042e3627791ffaa:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_regular_file(const std::filesystem::path& path)

Check if a path is a regular file.

.. ref-code-block:: cpp

	// Check if a file exists and is regular
	if (:ref:`is_regular_file <doxid-namespacetoolbox_1_1file_1a43823bbfcad90d86c042e3627791ffaa>`("/path/to/file.txt")) {
	    std::cout << "Regular file exists\n";
	} else {
	    std::cerr << "Not a regular file\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a regular file, false otherwise

.. index:: pair: function; is_directory
.. _doxid-namespacetoolbox_1_1file_1aa25c437b3e41a9a5ef2860f9b7c2012f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_directory(const std::filesystem::path& path)

Check if a path is a directory.

.. ref-code-block:: cpp

	// Check if a directory exists
	if (:ref:`is_directory <doxid-namespacetoolbox_1_1file_1aa25c437b3e41a9a5ef2860f9b7c2012f>`("/path/to/dir")) {
	    std::cout << "Directory exists\n";
	} else {
	    std::cerr << "Not a directory\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a directory, false otherwise

.. index:: pair: function; is_symlink
.. _doxid-namespacetoolbox_1_1file_1aebd2ce80a8b6a1e94b58b9b7ab0dc6a8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_symlink(const std::filesystem::path& path)

Check if a path is a symbolic link.

.. ref-code-block:: cpp

	// Check if a path is a symlink
	if (:ref:`is_symlink <doxid-namespacetoolbox_1_1file_1aebd2ce80a8b6a1e94b58b9b7ab0dc6a8>`("/path/to/symlink")) {
	    std::cout << "Symbolic link detected\n";
	} else {
	    std::cerr << "Not a symbolic link\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a symbolic link, false otherwise

.. index:: pair: function; is_fifo
.. _doxid-namespacetoolbox_1_1file_1a18d000663dbb8b2ddcbb88c613cdac2b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_fifo(const std::filesystem::path& path)

Check if a path is a FIFO (named pipe)

.. ref-code-block:: cpp

	// Check if a path is a FIFO
	if (:ref:`is_fifo <doxid-namespacetoolbox_1_1file_1a18d000663dbb8b2ddcbb88c613cdac2b>`("/path/to/fifo")) {
	    std::cout << "FIFO detected\n";
	} else {
	    std::cerr << "Not a FIFO\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a FIFO, false otherwise

.. index:: pair: function; is_socket
.. _doxid-namespacetoolbox_1_1file_1a9ac1ee2d33be226474ce83fb1d5df0c4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_socket(const std::filesystem::path& path)

Check if a path is a socket.

.. ref-code-block:: cpp

	// Check if a path is a socket
	if (:ref:`is_socket <doxid-namespacetoolbox_1_1file_1a9ac1ee2d33be226474ce83fb1d5df0c4>`("/path/to/socket")) {
	    std::cout << "Socket detected\n";
	} else {
	    std::cerr << "Not a socket\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a socket, false otherwise

.. index:: pair: function; is_block_file
.. _doxid-namespacetoolbox_1_1file_1a71e1f7c73c64935f5c3c1cdb0003da67:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_block_file(const std::filesystem::path& path)

Check if a path is a block device.

.. ref-code-block:: cpp

	// Check if a path is a block device
	if (:ref:`is_block_file <doxid-namespacetoolbox_1_1file_1a71e1f7c73c64935f5c3c1cdb0003da67>`("/dev/sda1")) {
	    std::cout << "Block device detected\n";
	} else {
	    std::cerr << "Not a block device\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a block device, false otherwise

.. index:: pair: function; is_character_file
.. _doxid-namespacetoolbox_1_1file_1ab07f7f5501955c1019ba3536ba0973e1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_character_file(const std::filesystem::path& path)

Check if a path is a character device.

.. ref-code-block:: cpp

	// Check if a path is a character device
	if (:ref:`is_character_file <doxid-namespacetoolbox_1_1file_1ab07f7f5501955c1019ba3536ba0973e1>`("/dev/tty")) {
	    std::cout << "Character device detected\n";
	} else {
	    std::cerr << "Not a character device\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is a character device, false otherwise

.. index:: pair: function; is_other
.. _doxid-namespacetoolbox_1_1file_1a5baf94bae1c814008785e16d5925a93d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_other(const std::filesystem::path& path)

Check if a path is of an unknown type.

.. ref-code-block:: cpp

	// Check if a path is of unknown type
	if (:ref:`is_other <doxid-namespacetoolbox_1_1file_1a5baf94bae1c814008785e16d5925a93d>`("/path/to/unknown")) {
	    std::cout << "Unknown file type detected\n";
	} else {
	    std::cerr << "File type is known\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is of an unknown type, false otherwise

.. index:: pair: function; is_empty
.. _doxid-namespacetoolbox_1_1file_1a72b627127638d1708e29fb365e4c1f1c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	is_empty(const std::filesystem::path& path)

Check if a path is empty.

.. ref-code-block:: cpp

	// Check if a directory is empty
	if (:ref:`is_empty <doxid-namespacetoolbox_1_1file_1a72b627127638d1708e29fb365e4c1f1c>`("/path/to/dir")) {
	    std::cout << "Directory is empty\n";
	} else {
	    std::cerr << "Directory is not empty\n";
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check



.. rubric:: Returns:

True if the path is empty, false otherwise

.. index:: pair: function; get_creation_time
.. _doxid-namespacetoolbox_1_1file_1ad3979fb549eb4b46b46c71b829bab0c1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_creation_time(const std::filesystem::path& path)

Get the creation time of a file.

.. ref-code-block:: cpp

	// Get creation time of a file
	try {
	    auto creation_time = :ref:`get_creation_time <doxid-namespacetoolbox_1_1file_1ad3979fb549eb4b46b46c71b829bab0c1>`("/path/to/file.txt");
	    std::cout << "File created at: " << creation_time << "\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check

	*
		- std::filesystem::filesystem_error

		- if the file doesn't exist or can't be accessed



.. rubric:: Returns:

The creation time of the file

.. index:: pair: function; get_last_access_time
.. _doxid-namespacetoolbox_1_1file_1a3833d3e8d3e54e2dadd40cbdbe412d3e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_last_access_time(const std::filesystem::path& path)

Get the last access time of a file.

.. ref-code-block:: cpp

	// Get last access time of a file
	try {
	    auto access_time = :ref:`get_last_access_time <doxid-namespacetoolbox_1_1file_1a3833d3e8d3e54e2dadd40cbdbe412d3e>`("/path/to/file.txt");
	    std::cout << "Last accessed: " << access_time << "\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check

	*
		- std::filesystem::filesystem_error

		- if the file doesn't exist or can't be accessed



.. rubric:: Returns:

The last access time as file_time_type

.. index:: pair: function; get_last_write_time
.. _doxid-namespacetoolbox_1_1file_1a98028c1a5c72cc4c4b8b4c0f9e6d4af6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_last_write_time(const std::filesystem::path& path)

Get the last write time of a file.

.. ref-code-block:: cpp

	// Get last modification time of a file
	try {
	    auto write_time = :ref:`get_last_write_time <doxid-namespacetoolbox_1_1file_1a98028c1a5c72cc4c4b8b4c0f9e6d4af6>`("/path/to/file.txt");
	    std::cout << "Last modified: " << write_time << "\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to check

	*
		- std::filesystem::filesystem_error

		- if the file doesn't exist or can't be accessed



.. rubric:: Returns:

The last write time as file_time_type

.. index:: pair: function; get_symlink_target
.. _doxid-namespacetoolbox_1_1file_1a5edf9150543eb7a956ea0eee18c87f20:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_symlink_target(const std::filesystem::path& path)

Get the target of a symbolic link.

.. ref-code-block:: cpp

	// Get symlink target
	try {
	    auto target = :ref:`get_symlink_target <doxid-namespacetoolbox_1_1file_1a5edf9150543eb7a956ea0eee18c87f20>`("/path/to/symlink");
	    std::cout << "Symlink points to: " << target << "\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to the symbolic link

	*
		- std::filesystem::filesystem_error

		- if the path is not a symbolic link



.. rubric:: Returns:

The target path as std::filesystem::path

.. index:: pair: function; traverse_directory
.. _doxid-namespacetoolbox_1_1file_1aa4085a2e77b2b2c55e81b3161960c55d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	traverse_directory(const std::filesystem::path& path)

Traverse a directory.

.. ref-code-block:: cpp

	// List files in a directory
	try {
	    auto files = :ref:`traverse_directory <doxid-namespacetoolbox_1_1file_1aa4085a2e77b2b2c55e81b3161960c55d>`("/path/to/dir");
	    for (const auto& file : files) {
	        std::cout << file << "\n";
	    }
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to traverse

	*
		- std::filesystem::filesystem_error

		- if the path is not a directory



.. rubric:: Returns:

A vector of paths in the directory

.. index:: pair: function; recursive_traverse_directory
.. _doxid-namespacetoolbox_1_1file_1a131b4596d21d7af9c3a20625c2b84f7b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	recursive_traverse_directory(const std::filesystem::path& path)

Recursively traverse a directory.

.. ref-code-block:: cpp

	// Recursively list files in a directory
	try {
	    auto all_files = :ref:`recursive_traverse_directory <doxid-namespacetoolbox_1_1file_1a131b4596d21d7af9c3a20625c2b84f7b>`("/path/to/dir");
	    for (const auto& file : all_files) {
	        std::cout << file << "\n";
	    }
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to traverse

	*
		- std::filesystem::filesystem_error

		- if the path is not a directory



.. rubric:: Returns:

A vector of paths in the directory and its subdirectories

.. index:: pair: function; traverse_directory_files
.. _doxid-namespacetoolbox_1_1file_1a77fa6ba4b9d31a41208fa4bf7b98ee0c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	traverse_directory_files(
		const std::filesystem::path& path,
		std::function<void(const std::filesystem::path&)>& callback
	)

Traverse a directory and call a callback for each file.

.. ref-code-block:: cpp

	// Process each file in a directory
	try {
	    :ref:`traverse_directory_files <doxid-namespacetoolbox_1_1file_1a77fa6ba4b9d31a41208fa4bf7b98ee0c>`("/path/to/dir", [](const auto& file) {
	        std::cout << "Processing: " << file << "\n";
	    });
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to traverse

	*
		- callback

		- The callback to call for each file

	*
		- std::filesystem::filesystem_error

		- if the path is not a directory

.. index:: pair: function; recursive_traverse_directory_files
.. _doxid-namespacetoolbox_1_1file_1a61389af42cef158903ec3cdd1487837d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	recursive_traverse_directory_files(
		const std::filesystem::path& path,
		std::function<void(const std::filesystem::path&)>& callback
	)

Recursively traverse a directory and call a callback for each file.

.. ref-code-block:: cpp

	// Process each file recursively
	try {
	    :ref:`recursive_traverse_directory_files <doxid-namespacetoolbox_1_1file_1a61389af42cef158903ec3cdd1487837d>`("/path/to/dir", [](const auto& file) {
	        std::cout << "Processing: " << file << "\n";
	    });
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- path

		- The path to traverse

	*
		- callback

		- The callback to call for each file

	*
		- std::filesystem::filesystem_error

		- if the path is not a directory

.. index:: pair: function; get_current_working_directory
.. _doxid-namespacetoolbox_1_1file_1ab446410f8d2e28ef4709e79c6f3192c9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_current_working_directory()

Get the current working directory.

.. ref-code-block:: cpp

	// Print current working directory
	try {
	    auto cwd = :ref:`get_current_working_directory <doxid-namespacetoolbox_1_1file_1ab446410f8d2e28ef4709e79c6f3192c9>`();
	    std::cout << "Current directory: " << cwd << "\n";
	} catch (const std::filesystem::filesystem_error& e) {
	    std::cerr << "Error: " << e.what() << '\n';
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- std::filesystem::filesystem_error

		- if the current directory cannot be determined



.. rubric:: Returns:

The current working directory as std::filesystem::path


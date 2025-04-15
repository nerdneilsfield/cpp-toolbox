.. index:: pair: namespace; toolbox::base
.. _doxid-namespacetoolbox_1_1base:

namespace toolbox::base
=======================

.. toctree::
	:hidden:

	namespace_toolbox_base_detail.rst
	class_toolbox_base_PoolDeleter.rst
	class_toolbox_base_memory_pool_t.rst
	class_toolbox_base_object_pool_t.rst
	class_toolbox_base_thread_pool_singleton_t.rst
	class_toolbox_base_thread_pool_t.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace base {

	// namespaces

	namespace :ref:`toolbox::base::detail<doxid-namespacetoolbox_1_1base_1_1detail>`;

	// classes

	template <typename T>
	class :ref:`PoolDeleter<doxid-classtoolbox_1_1base_1_1PoolDeleter>`;

	class :ref:`memory_pool_t<doxid-classtoolbox_1_1base_1_1memory__pool__t>`;

	template <typename T>
	class :ref:`object_pool_t<doxid-classtoolbox_1_1base_1_1object__pool__t>`;

	class :ref:`thread_pool_singleton_t<doxid-classtoolbox_1_1base_1_1thread__pool__singleton__t>`;
	class :ref:`thread_pool_t<doxid-classtoolbox_1_1base_1_1thread__pool__t>`;

	// global functions

	CPP_TOOLBOX_EXPORT auto
	:ref:`get_environment_variable<doxid-namespacetoolbox_1_1base_1aaf4ba06e16ef767bd29d5b1c9fc760e6>`(const std::string& name);

	CPP_TOOLBOX_EXPORT auto
	:ref:`set_environment_variable<doxid-namespacetoolbox_1_1base_1af8f1b6b2951fa719590c6a8483eaa17e>`(
		const std::string& name,
		const std::string& value
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`remove_environment_variable<doxid-namespacetoolbox_1_1base_1ad492fe465930e627f40feec79b17eb8e>`(const std::string& name);

	} // namespace base
.. _details-namespacetoolbox_1_1base:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; get_environment_variable
.. _doxid-namespacetoolbox_1_1base_1aaf4ba06e16ef767bd29d5b1c9fc760e6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	get_environment_variable(const std::string& name)

Retrieves the value of an environment variable.

This function queries the system environment variables and returns the value associated with the specified name. If the environment variable does not exist, an empty string is returned.

.. ref-code-block:: cpp

	// Get the PATH environment variable
	auto path = :ref:`get_environment_variable <doxid-namespacetoolbox_1_1base_1aaf4ba06e16ef767bd29d5b1c9fc760e6>`("PATH");
	// path will contain the PATH environment variable value
	
	// Get a non-existent variable
	auto value = :ref:`get_environment_variable <doxid-namespacetoolbox_1_1base_1aaf4ba06e16ef767bd29d5b1c9fc760e6>`("NON_EXISTENT_VAR");
	// value will be an empty string



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- name

		- The name of the environment variable to retrieve. Must be non-empty.



.. rubric:: Returns:

The value of the environment variable as a string. Returns an empty string if the variable is not found.

.. index:: pair: function; set_environment_variable
.. _doxid-namespacetoolbox_1_1base_1af8f1b6b2951fa719590c6a8483eaa17e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	set_environment_variable(
		const std::string& name,
		const std::string& value
	)

Sets or updates an environment variable.

This function creates or modifies an environment variable with the specified name and value. The change affects the current process and any child processes spawned after the change.

.. ref-code-block:: cpp

	// Set a new environment variable
	bool success = :ref:`set_environment_variable <doxid-namespacetoolbox_1_1base_1af8f1b6b2951fa719590c6a8483eaa17e>`("MY_VAR", "12345");
	// success will be true if the variable was set
	
	// Update an existing variable
	:ref:`set_environment_variable <doxid-namespacetoolbox_1_1base_1af8f1b6b2951fa719590c6a8483eaa17e>`("PATH", "/new/path:" +
	:ref:`get_environment_variable <doxid-namespacetoolbox_1_1base_1aaf4ba06e16ef767bd29d5b1c9fc760e6>`("PATH"));



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- name

		- The name of the environment variable to set. Must be non-empty.

	*
		- value

		- The value to assign to the environment variable.



.. rubric:: Returns:

True if the environment variable was successfully set, false otherwise. Failure can occur due to invalid input or system limitations.

.. index:: pair: function; remove_environment_variable
.. _doxid-namespacetoolbox_1_1base_1ad492fe465930e627f40feec79b17eb8e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	remove_environment_variable(const std::string& name)

Removes an environment variable from the current process environment.

This function deletes the specified environment variable from the current process's environment. The change affects the current process and any child processes spawned after the change.

.. ref-code-block:: cpp

	// Remove a temporary environment variable
	bool removed = :ref:`remove_environment_variable <doxid-namespacetoolbox_1_1base_1ad492fe465930e627f40feec79b17eb8e>`("TEMP_VAR");
	// removed will be true if the variable existed and was removed
	
	// Attempt to remove a non-existent variable
	bool result = :ref:`remove_environment_variable <doxid-namespacetoolbox_1_1base_1ad492fe465930e627f40feec79b17eb8e>`("NON_EXISTENT_VAR");
	// result will be false



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- name

		- The name of the environment variable to remove. Must be non-empty.



.. rubric:: Returns:

True if the environment variable was successfully removed, false otherwise. Returns false if the variable did not exist or could not be removed.


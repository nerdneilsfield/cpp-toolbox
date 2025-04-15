.. index:: pair: group; Logging Macros
.. _doxid-group__LoggingMacros:

Logging Macros
==============

.. toctree::
	:hidden:

Overview
~~~~~~~~

Macros for convenient logging with different levels and formats. :ref:`More...<details-group__LoggingMacros>`


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	// macros

	#define :ref:`LOG_CRITICAL_D<doxid-group__LoggingMacros_1ga227b917e88f64d9f90c06c54e04c648e>`(x)
	#define :ref:`LOG_CRITICAL_F<doxid-group__LoggingMacros_1gaaa6f7679d883afb22171314099518ee1>`
	#define :ref:`LOG_CRITICAL_S<doxid-group__LoggingMacros_1ga00b710fb8e78bfaa73323b9deb1c43cc>`
	#define :ref:`LOG_DEBUG_D<doxid-group__LoggingMacros_1ga1ba761e7159ad23893e8bb9807d31ead>`(x)
	#define :ref:`LOG_DEBUG_F<doxid-group__LoggingMacros_1ga2eff2cba0a7856857be1df094bebf834>`
	#define :ref:`LOG_DEBUG_S<doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>`
	#define :ref:`LOG_ERROR_D<doxid-group__LoggingMacros_1ga7768b4f32a0d8d3cdaa9a1db5423199a>`(x)
	#define :ref:`LOG_ERROR_F<doxid-group__LoggingMacros_1ga4e4e99c3796c36ea1d541844fb955cd8>`
	#define :ref:`LOG_ERROR_S<doxid-group__LoggingMacros_1ga869e13d38a14388c77018138eb480ad5>`
	#define :ref:`LOG_INFO_D<doxid-group__LoggingMacros_1gab6b020a0e06bac23bb4d7a0cc02e5897>`(x)
	#define :ref:`LOG_INFO_F<doxid-group__LoggingMacros_1ga0d7123d7bfdf773f24ed68b102c7f7c7>`
	#define :ref:`LOG_INFO_S<doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>`
	#define :ref:`LOG_TRACE_F<doxid-group__LoggingMacros_1ga9909fc8f33cb3e2c2d88d50e3088a0c2>`
	#define :ref:`LOG_TRACE_S<doxid-group__LoggingMacros_1ga583ca116ff9e9817f4fde393c79d1b6a>`
	#define :ref:`LOG_WARN_D<doxid-group__LoggingMacros_1gadf8b29748ab1e51dc7944748077ec0f7>`(x)
	#define :ref:`LOG_WARN_F<doxid-group__LoggingMacros_1ga7ce06e64838465cc3c438e9d99f201b0>`
	#define :ref:`LOG_WARN_S<doxid-group__LoggingMacros_1gad9b694972a4957493602ba1e7d3c5925>`

.. _details-group__LoggingMacros:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~

Macros for convenient logging with different levels and formats.

Macros
------

.. index:: pair: define; LOG_CRITICAL_D
.. _doxid-group__LoggingMacros_1ga227b917e88f64d9f90c06c54e04c648e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_CRITICAL_D(x)

Macro for CRITICAL level logging with source location.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x

		- 
		  The message to log
		  
		  .. ref-code-block:: cpp
		  
		  	:ref:`LOG_CRITICAL_D <doxid-group__LoggingMacros_1ga227b917e88f64d9f90c06c54e04c648e>`("System failure detected");

.. index:: pair: define; LOG_CRITICAL_F
.. _doxid-group__LoggingMacros_1gaaa6f7679d883afb22171314099518ee1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_CRITICAL_F

Macro for CRITICAL level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_CRITICAL_F <doxid-group__LoggingMacros_1gaaa6f7679d883afb22171314099518ee1>`("System failure in module {}", module_name);

.. index:: pair: define; LOG_CRITICAL_S
.. _doxid-group__LoggingMacros_1ga00b710fb8e78bfaa73323b9deb1c43cc:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_CRITICAL_S

Macro for CRITICAL level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_CRITICAL_S <doxid-group__LoggingMacros_1ga00b710fb8e78bfaa73323b9deb1c43cc>` << "System shutdown due to critical error in " << component;

.. index:: pair: define; LOG_DEBUG_D
.. _doxid-group__LoggingMacros_1ga1ba761e7159ad23893e8bb9807d31ead:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_DEBUG_D(x)

Macro for DEBUG level logging with source location.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x

		- 
		  The message to log
		  
		  .. ref-code-block:: cpp
		  
		  	:ref:`LOG_DEBUG_D <doxid-group__LoggingMacros_1ga1ba761e7159ad23893e8bb9807d31ead>`("Unexpected value: " << value);

.. index:: pair: define; LOG_DEBUG_F
.. _doxid-group__LoggingMacros_1ga2eff2cba0a7856857be1df094bebf834:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_DEBUG_F

Macro for DEBUG level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_DEBUG_F <doxid-group__LoggingMacros_1ga2eff2cba0a7856857be1df094bebf834>`("Received {} bytes of data", data_size);

.. index:: pair: define; LOG_DEBUG_S
.. _doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_DEBUG_S

Macro for DEBUG level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_DEBUG_S <doxid-group__LoggingMacros_1ga1b58a6e7c97c1f4ca2fddf21b5e47251>` << "Processing " << items.size() << " items";

.. index:: pair: define; LOG_ERROR_D
.. _doxid-group__LoggingMacros_1ga7768b4f32a0d8d3cdaa9a1db5423199a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_ERROR_D(x)

Macro for ERROR level logging with source location.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x

		- 
		  The message to log
		  
		  .. ref-code-block:: cpp
		  
		  	:ref:`LOG_ERROR_D <doxid-group__LoggingMacros_1ga7768b4f32a0d8d3cdaa9a1db5423199a>`("Failed to connect to database");

.. index:: pair: define; LOG_ERROR_F
.. _doxid-group__LoggingMacros_1ga4e4e99c3796c36ea1d541844fb955cd8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_ERROR_F

Macro for ERROR level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_ERROR_F <doxid-group__LoggingMacros_1ga4e4e99c3796c36ea1d541844fb955cd8>`("Failed to process request: {}", error_message);

.. index:: pair: define; LOG_ERROR_S
.. _doxid-group__LoggingMacros_1ga869e13d38a14388c77018138eb480ad5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_ERROR_S

Macro for ERROR level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_ERROR_S <doxid-group__LoggingMacros_1ga869e13d38a14388c77018138eb480ad5>` << "Failed to open file: " << filename;

.. index:: pair: define; LOG_INFO_D
.. _doxid-group__LoggingMacros_1gab6b020a0e06bac23bb4d7a0cc02e5897:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_INFO_D(x)

Macro for INFO level logging with source location.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x

		- 
		  The message to log
		  
		  .. ref-code-block:: cpp
		  
		  	:ref:`LOG_INFO_D <doxid-group__LoggingMacros_1gab6b020a0e06bac23bb4d7a0cc02e5897>`("Initialization complete");

.. index:: pair: define; LOG_INFO_F
.. _doxid-group__LoggingMacros_1ga0d7123d7bfdf773f24ed68b102c7f7c7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_INFO_F

Macro for INFO level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_INFO_F <doxid-group__LoggingMacros_1ga0d7123d7bfdf773f24ed68b102c7f7c7>`("Application started successfully");

.. index:: pair: define; LOG_INFO_S
.. _doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_INFO_S

Macro for INFO level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_INFO_S <doxid-group__LoggingMacros_1ga28d4d51a64e29b87c59c10f86615f4bf>` << "Application version: " << version;

.. index:: pair: define; LOG_TRACE_F
.. _doxid-group__LoggingMacros_1ga9909fc8f33cb3e2c2d88d50e3088a0c2:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_TRACE_F

Macro for TRACE level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_TRACE_F <doxid-group__LoggingMacros_1ga9909fc8f33cb3e2c2d88d50e3088a0c2>`("Processing item {}", item_id);

.. index:: pair: define; LOG_TRACE_S
.. _doxid-group__LoggingMacros_1ga583ca116ff9e9817f4fde393c79d1b6a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_TRACE_S

Macro for TRACE level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_TRACE_S <doxid-group__LoggingMacros_1ga583ca116ff9e9817f4fde393c79d1b6a>` << "Entering function " << function_name;

.. index:: pair: define; LOG_WARN_D
.. _doxid-group__LoggingMacros_1gadf8b29748ab1e51dc7944748077ec0f7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_WARN_D(x)

Macro for WARN level logging with source location.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- x

		- 
		  The message to log
		  
		  .. ref-code-block:: cpp
		  
		  	:ref:`LOG_WARN_D <doxid-group__LoggingMacros_1gadf8b29748ab1e51dc7944748077ec0f7>`("Potential memory leak detected");

.. index:: pair: define; LOG_WARN_F
.. _doxid-group__LoggingMacros_1ga7ce06e64838465cc3c438e9d99f201b0:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_WARN_F

Macro for WARN level format logging.

.. ref-code-block:: cpp

	:ref:`LOG_WARN_F <doxid-group__LoggingMacros_1ga7ce06e64838465cc3c438e9d99f201b0>`("Disk space is low: {}MB remaining", free_space);

.. index:: pair: define; LOG_WARN_S
.. _doxid-group__LoggingMacros_1gad9b694972a4957493602ba1e7d3c5925:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define LOG_WARN_S

Macro for WARN level stream logging.

.. ref-code-block:: cpp

	:ref:`LOG_WARN_S <doxid-group__LoggingMacros_1gad9b694972a4957493602ba1e7d3c5925>` << "Low memory: " << free_memory << "MB available";


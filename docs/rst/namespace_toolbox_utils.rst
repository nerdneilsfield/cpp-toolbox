.. index:: pair: namespace; toolbox::utils
.. _doxid-namespacetoolbox_1_1utils:

namespace toolbox::utils
========================

.. toctree::
	:hidden:

	namespace_toolbox_utils_detail.rst
	namespace_toolbox_utils_impl.rst
	class_toolbox_utils_ClickException.rst
	class_toolbox_utils_CommandLineApp.rst
	class_toolbox_utils_ParameterError.rst
	class_toolbox_utils_UsageError.rst
	class_toolbox_utils_argument_t.rst
	class_toolbox_utils_command_t.rst
	class_toolbox_utils_flag_t.rst
	class_toolbox_utils_option_t.rst
	class_toolbox_utils_parameter_t.rst
	class_toolbox_utils_simple_timer_array_t.rst
	class_toolbox_utils_simple_timer_interface_t.rst
	class_toolbox_utils_stop_watch_timer_t.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace utils {

	// namespaces

	namespace :ref:`toolbox::utils::detail<doxid-namespacetoolbox_1_1utils_1_1detail>`;
		namespace :ref:`toolbox::utils::detail::has_istream_operator std<doxid-namespacetoolbox_1_1utils_1_1detail_1_1has__istream__operator_01std>`;
	namespace :ref:`toolbox::utils::impl<doxid-namespacetoolbox_1_1utils_1_1impl>`;

	// classes

	class :ref:`ClickException<doxid-classtoolbox_1_1utils_1_1ClickException>`;
	class :ref:`CommandLineApp<doxid-classtoolbox_1_1utils_1_1CommandLineApp>`;
	class :ref:`ParameterError<doxid-classtoolbox_1_1utils_1_1ParameterError>`;
	class :ref:`UsageError<doxid-classtoolbox_1_1utils_1_1UsageError>`;

	template <typename T>
	class :ref:`argument_t<doxid-classtoolbox_1_1utils_1_1argument__t>`;

	class :ref:`command_t<doxid-classtoolbox_1_1utils_1_1command__t>`;
	class :ref:`flag_t<doxid-classtoolbox_1_1utils_1_1flag__t>`;

	template <typename T>
	class :ref:`option_t<doxid-classtoolbox_1_1utils_1_1option__t>`;

	class :ref:`parameter_t<doxid-classtoolbox_1_1utils_1_1parameter__t>`;
	class :ref:`simple_timer_array_t<doxid-classtoolbox_1_1utils_1_1simple__timer__array__t>`;
	class :ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`;
	class :ref:`stop_watch_timer_t<doxid-classtoolbox_1_1utils_1_1stop__watch__timer__t>`;

	// global functions

	auto
	:ref:`create_timer<doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int size
	);

	auto
	:ref:`create_timer_with_names<doxid-namespacetoolbox_1_1utils_1a0c968dfd3bb680862601eb400f1c7048>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		const std::vector<std::string>& names
	);

	auto
	:ref:`delete_timer<doxid-namespacetoolbox_1_1utils_1aa8e612fa69792c6ed76c23854327b383>`(:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface);

	auto
	:ref:`start_timer<doxid-namespacetoolbox_1_1utils_1a9a3f35e20084e76ec978b7e4382f32da>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	auto
	:ref:`stop_timer<doxid-namespacetoolbox_1_1utils_1ad6cc9cbc03f538f2345b7a99d38a491f>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	auto
	:ref:`reset_timer<doxid-namespacetoolbox_1_1utils_1a9b2dac75b55638fac7dd7cc828857be1>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	auto
	:ref:`display_timer<doxid-namespacetoolbox_1_1utils_1a68025dd39e5b997d8f135daf943b63d5>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	auto
	:ref:`display_all_timer<doxid-namespacetoolbox_1_1utils_1a51ff38032ae9aa82ae0a1f010f02a658>`(:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface);

	auto
	:ref:`get_timer_elapsed<doxid-namespacetoolbox_1_1utils_1ac1758c4154d27ba9aa8db3eb75d7944d>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	auto
	:ref:`get_timer_elapsed_ms<doxid-namespacetoolbox_1_1utils_1a0535e571ea6c9132f893b2596af4d020>`(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	);

	} // namespace utils
.. _details-namespacetoolbox_1_1utils:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Functions
----------------

.. index:: pair: function; create_timer
.. _doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	create_timer(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int size
	)

Create a timer collection with specified size.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	create_timer(&timers, 3);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to store the created interface

	*
		- size

		- Number of timers to create



.. rubric:: Returns:

true if creation succeeded, false otherwise

.. index:: pair: function; create_timer_with_names
.. _doxid-namespacetoolbox_1_1utils_1a0c968dfd3bb680862601eb400f1c7048:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	create_timer_with_names(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		const std::vector<std::string>& names
	)

Create a timer collection with specified names.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	std::vector<std::string> names = {"timer1", "timer2", "timer3"};
	:ref:`create_timer_with_names <doxid-namespacetoolbox_1_1utils_1a0c968dfd3bb680862601eb400f1c7048>`(&timers, names);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to store the created interface

	*
		- names

		- Vector of names for each timer



.. rubric:: Returns:

true if creation succeeded, false otherwise

.. index:: pair: function; delete_timer
.. _doxid-namespacetoolbox_1_1utils_1aa8e612fa69792c6ed76c23854327b383:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	delete_timer(:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface)

Delete a timer collection.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	:ref:`create_timer <doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c>`(&timers, 3);
	// ... use timers ...
	:ref:`delete_timer <doxid-namespacetoolbox_1_1utils_1aa8e612fa69792c6ed76c23854327b383>`(&timers);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the interface to delete



.. rubric:: Returns:

true if deletion succeeded, false otherwise

.. index:: pair: function; start_timer
.. _doxid-namespacetoolbox_1_1utils_1a9a3f35e20084e76ec978b7e4382f32da:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	start_timer(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Start a specific timer.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	create_timer(&timers, 3);
	start_timer(&timers, 0);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to start



.. rubric:: Returns:

true if operation succeeded, false otherwise

.. index:: pair: function; stop_timer
.. _doxid-namespacetoolbox_1_1utils_1ad6cc9cbc03f538f2345b7a99d38a491f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	stop_timer(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Stop a specific timer.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	create_timer(&timers, 3);
	start_timer(&timers, 0);
	// ... perform operation ...
	stop_timer(&timers, 0);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to stop



.. rubric:: Returns:

true if operation succeeded, false otherwise

.. index:: pair: function; reset_timer
.. _doxid-namespacetoolbox_1_1utils_1a9b2dac75b55638fac7dd7cc828857be1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	reset_timer(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Reset a specific timer.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	create_timer(&timers, 3);
	start_timer(&timers, 0);
	// ... perform operation ...
	reset_timer(&timers, 0);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to reset



.. rubric:: Returns:

true if operation succeeded, false otherwise

.. index:: pair: function; display_timer
.. _doxid-namespacetoolbox_1_1utils_1a68025dd39e5b997d8f135daf943b63d5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	display_timer(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Display statistics for a specific timer.

.. ref-code-block:: cpp

	simple_timer_interface_t* timers;
	create_timer(&timers, 3);
	start_timer(&timers, 0);
	// ... perform operation ...
	display_timer(&timers, 0);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to display



.. rubric:: Returns:

true if operation succeeded, false otherwise

.. index:: pair: function; display_all_timer
.. _doxid-namespacetoolbox_1_1utils_1a51ff38032ae9aa82ae0a1f010f02a658:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	display_all_timer(:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface)

Display statistics for all timers.

.. ref-code-block:: cpp

	start_timer(&timers, 0);
	// ... perform operation ...
	display_all_timer(&timers);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface



.. rubric:: Returns:

true if operation succeeded, false otherwise

.. index:: pair: function; get_timer_elapsed
.. _doxid-namespacetoolbox_1_1utils_1ac1758c4154d27ba9aa8db3eb75d7944d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	get_timer_elapsed(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Get the elapsed time in seconds for a specific timer.

.. ref-code-block:: cpp

	// Basic usage
	simple_timer_interface_t* timers;
	:ref:`create_timer <doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c>`(&timers, 3);
	:ref:`start_timer <doxid-namespacetoolbox_1_1utils_1a9a3f35e20084e76ec978b7e4382f32da>`(&timers, 0);
	// ... perform operation ...
	double elapsed_seconds = :ref:`get_timer_elapsed <doxid-namespacetoolbox_1_1utils_1ac1758c4154d27ba9aa8db3eb75d7944d>`(&timers, 0);
	
	// Safe usage with null check
	simple_timer_interface_t* timers = nullptr;
	double elapsed = :ref:`get_timer_elapsed <doxid-namespacetoolbox_1_1utils_1ac1758c4154d27ba9aa8db3eb75d7944d>`(&timers, 0); // Returns 0.0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to query



.. rubric:: Returns:

Elapsed time in seconds. Returns 0.0 if timer_interface is null

.. index:: pair: function; get_timer_elapsed_ms
.. _doxid-namespacetoolbox_1_1utils_1a0535e571ea6c9132f893b2596af4d020:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	auto
	get_timer_elapsed_ms(
		:ref:`simple_timer_interface_t<doxid-classtoolbox_1_1utils_1_1simple__timer__interface__t>`** timer_interface,
		int id
	)

Get the elapsed time in milliseconds for a specific timer.

.. ref-code-block:: cpp

	// Basic usage
	simple_timer_interface_t* timers;
	:ref:`create_timer <doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c>`(&timers, 3);
	:ref:`start_timer <doxid-namespacetoolbox_1_1utils_1a9a3f35e20084e76ec978b7e4382f32da>`(&timers, 0);
	// ... perform operation ...
	double elapsed_ms = :ref:`get_timer_elapsed_ms <doxid-namespacetoolbox_1_1utils_1a0535e571ea6c9132f893b2596af4d020>`(&timers, 0);
	
	// Comparing seconds and milliseconds
	simple_timer_interface_t* timers;
	:ref:`create_timer <doxid-namespacetoolbox_1_1utils_1aabc758146e0a0ede00b635fec794f38c>`(&timers, 1);
	:ref:`start_timer <doxid-namespacetoolbox_1_1utils_1a9a3f35e20084e76ec978b7e4382f32da>`(&timers, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	:ref:`stop_timer <doxid-namespacetoolbox_1_1utils_1ad6cc9cbc03f538f2345b7a99d38a491f>`(&timers, 0);
	double seconds = :ref:`get_timer_elapsed <doxid-namespacetoolbox_1_1utils_1ac1758c4154d27ba9aa8db3eb75d7944d>`(&timers, 0);    // ~1.5 seconds
	double milliseconds = :ref:`get_timer_elapsed_ms <doxid-namespacetoolbox_1_1utils_1a0535e571ea6c9132f893b2596af4d020>`(&timers, 0); // ~1500.0 ms



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- timer_interface

		- Pointer to the timer interface

	*
		- id

		- Index of the timer to query



.. rubric:: Returns:

Elapsed time in milliseconds. Returns 0.0 if timer_interface is null


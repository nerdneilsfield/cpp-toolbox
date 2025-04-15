.. _global:
.. index:: pair: namespace; global

Global Namespace
================

.. toctree::
	:hidden:

	namespace_cpp_toolbox.rst
	namespace_toolbox.rst
	class_exported_class.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	// namespaces

	namespace :ref:`cpp_toolbox<doxid-namespacecpp__toolbox>`;
		namespace :ref:`cpp_toolbox::io<doxid-namespacecpp__toolbox_1_1io>`;
	namespace :ref:`toolbox<doxid-namespacetoolbox>`;
		namespace :ref:`toolbox::base<doxid-namespacetoolbox_1_1base>`;
			namespace :ref:`toolbox::base::detail<doxid-namespacetoolbox_1_1base_1_1detail>`;
		namespace :ref:`toolbox::concurrent<doxid-namespacetoolbox_1_1concurrent>`;
		namespace :ref:`toolbox::container<doxid-namespacetoolbox_1_1container>`;
			namespace :ref:`toolbox::container::detail<doxid-namespacetoolbox_1_1container_1_1detail>`;
			namespace :ref:`toolbox::container::string<doxid-namespacetoolbox_1_1container_1_1string>`;
		namespace :ref:`toolbox::file<doxid-namespacetoolbox_1_1file>`;
		namespace :ref:`toolbox::functional<doxid-namespacetoolbox_1_1functional>`;
			namespace :ref:`toolbox::functional::detail<doxid-namespacetoolbox_1_1functional_1_1detail>`;
			namespace :ref:`toolbox::functional::detail_impl<doxid-namespacetoolbox_1_1functional_1_1detail__impl>`;
		namespace :ref:`toolbox::io<doxid-namespacetoolbox_1_1io>`;
		namespace :ref:`toolbox::logger<doxid-namespacetoolbox_1_1logger>`;
		namespace :ref:`toolbox::traits<doxid-namespacetoolbox_1_1traits>`;
			namespace :ref:`toolbox::traits::detail<doxid-namespacetoolbox_1_1traits_1_1detail>`;
		namespace :ref:`toolbox::types<doxid-namespacetoolbox_1_1types>`;
			namespace :ref:`toolbox::types::detail<doxid-namespacetoolbox_1_1types_1_1detail>`;
		namespace :ref:`toolbox::utils<doxid-namespacetoolbox_1_1utils>`;
			namespace :ref:`toolbox::utils::detail<doxid-namespacetoolbox_1_1utils_1_1detail>`;
				namespace :ref:`toolbox::utils::detail::has_istream_operator std<doxid-namespacetoolbox_1_1utils_1_1detail_1_1has__istream__operator_01std>`;
			namespace :ref:`toolbox::utils::impl<doxid-namespacetoolbox_1_1utils_1_1impl>`;

	// typedefs

	typedef int8_t :ref:`i8<doxid-types_8hpp_1a196513547113c3f96a078ee3e658a6ef>`;
	typedef int16_t :ref:`i16<doxid-types_8hpp_1a977685259fe8059c48e92450967d46a8>`;
	typedef int32_t :ref:`i32<doxid-types_8hpp_1aec41e5f8e4b7aef2540e84268692fa84>`;
	typedef int64_t :ref:`i64<doxid-types_8hpp_1a86025ec3a0bc201ca893caac2cb2acbd>`;
	typedef uint8_t :ref:`u8<doxid-types_8hpp_1a987e4d6cf789cb59dbb390728271e4d6>`;
	typedef uint16_t :ref:`u16<doxid-types_8hpp_1a6dbf9516e516c901604625282c174aa4>`;
	typedef uint32_t :ref:`u32<doxid-types_8hpp_1a70c1fcc1a4e96af8f6580f708657cd41>`;
	typedef uint64_t :ref:`u64<doxid-types_8hpp_1a1af9f95651e921615e75917a6dd1b574>`;
	typedef float :ref:`f32<doxid-types_8hpp_1ad34d88453d37b65a09797bad37f2f527>`;
	typedef double :ref:`f64<doxid-types_8hpp_1abc2f107791bd7c1d49ecf0f168c085a3>`;
	typedef std::atomic<:ref:`i8<doxid-types_8hpp_1a196513547113c3f96a078ee3e658a6ef>`> :ref:`ai8<doxid-types_8hpp_1ab7a186619e8b1957c400972766741c7f>`;
	typedef std::atomic<:ref:`i16<doxid-types_8hpp_1a977685259fe8059c48e92450967d46a8>`> :ref:`ai16<doxid-types_8hpp_1ae7779e5100a922f202b2b4c42f711145>`;
	typedef std::atomic<:ref:`i32<doxid-types_8hpp_1aec41e5f8e4b7aef2540e84268692fa84>`> :ref:`ai32<doxid-types_8hpp_1a6af7812bce8fbc7becbab8eb449495a9>`;
	typedef std::atomic<:ref:`i64<doxid-types_8hpp_1a86025ec3a0bc201ca893caac2cb2acbd>`> :ref:`ai64<doxid-types_8hpp_1a8a0e830251e13b837d05c2f1c5d8fb39>`;
	typedef std::atomic<:ref:`f32<doxid-types_8hpp_1ad34d88453d37b65a09797bad37f2f527>`> :ref:`af32<doxid-types_8hpp_1a3999e3e60fa8b38b7a00e57826ae0516>`;
	typedef std::atomic<:ref:`f64<doxid-types_8hpp_1abc2f107791bd7c1d49ecf0f168c085a3>`> :ref:`af64<doxid-types_8hpp_1a1a7b2a9f8e0be7df7a27590be4d415fb>`;
	typedef std::atomic<:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`> :ref:`asize_t<doxid-types_8hpp_1a729c80646c837a2761bafc57c4825dba>`;
	typedef std::atomic<bool> :ref:`abool<doxid-types_8hpp_1aa4e1bfd63417b83934a83e047cac8ee8>`;
	typedef std::atomic<:ref:`u8<doxid-types_8hpp_1a987e4d6cf789cb59dbb390728271e4d6>`> :ref:`au8<doxid-types_8hpp_1a47f04e2bdee5caf57560bec698794a34>`;
	typedef std::atomic<:ref:`u16<doxid-types_8hpp_1a6dbf9516e516c901604625282c174aa4>`> :ref:`au16<doxid-types_8hpp_1a0d8b9aa12e17ab492e19748fcc2c2817>`;
	typedef std::atomic<:ref:`u32<doxid-types_8hpp_1a70c1fcc1a4e96af8f6580f708657cd41>`> :ref:`au32<doxid-types_8hpp_1ac3e47dd254dcab69c49f6aaa945b07d4>`;
	typedef std::atomic<:ref:`u64<doxid-types_8hpp_1a1af9f95651e921615e75917a6dd1b574>`> :ref:`au64<doxid-types_8hpp_1a63384b8b0116f867bac4f2d58b57daf1>`;
	typedef std::size_t :ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`;

	// classes

	class :ref:`exported_class<doxid-classexported__class>`;

	// global functions

	constexpr const char*
	:ref:`get_cpp_version<doxid-macro_8hpp_1ae993975970c1d826a15e5fca07370859>`();

	// macros

	#define :ref:`CPP_11<doxid-macro_8hpp_1aa66072fd34bc1b7e07b68c362c51ec77>`
	#define :target:`CPP_14<doxid-macro_8hpp_1a1bf1fb838aec0dacee3a689e55bc3ce7>`
	#define :target:`CPP_17<doxid-macro_8hpp_1a2b78a25a0c887ed4960c4e346f5cbb9e>`
	#define :target:`CPP_20<doxid-macro_8hpp_1a97bb0231779cfb2ea2cb3da0eaa83207>`
	#define :ref:`CPP_TOOLBOX_ALIGNAS<doxid-macro_8hpp_1ae8ab5f666fb0fcd7484e839c0a3913ba>`(Alignment)

	#define :ref:`CPP_TOOLBOX_ASSERT<doxid-macro_8hpp_1a07b73c50ffd2db423e667d4d9320f9a5>`( \
		Condition, \
		Message \
	)

	#define :target:`CPP_TOOLBOX_CONTAINER_STRING_HPP<doxid-string_8hpp_1ac5641c9a187e3701f2ceb6466362ba04>`
	#define :ref:`CPP_TOOLBOX_COUNT_ARGS<doxid-macro_8hpp_1aab3ac1f988e0c757f741e2ba3ce912f4>`(...)

	#define :target:`CPP_TOOLBOX_COUNT_ARGS_IMPL<doxid-macro_8hpp_1a28c982752d4e852cb067f8f0568c78c1>`( \
		_1, \
		_2, \
		_3, \
		_4, \
		_5, \
		_6, \
		_7, \
		_8, \
		_9, \
		_10, \
		Count, \
		... \
	)

	#define :ref:`CPP_TOOLBOX_CXX_LARGE_THAN_17<doxid-macro_8hpp_1a7bc7c68b2b225846c20f71930729fe30>`(Content)
	#define :ref:`CPP_TOOLBOX_CXX_VERSION<doxid-macro_8hpp_1a8ea85e1fa8e67520bf3aff86c29e747e>`
	#define :ref:`CPP_TOOLBOX_DEFAULT_CONSTRUCTOR<doxid-macro_8hpp_1a889194aa6d2dfc2637918a28833dd472>`(ClassType)

	#define :ref:`CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR<doxid-functional_8hpp_1a75326a1c2d103444262ab5d5adf40753>`( \
		FunctorName, \
		ReturnType, \
		Params, \
		Body \
	)

	#define :ref:`CPP_TOOLBOX_DELETE_CONSTRUCTOR<doxid-macro_8hpp_1ab9531237c7e39a8932c326b76e8fa836>`(ClassType)
	#define :ref:`CPP_TOOLBOX_DISABLE_COPY<doxid-macro_8hpp_1af1de779914dcded178fdd741367d0bcd>`(ClassType)
	#define :ref:`CPP_TOOLBOX_DISABLE_COPY_AND_MOVE<doxid-macro_8hpp_1a6497f847f85135e611cfce9aed2b051c>`(ClassType)
	#define :ref:`CPP_TOOLBOX_DISABLE_MOVE<doxid-macro_8hpp_1afdec75c7ea0e64620c5b13546ab3a0a2>`(ClassType)
	#define :ref:`CPP_TOOLBOX_FALLTHROUGH<doxid-macro_8hpp_1a3cb08ff0ec1356360c27432f9a2b2088>`
	#define :ref:`CPP_TOOLBOX_FORCE_INLINE<doxid-macro_8hpp_1a3ed967026f61635681019d68eaceb786>`
	#define :ref:`CPP_TOOLBOX_LIKELY<doxid-macro_8hpp_1a09effedcef20be6c3991ea6fb2f1bf39>`(Condition)

	#define :ref:`CPP_TOOLBOX_LOG_DEBUG<doxid-macro_8hpp_1a58988cbb122cf968db2fe5c7be7a929d>`( \
		fmt, \
		... \
	)

	#define :ref:`CPP_TOOLBOX_NODISCARD<doxid-macro_8hpp_1ab22050c00281c86d4ff32c97a8c3fa97>`

	#define :target:`CPP_TOOLBOX_REPEAT<doxid-macro_8hpp_1a8a46eee71dcd68738cf5987934e5b297>`( \
		n, \
		x \
	)

	#define :ref:`CPP_TOOLBOX_REPEAT_2<doxid-macro_8hpp_1a4538a153893ad1b9b6df324eab55b0ec>`(x)
	#define :target:`CPP_TOOLBOX_REPEAT_3<doxid-macro_8hpp_1a0079071b5ef96d007e696f14216b6b04>`(x)
	#define :target:`CPP_TOOLBOX_REPEAT_4<doxid-macro_8hpp_1acd42d6a2a5a616451e8572a5e11b7b0a>`(x)
	#define :target:`CPP_TOOLBOX_REPEAT_5<doxid-macro_8hpp_1a238a16e6d704923cfbb16a4f0a346a1c>`(x)
	#define :ref:`CPP_TOOLBOX_SAFE_CALL<doxid-macro_8hpp_1a2d4fba42ce9373d96d76575f9baad4be>`(Func)
	#define :ref:`CPP_TOOLBOX_SINGLETON<doxid-macro_8hpp_1ae96fe8cc69010f510fec50602c56b466>`(ClassType)

	#define :ref:`CPP_TOOLBOX_STATIC_ASSERT<doxid-macro_8hpp_1aeee8264ff582ee75d54d3ad07d099e62>`( \
		Condition, \
		Message \
	)

	#define :ref:`CPP_TOOLBOX_STRINGIZE<doxid-macro_8hpp_1aa0fd3bf42fc798fc571f2e6ce57f0b63>`(x)
	#define :target:`CPP_TOOLBOX_STRINGIZE_IMPL<doxid-macro_8hpp_1aecfa9d77154e10a63552197d8164459e>`(x)

	#define :ref:`CPP_TOOLBOX_STRING_CONCAT<doxid-macro_8hpp_1aa9292500bbfdba3794ba681d678afe77>`( \
		a, \
		b \
	)

	#define :target:`CPP_TOOLBOX_UNLIKELY<doxid-macro_8hpp_1aecb14a8e5e6ec5bb9525c65b1706eb5d>`(Condition)
	#define :ref:`CPP_TOOLBOX_UNREACHABLE<doxid-macro_8hpp_1a7c2a3ecd73816f08ca3c5026bbb3f2e7>`()
	#define :ref:`CPP_TOOLBOX_UNUSED<doxid-macro_8hpp_1af8c06a20283c12abe6586da7762a8ae5>`(Variable)

	#define :ref:`CPP_VERSION_STR<doxid-macro_8hpp_1a2337ee1912889babdb32b9f06bfe9753>`( \
		year, \
		month \
	)

	#define :ref:`CPP_VERSION_TO_INT<doxid-macro_8hpp_1ab6cc83217a072b02280fddea5cdeb6ec>`( \
		year, \
		month \
	)

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
	#define :ref:`__CURRENT_FUNCTION__<doxid-macro_8hpp_1a5436a8cb969eaf89c020539b27d28976>`

.. _details-global:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Typedefs
--------

.. index:: pair: typedef; i8
.. _doxid-types_8hpp_1a196513547113c3f96a078ee3e658a6ef:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef int8_t i8

8-bit signed integer

.. index:: pair: typedef; i16
.. _doxid-types_8hpp_1a977685259fe8059c48e92450967d46a8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef int16_t i16

16-bit signed integer

.. index:: pair: typedef; i32
.. _doxid-types_8hpp_1aec41e5f8e4b7aef2540e84268692fa84:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef int32_t i32

32-bit signed integer

.. index:: pair: typedef; i64
.. _doxid-types_8hpp_1a86025ec3a0bc201ca893caac2cb2acbd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef int64_t i64

64-bit signed integer

.. index:: pair: typedef; u8
.. _doxid-types_8hpp_1a987e4d6cf789cb59dbb390728271e4d6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef uint8_t u8

8-bit unsigned integer

.. index:: pair: typedef; u16
.. _doxid-types_8hpp_1a6dbf9516e516c901604625282c174aa4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef uint16_t u16

16-bit unsigned integer

.. index:: pair: typedef; u32
.. _doxid-types_8hpp_1a70c1fcc1a4e96af8f6580f708657cd41:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef uint32_t u32

32-bit unsigned integer

.. index:: pair: typedef; u64
.. _doxid-types_8hpp_1a1af9f95651e921615e75917a6dd1b574:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef uint64_t u64

64-bit unsigned integer

.. index:: pair: typedef; f32
.. _doxid-types_8hpp_1ad34d88453d37b65a09797bad37f2f527:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef float f32

32-bit floating point

.. index:: pair: typedef; f64
.. _doxid-types_8hpp_1abc2f107791bd7c1d49ecf0f168c085a3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef double f64

64-bit floating point

.. index:: pair: typedef; ai8
.. _doxid-types_8hpp_1ab7a186619e8b1957c400972766741c7f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`i8<doxid-types_8hpp_1a196513547113c3f96a078ee3e658a6ef>`> ai8

Atomic 8-bit signed integer.

.. index:: pair: typedef; ai16
.. _doxid-types_8hpp_1ae7779e5100a922f202b2b4c42f711145:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`i16<doxid-types_8hpp_1a977685259fe8059c48e92450967d46a8>`> ai16

Atomic 16-bit signed integer.

.. index:: pair: typedef; ai32
.. _doxid-types_8hpp_1a6af7812bce8fbc7becbab8eb449495a9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`i32<doxid-types_8hpp_1aec41e5f8e4b7aef2540e84268692fa84>`> ai32

Atomic 32-bit signed integer.

.. index:: pair: typedef; ai64
.. _doxid-types_8hpp_1a8a0e830251e13b837d05c2f1c5d8fb39:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`i64<doxid-types_8hpp_1a86025ec3a0bc201ca893caac2cb2acbd>`> ai64

Atomic 64-bit signed integer.

.. index:: pair: typedef; af32
.. _doxid-types_8hpp_1a3999e3e60fa8b38b7a00e57826ae0516:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`f32<doxid-types_8hpp_1ad34d88453d37b65a09797bad37f2f527>`> af32

Atomic 32-bit float.

.. index:: pair: typedef; af64
.. _doxid-types_8hpp_1a1a7b2a9f8e0be7df7a27590be4d415fb:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`f64<doxid-types_8hpp_1abc2f107791bd7c1d49ecf0f168c085a3>`> af64

Atomic 64-bit float.

.. index:: pair: typedef; asize_t
.. _doxid-types_8hpp_1a729c80646c837a2761bafc57c4825dba:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`size_t<doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638>`> asize_t

Atomic size_t.

.. index:: pair: typedef; abool
.. _doxid-types_8hpp_1aa4e1bfd63417b83934a83e047cac8ee8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<bool> abool

Atomic boolean.

.. index:: pair: typedef; au8
.. _doxid-types_8hpp_1a47f04e2bdee5caf57560bec698794a34:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`u8<doxid-types_8hpp_1a987e4d6cf789cb59dbb390728271e4d6>`> au8

Atomic 8-bit unsigned integer.

.. index:: pair: typedef; au16
.. _doxid-types_8hpp_1a0d8b9aa12e17ab492e19748fcc2c2817:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`u16<doxid-types_8hpp_1a6dbf9516e516c901604625282c174aa4>`> au16

Atomic 16-bit unsigned integer.

.. index:: pair: typedef; au32
.. _doxid-types_8hpp_1ac3e47dd254dcab69c49f6aaa945b07d4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`u32<doxid-types_8hpp_1a70c1fcc1a4e96af8f6580f708657cd41>`> au32

Atomic 32-bit unsigned integer.

.. index:: pair: typedef; au64
.. _doxid-types_8hpp_1a63384b8b0116f867bac4f2d58b57daf1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::atomic<:ref:`u64<doxid-types_8hpp_1a1af9f95651e921615e75917a6dd1b574>`> au64

Atomic 64-bit unsigned integer.

.. index:: pair: typedef; size_t
.. _doxid-types_8hpp_1a801d6a451a01953ef8cbae6feb6a3638:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	typedef std::size_t size_t

Size type alias.

Global Functions
----------------

.. index:: pair: function; get_cpp_version
.. _doxid-macro_8hpp_1ae993975970c1d826a15e5fca07370859:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	constexpr const char*
	get_cpp_version()

Get C++ version as string.

.. ref-code-block:: cpp

	const char* version = :ref:`get_cpp_version <doxid-macro_8hpp_1ae993975970c1d826a15e5fca07370859>`();
	std::cout << "Using " << version << std::endl;



.. rubric:: Returns:

String representation of C++ version

Macros
------

.. index:: pair: define; CPP_11
.. _doxid-macro_8hpp_1aa66072fd34bc1b7e07b68c362c51ec77:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_11

C++ standard detection.

C++ version macros

.. index:: pair: define; CPP_TOOLBOX_ALIGNAS
.. _doxid-macro_8hpp_1ae8ab5f666fb0fcd7484e839c0a3913ba:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_ALIGNAS(Alignment)

Function name macro.

Data alignment macro



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Alignment

		- Alignment value

.. index:: pair: define; CPP_TOOLBOX_ASSERT
.. _doxid-macro_8hpp_1a07b73c50ffd2db423e667d4d9320f9a5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_ASSERT( \
		Condition, \
		Message \
	)

Runtime assertion macro (debug only)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Condition

		- Assertion condition

	*
		- Message

		- Error message

.. index:: pair: define; CPP_TOOLBOX_COUNT_ARGS
.. _doxid-macro_8hpp_1aab3ac1f988e0c757f741e2ba3ce912f4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_COUNT_ARGS(...)

Count number of arguments.

.. index:: pair: define; CPP_TOOLBOX_CXX_LARGE_THAN_17
.. _doxid-macro_8hpp_1a7bc7c68b2b225846c20f71930729fe30:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_CXX_LARGE_THAN_17(Content)

Static assert for C++17 requirement.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Content

		- Assert message

.. index:: pair: define; CPP_TOOLBOX_CXX_VERSION
.. _doxid-macro_8hpp_1a8ea85e1fa8e67520bf3aff86c29e747e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_CXX_VERSION

Get C++ version string.

.. index:: pair: define; CPP_TOOLBOX_DEFAULT_CONSTRUCTOR
.. _doxid-macro_8hpp_1a889194aa6d2dfc2637918a28833dd472:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(ClassType)

Default constructor macro.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR
.. _doxid-functional_8hpp_1a75326a1c2d103444262ab5d5adf40753:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR( \
		FunctorName, \
		ReturnType, \
		Params, \
		Body \
	)

Macro to define a simple functor (function object) with a single operator()

.. ref-code-block:: cpp

	// Define a functor that adds two numbers
	:ref:`CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR <doxid-functional_8hpp_1a75326a1c2d103444262ab5d5adf40753>`(
	  Adder,           // FunctorName
	  int,             // ReturnType
	  (int a, int b),  // Params
	  return a + b;    // Body
	);
	
	// Use the functor
	Adder add;
	int sum = add(1, 2); // Returns 3
	
	// Define a functor that prints a message
	:ref:`CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR <doxid-functional_8hpp_1a75326a1c2d103444262ab5d5adf40753>`(
	  Printer,         // FunctorName
	  void,           // ReturnType
	  (const std::string& msg),  // Params
	  std::cout << msg << std::endl;  // Body
	);



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- FunctorName

		- Name of the functor struct to create

	*
		- ReturnType

		- Return type of operator()

	*
		- Params

		- Parameter list for operator() (must include parentheses)

	*
		- Body

		- Function body code (must include return statement if ReturnType is not void)

.. index:: pair: define; CPP_TOOLBOX_DELETE_CONSTRUCTOR
.. _doxid-macro_8hpp_1ab9531237c7e39a8932c326b76e8fa836:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DELETE_CONSTRUCTOR(ClassType)

Delete copy and move constructors.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_DISABLE_COPY
.. _doxid-macro_8hpp_1af1de779914dcded178fdd741367d0bcd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DISABLE_COPY(ClassType)

Disable copy operations for a class.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_DISABLE_COPY_AND_MOVE
.. _doxid-macro_8hpp_1a6497f847f85135e611cfce9aed2b051c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(ClassType)

Disable both copy and move operations.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_DISABLE_MOVE
.. _doxid-macro_8hpp_1afdec75c7ea0e64620c5b13546ab3a0a2:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_DISABLE_MOVE(ClassType)

Disable move operations for a class.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_FALLTHROUGH
.. _doxid-macro_8hpp_1a3cb08ff0ec1356360c27432f9a2b2088:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_FALLTHROUGH

Fallthrough attribute.

.. index:: pair: define; CPP_TOOLBOX_FORCE_INLINE
.. _doxid-macro_8hpp_1a3ed967026f61635681019d68eaceb786:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_FORCE_INLINE

Force inline macro.

.. index:: pair: define; CPP_TOOLBOX_LIKELY
.. _doxid-macro_8hpp_1a09effedcef20be6c3991ea6fb2f1bf39:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_LIKELY(Condition)

Branch prediction optimization macros.

.. index:: pair: define; CPP_TOOLBOX_LOG_DEBUG
.. _doxid-macro_8hpp_1a58988cbb122cf968db2fe5c7be7a929d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_LOG_DEBUG( \
		fmt, \
		... \
	)

Debug logging macro.

.. index:: pair: define; CPP_TOOLBOX_NODISCARD
.. _doxid-macro_8hpp_1ab22050c00281c86d4ff32c97a8c3fa97:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_NODISCARD

No discard attribute.

.. index:: pair: define; CPP_TOOLBOX_REPEAT_2
.. _doxid-macro_8hpp_1a4538a153893ad1b9b6df324eab55b0ec:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_REPEAT_2(x)

Repeat macros.

.. index:: pair: define; CPP_TOOLBOX_SAFE_CALL
.. _doxid-macro_8hpp_1a2d4fba42ce9373d96d76575f9baad4be:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_SAFE_CALL(Func)

Safe function call with exception handling.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Func

		- Function to call

.. index:: pair: define; CPP_TOOLBOX_SINGLETON
.. _doxid-macro_8hpp_1ae96fe8cc69010f510fec50602c56b466:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_SINGLETON(ClassType)

Singleton pattern macro.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ClassType

		- Class name

.. index:: pair: define; CPP_TOOLBOX_STATIC_ASSERT
.. _doxid-macro_8hpp_1aeee8264ff582ee75d54d3ad07d099e62:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_STATIC_ASSERT( \
		Condition, \
		Message \
	)

Static assertion macro.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Condition

		- Assertion condition

	*
		- Message

		- Error message

.. index:: pair: define; CPP_TOOLBOX_STRINGIZE
.. _doxid-macro_8hpp_1aa0fd3bf42fc798fc571f2e6ce57f0b63:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_STRINGIZE(x)

Stringize macro.

.. index:: pair: define; CPP_TOOLBOX_STRING_CONCAT
.. _doxid-macro_8hpp_1aa9292500bbfdba3794ba681d678afe77:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_STRING_CONCAT( \
		a, \
		b \
	)

String concatenation.

.. index:: pair: define; CPP_TOOLBOX_UNREACHABLE
.. _doxid-macro_8hpp_1a7c2a3ecd73816f08ca3c5026bbb3f2e7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_UNREACHABLE()

Unreachable code marker.

.. index:: pair: define; CPP_TOOLBOX_UNUSED
.. _doxid-macro_8hpp_1af8c06a20283c12abe6586da7762a8ae5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_TOOLBOX_UNUSED(Variable)

Mark variable as unused.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Variable

		- Variable name

.. index:: pair: define; CPP_VERSION_STR
.. _doxid-macro_8hpp_1a2337ee1912889babdb32b9f06bfe9753:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_VERSION_STR( \
		year, \
		month \
	)

Compiler detection macros.

Platform detection macros

Architecture detection macros

Convert C++ version to string



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- year

		- Year part of version number

	*
		- month

		- Month part of version number



.. rubric:: Returns:

String representation of version

.. index:: pair: define; CPP_VERSION_TO_INT
.. _doxid-macro_8hpp_1ab6cc83217a072b02280fddea5cdeb6ec:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define CPP_VERSION_TO_INT( \
		year, \
		month \
	)

Convert C++ version to integer.



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- year

		- Year part of version number

	*
		- month

		- Month part of version number



.. rubric:: Returns:

Integer representation of version

.. index:: pair: define; __CURRENT_FUNCTION__
.. _doxid-macro_8hpp_1a5436a8cb969eaf89c020539b27d28976:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	#define __CURRENT_FUNCTION__

Current function name macro.


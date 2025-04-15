.. index:: pair: namespace; toolbox::functional
.. _doxid-namespacetoolbox_1_1functional:

namespace toolbox::functional
=============================

.. toctree::
	:hidden:

	namespace_toolbox_functional_detail.rst
	namespace_toolbox_functional_detail_impl.rst
	class_toolbox_functional_MemoizedFunction.rst

Overview
~~~~~~~~




.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	
	namespace functional {

	// namespaces

	namespace :ref:`toolbox::functional::detail<doxid-namespacetoolbox_1_1functional_1_1detail>`;
	namespace :ref:`toolbox::functional::detail_impl<doxid-namespacetoolbox_1_1functional_1_1detail__impl>`;

	// classes

	template <
		typename R,
		typename... Args
	>
	class :ref:`MemoizedFunction<R(Args...)><doxid-classtoolbox_1_1functional_1_1MemoizedFunction_3_01R_07Args_8_8_8_08_4>`;

	// global variables

	class CPP_TOOLBOX_EXPORT :ref:`MemoizedFunction<doxid-namespacetoolbox_1_1functional_1ae72a328600ef1342629b7fe3bab4d88b>`;

	// global functions

	template <
		typename G,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`compose<doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`(
		G&& g,
		F&& f
	);

	template <
		typename F1,
		typename... FRest
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`compose<doxid-namespacetoolbox_1_1functional_1ad505ee192c5c9ebd8d6d8d8628d888d5>`(
		F1&& f1,
		FRest&&... rest
	);

	CPP_TOOLBOX_EXPORT auto
	:ref:`compose<doxid-namespacetoolbox_1_1functional_1a8aa30db9ce055ace2af1719e8c76a9b0>`();

	template <
		typename F,
		typename Arg1
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`bind_first<doxid-namespacetoolbox_1_1functional_1aecc9e3b337e5a7e2d3d9eaac80939002>`(
		F&& f,
		Arg1&& arg1
	);

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(
		const std::optional<T>& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1ad15ccddfc681b69126102b97ee79e3ac>`(
		std::optional<T>&& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`flatMap<doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd>`(
		const std::optional<T>& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`flatMap<doxid-namespacetoolbox_1_1functional_1a8866f4d6dc615d3c2b237adea3cf0810>`(
		std::optional<T>&& opt,
		F&& f
	);

	template <
		typename T,
		typename U
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`orElse<doxid-namespacetoolbox_1_1functional_1a50e389bf7d4e1f5788548cd2784de28f>`(
		const std::optional<T>& opt,
		U&& default_value
	);

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`orElseGet<doxid-namespacetoolbox_1_1functional_1a4a7a4acdd8ba7485698f9ea3e64920e9>`(
		const std::optional<T>& opt,
		F&& default_func
	);

	template <
		typename T,
		typename P
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(
		const std::optional<T>& opt,
		P&& p
	);

	template <
		typename T,
		typename P
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1a887c36248685aa88197aa703787c30a8>`(
		std::optional<T>&& opt,
		P&& p
	);

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(
		const std::variant<Ts...>& var,
		Fs&&... visitors
	);

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1a1893e027e95dd95cee03d2c216de4205>`(
		std::variant<Ts...>& var,
		Fs&&... visitors
	);

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1a09ce89a295a09678c2d0aa2dd9903ceb>`(
		std::variant<Ts...>&& var,
		Fs&&... visitors
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a470b06977505a96a3ee4ca05e6ca9907>`(
		const std::variant<Ts...>& var,
		F&& f
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a7e06cdbbc5ffdb751d6ec6b7b42dc2f3>`(
		std::variant<Ts...>& var,
		F&& f
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1ad0df97865b75c205b0ae131de8b81688>`(
		std::variant<Ts...>&& var,
		F&& f
	);

	template <
		typename Container,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1aee9e6afdbc445efd38008f88863d64f1>`(
		const Container& input,
		Func&& f
	);

	template <
		typename Container,
		typename Predicate
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1a04bfaacdb77bebe2eecb48d3ae6167da>`(
		const Container& input,
		Predicate&& p
	);

	template <
		typename Container,
		typename T,
		typename BinaryOp
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`reduce<doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(
		const Container& input,
		T identity,
		BinaryOp&& op
	);

	template <
		typename Container,
		typename BinaryOp
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`reduce<doxid-namespacetoolbox_1_1functional_1a066d6a37ea371a922c192f5502bb1a26>`(
		const Container& input,
		BinaryOp&& op
	);

	template <typename... Containers>
	CPP_TOOLBOX_EXPORT auto
	:ref:`zip<doxid-namespacetoolbox_1_1functional_1a14a774e92ad2ff34a0d6fb26e1929527>`(const Containers&... containers);

	template <
		typename ContainerKeys,
		typename ContainerValues,
		typename Key = typename std::decay_t<ContainerKeys>::value_type,
		typename Value = typename std::decay_t<ContainerValues>::value_type,
		typename Hash = std::hash<Key>,
		typename KeyEqual = std::equal_to<Key>,
		typename Alloc = std::allocator<std::pair<const Key, Value>>
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`zip_to_unordered_map<doxid-namespacetoolbox_1_1functional_1a4fb6acc3f4d90809e63e519440e55b7c>`(
		const ContainerKeys& keys,
		const ContainerValues& values
	);

	template <
		typename Signature,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`memoize<doxid-namespacetoolbox_1_1functional_1a8c3f8127f603901d466318d6aaff381f>`(Func&& f);

	template <
		typename R,
		typename... Args,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	:ref:`memoize_explicit<doxid-namespacetoolbox_1_1functional_1ae451cc75b6a39fd16d3c1c680988706e>`(Func&& f);

	template <
		typename G,
		typename F
	>
	auto
	:ref:`compose<doxid-namespacetoolbox_1_1functional_1a73614e7a6d879b7e2ff3be342d01f832>`(
		G&& g,
		F&& f
	);

	template <
		typename F1,
		typename... FRest
	>
	auto
	:ref:`compose<doxid-namespacetoolbox_1_1functional_1a62235f5b9cd7bdf5f6754cb243e703a4>`(
		F1&& f1,
		FRest&&... rest
	);

	template <
		typename F,
		typename Arg1
	>
	auto
	:ref:`bind_first<doxid-namespacetoolbox_1_1functional_1a1e0ae6022a29dcda8ebca974573bf70a>`(
		F&& f,
		Arg1&& arg1
	);

	template <
		typename T,
		typename F
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a4ab57c3803f4ac4bb491400a3b6a4233>`(
		const std::optional<T>& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a86d8ff17712cc38ece6f1a84548dda26>`(
		std::optional<T>&& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	auto
	:ref:`flatMap<doxid-namespacetoolbox_1_1functional_1a424450bf0ce67fb53903b20dee8b1765>`(
		const std::optional<T>& opt,
		F&& f
	);

	template <
		typename T,
		typename F
	>
	auto
	:ref:`flatMap<doxid-namespacetoolbox_1_1functional_1a20f50536887abe0448cb0484c4ec3abb>`(
		std::optional<T>&& opt,
		F&& f
	);

	template <
		typename T,
		typename U
	>
	auto
	:ref:`orElse<doxid-namespacetoolbox_1_1functional_1ad0b4c848e551d917eebb0bebfa50fc66>`(
		const std::optional<T>& opt,
		U&& default_value
	);

	template <
		typename T,
		typename F
	>
	auto
	:ref:`orElseGet<doxid-namespacetoolbox_1_1functional_1ae1baad6982df3cde2d03724aa870e274>`(
		const std::optional<T>& opt,
		F&& default_func
	);

	template <
		typename T,
		typename P
	>
	auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1a68ecec782b192847390f333b4af52dc3>`(
		const std::optional<T>& opt,
		P&& p
	);

	template <
		typename T,
		typename P
	>
	auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1adb40cb4be7d2e760e5cf545c558aa87f>`(
		std::optional<T>&& opt,
		P&& p
	);

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1adfc20d2de6408ff5e8bda0b7174c2bf9>`(
		const std::variant<Ts...>& var,
		Fs&&... visitors
	);

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1a31150fb88996d76590ccc35593c7b70d>`(
		std::variant<Ts...>& var,
		Fs&&... visitors
	);

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	:ref:`match<doxid-namespacetoolbox_1_1functional_1a48db12c30c97eddfdbe5bb80179b94f4>`(
		std::variant<Ts...>&& var,
		Fs&&... visitors
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a65a3a49ee3d8b5063e93e806d350db95>`(
		const std::variant<Ts...>& var,
		F&& f
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1abe0946f6ce3008057d1e5a6c5ee9b120>`(
		std::variant<Ts...>& var,
		F&& f
	);

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1a6ec605443bd2b2ee732a30baf2427805>`(
		std::variant<Ts...>&& var,
		F&& f
	);

	template <
		typename Container,
		typename Func
	>
	auto
	:ref:`map<doxid-namespacetoolbox_1_1functional_1aba27921b0ebcf50a284153a64c156a57>`(
		const Container& input,
		Func&& f
	);

	template <
		typename Container,
		typename Predicate
	>
	auto
	:ref:`filter<doxid-namespacetoolbox_1_1functional_1aaaf8da56d5d7e9e09986db066318b5cf>`(
		const Container& input,
		Predicate&& p
	);

	template <
		typename Container,
		typename T,
		typename BinaryOp
	>
	auto
	:ref:`reduce<doxid-namespacetoolbox_1_1functional_1aea3f2ee7ec3d87022b31c2f07743c4e6>`(
		const Container& input,
		T identity,
		BinaryOp&& op
	);

	template <
		typename Container,
		typename BinaryOp
	>
	auto
	:ref:`reduce<doxid-namespacetoolbox_1_1functional_1a10b525e7afccde1000246e289b1469af>`(
		const Container& input,
		BinaryOp&& op
	);

	template <typename... Containers>
	auto
	:ref:`zip<doxid-namespacetoolbox_1_1functional_1a5b667889be637834a733f0dbf6a145e7>`(const Containers&... containers);

	template <
		typename ContainerKeys,
		typename ContainerValues,
		typename Key,
		typename Value,
		typename Hash,
		typename KeyEqual,
		typename Alloc
	>
	auto
	:ref:`zip_to_unordered_map<doxid-namespacetoolbox_1_1functional_1a04b877b2a05cd06d327adb9f9bd273ea>`(
		const ContainerKeys& keys,
		const ContainerValues& values
	);

	template <
		typename Signature,
		typename Func
	>
	auto
	:ref:`memoize<doxid-namespacetoolbox_1_1functional_1a67ae7d48b6cf42eb8a27dbf3a970167b>`(Func&& f);

	template <
		typename R,
		typename... Args,
		typename Func
	>
	auto
	:ref:`memoize_explicit<doxid-namespacetoolbox_1_1functional_1af4ca279b16162dbbf7042fc2681bdba7>`(Func&& f);

	} // namespace functional
.. _details-namespacetoolbox_1_1functional:

Detailed Documentation
~~~~~~~~~~~~~~~~~~~~~~



Global Variables
----------------

.. index:: pair: variable; MemoizedFunction
.. _doxid-namespacetoolbox_1_1functional_1ae72a328600ef1342629b7fe3bab4d88b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	class CPP_TOOLBOX_EXPORT MemoizedFunction

Memoized function class that caches function results.

.. ref-code-block:: cpp

	// Example usage of MemoizedFunction class
	// Create memoized factorial function
	:ref:`MemoizedFunction <doxid-namespacetoolbox_1_1functional_1ae72a328600ef1342629b7fe3bab4d88b>`<int(int)> fact([](int n) {
	  if (n <= 1) return 1;
	  return n * fact(n-1);
	});
	
	int result1 = fact(5); // Computes and caches
	int result2 = fact(5); // Returns cached result

.. note::

	Thread-safe implementation



.. note::

	Function arguments must be copyable and form valid map key



.. note::

	Return type must be copyable



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Signature

		- Function signature type (e.g., int(int, std::string))

Global Functions
----------------

.. index:: pair: function; compose
.. _doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename G,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	compose(
		G&& g,
		F&& f
	)

Composes two functions into a single function.

.. ref-code-block:: cpp

	// Example of function composition
	auto add_one = [](int x) { return x + 1; };
	auto multiply_two = [](int x) { return x * 2; };
	auto composed = :ref:`compose <doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`(multiply_two, add_one);
	int result = composed(5); // Returns 12: ((5 + 1) * 2)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- G

		- Type of outer function

	*
		- F

		- Type of inner function

	*
		- g

		- Outer function

	*
		- f

		- Inner function



.. rubric:: Returns:

A function that applies f then g

.. index:: pair: function; compose
.. _doxid-namespacetoolbox_1_1functional_1ad505ee192c5c9ebd8d6d8d8628d888d5:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename F1,
		typename... FRest
	>
	CPP_TOOLBOX_EXPORT auto
	compose(
		F1&& f1,
		FRest&&... rest
	)

Composes multiple functions into a single function.

.. ref-code-block:: cpp

	// Example of multiple function composition
	auto add_one = [](int x) { return x + 1; };
	auto multiply_two = [](int x) { return x * 2; };
	auto square = [](int x) { return x * x; };
	auto composed = :ref:`compose <doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`(square, multiply_two, add_one);
	int result = composed(5); // Returns 144: ((5 + 1) * 2)^2



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F1

		- Type of first (outermost) function

	*
		- FRest

		- Types of remaining functions

	*
		- f1

		- First function

	*
		- rest

		- Remaining functions



.. rubric:: Returns:

A function that applies functions from right to left

.. index:: pair: function; compose
.. _doxid-namespacetoolbox_1_1functional_1a8aa30db9ce055ace2af1719e8c76a9b0:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	CPP_TOOLBOX_EXPORT auto
	compose()

Empty compose function that throws an error.

.. ref-code-block:: cpp

	try {
	  auto empty = :ref:`compose <doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`();
	} catch(const std::logic_error& e) {
	  // Exception caught
	}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- std::logic_error

		- when called

.. index:: pair: function; bind_first
.. _doxid-namespacetoolbox_1_1functional_1aecc9e3b337e5a7e2d3d9eaac80939002:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename F,
		typename Arg1
	>
	CPP_TOOLBOX_EXPORT auto
	bind_first(
		F&& f,
		Arg1&& arg1
	)

Binds the first argument of a function.

.. ref-code-block:: cpp

	// Example of binding first argument
	auto divide = [](int x, int y) { return x / y; };
	auto divide_10_by = :ref:`bind_first <doxid-namespacetoolbox_1_1functional_1aecc9e3b337e5a7e2d3d9eaac80939002>`(divide, 10);
	int result = divide_10_by(2); // Returns 5



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F

		- Function type

	*
		- Arg1

		- Type of first argument

	*
		- f

		- Function to bind

	*
		- arg1

		- Value to bind as first argument



.. rubric:: Returns:

Function with first argument bound

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		const std::optional<T>& opt,
		F&& f
	)

Maps a function over an optional value.

.. ref-code-block:: cpp

	// Example of mapping over optional
	std::optional<int> opt(5);
	auto times_two = [](int x) { return x * 2; };
	auto result = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(opt, times_two); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type

	*
		- opt

		- Optional to map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Optional containing result of f if opt has value, empty optional otherwise

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1ad15ccddfc681b69126102b97ee79e3ac:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		std::optional<T>&& opt,
		F&& f
	)

Maps a function over an optional rvalue.

.. ref-code-block:: cpp

	// Example of mapping over optional rvalue
	auto result = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(std::optional<int>(5), [](int x) { return x * 2; }); //
	Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type

	*
		- opt

		- Optional rvalue to map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Optional containing result of f if opt has value, empty optional otherwise

.. index:: pair: function; flatMap
.. _doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	flatMap(
		const std::optional<T>& opt,
		F&& f
	)

Flat maps a function over an optional value.

.. ref-code-block:: cpp

	// Example of flat mapping over optional
	std::optional<int> opt(5);
	auto maybe_double = [](int x) -> std::optional<int> {
	  return x < 10 ? std::optional(x * 2) : std::nullopt;
	};
	auto result = :ref:`flatMap <doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd>`(opt, maybe_double); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning optional

	*
		- opt

		- Optional to flat map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Result of f if opt has value, empty optional otherwise

.. index:: pair: function; flatMap
.. _doxid-namespacetoolbox_1_1functional_1a8866f4d6dc615d3c2b237adea3cf0810:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	flatMap(
		std::optional<T>&& opt,
		F&& f
	)

Flat maps a function over an optional rvalue.

.. ref-code-block:: cpp

	// Example of flat mapping over optional rvalue
	auto result = :ref:`flatMap <doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd>`(std::optional<int>(5),
	                     [](int x) -> std::optional<int> {
	                       return x < 10 ? std::optional(x * 2) : std::nullopt;
	                     }); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning optional

	*
		- opt

		- Optional rvalue to flat map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Result of f if opt has value, empty optional otherwise

.. index:: pair: function; orElse
.. _doxid-namespacetoolbox_1_1functional_1a50e389bf7d4e1f5788548cd2784de28f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename U
	>
	CPP_TOOLBOX_EXPORT auto
	orElse(
		const std::optional<T>& opt,
		U&& default_value
	)

Returns the contained value or a default.

.. ref-code-block:: cpp

	// Example of providing default value
	std::optional<int> opt;
	int result = :ref:`orElse <doxid-namespacetoolbox_1_1functional_1a50e389bf7d4e1f5788548cd2784de28f>`(opt, 42); // Returns 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- U

		- Type of default value

	*
		- opt

		- Optional to get value from

	*
		- default_value

		- Value to return if optional is empty



.. rubric:: Returns:

Contained value or default

.. index:: pair: function; orElseGet
.. _doxid-namespacetoolbox_1_1functional_1a4a7a4acdd8ba7485698f9ea3e64920e9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	orElseGet(
		const std::optional<T>& opt,
		F&& default_func
	)

Returns the contained value or calls function for default.

.. ref-code-block:: cpp

	// Example of providing default via function
	std::optional<int> opt;
	auto get_default = []() { return 42; };
	int result = :ref:`orElseGet <doxid-namespacetoolbox_1_1functional_1a4a7a4acdd8ba7485698f9ea3e64920e9>`(opt, get_default); // Returns 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning default value

	*
		- opt

		- Optional to get value from

	*
		- default_func

		- Function to call for default value



.. rubric:: Returns:

Contained value or result of default_func

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename P
	>
	CPP_TOOLBOX_EXPORT auto
	filter(
		const std::optional<T>& opt,
		P&& p
	)

Filters an optional based on a predicate.

.. ref-code-block:: cpp

	// Example of filtering optional
	std::optional<int> opt(42);
	auto is_even = [](int x) { return x % 2 == 0; };
	auto result = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(opt, is_even); // Contains 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- P

		- Predicate type

	*
		- opt

		- Optional to filter

	*
		- p

		- Predicate to apply



.. rubric:: Returns:

Optional if predicate returns true, empty optional otherwise

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1a887c36248685aa88197aa703787c30a8:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename P
	>
	CPP_TOOLBOX_EXPORT auto
	filter(
		std::optional<T>&& opt,
		P&& p
	)

Filters an optional rvalue based on a predicate.

.. ref-code-block:: cpp

	// Example of filtering optional rvalue
	auto result = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(std::optional<int>(42), [](int x) { return x % 2 == 0;
	}); // Contains 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- P

		- Predicate type

	*
		- opt

		- Optional rvalue to filter

	*
		- p

		- Predicate to apply



.. rubric:: Returns:

Optional if predicate returns true, empty optional otherwise

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	match(
		const std::variant<Ts...>& var,
		Fs&&... visitors
	)

Pattern matches on a variant using visitor functions.

.. ref-code-block:: cpp

	// Example of pattern matching on variant
	std::variant<int, std::string> v = 42;
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(v,
	  [](int i) { return i * 2; },
	  [](const std::string& s) { return s.length(); }
	); // Returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1a1893e027e95dd95cee03d2c216de4205:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	match(
		std::variant<Ts...>& var,
		Fs&&... visitors
	)

Match variant with visitor functions for non-const lvalue variant.

.. ref-code-block:: cpp

	// Example of pattern matching on lvalue variant
	std::variant<int, std::string> v = 42;
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(v,
	  [](int& i) { i *= 2; return i; },
	  [](std::string& s) { s += "!"; return s.length(); }
	); // Modifies v and returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1a09ce89a295a09678c2d0aa2dd9903ceb:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	CPP_TOOLBOX_EXPORT auto
	match(
		std::variant<Ts...>&& var,
		Fs&&... visitors
	)

Match variant with visitor functions for rvalue variant.

.. ref-code-block:: cpp

	// Example of pattern matching on rvalue variant
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(std::variant<int, std::string>(42),
	  [](int&& i) { return i * 2; },
	  [](std::string&& s) { return s + "!"; }
	); // Returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a470b06977505a96a3ee4ca05e6ca9907:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		const std::variant<Ts...>& var,
		F&& f
	)

Apply a function to the value held by a variant and return result in a new variant.

.. ref-code-block:: cpp

	// Example of mapping over variant
	std::variant<int, std::string> v = 42;
	auto result = map<std::variant<double, size_t>>(v,
	  [](const auto& x) -> std::variant<double, size_t> {
	    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
	      return x * 1.5; // int -> double
	    } else {
	      return x.length(); // string -> size_t
	    }
	  }
	); // Returns variant containing 63.0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value

	*
		- std::bad_variant_access

		- if var is valueless_by_exception

	*
		- Any

		- exception thrown by f

	*
		- std::bad_variant_access

		- if f's result cannot construct any ResultVariant type



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a7e06cdbbc5ffdb751d6ec6b7b42dc2f3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		std::variant<Ts...>& var,
		F&& f
	)

Apply a function to the value held by a non-const lvalue variant.

.. ref-code-block:: cpp

	// Example of mapping over lvalue variant
	std::variant<int, std::string> v = 42;
	auto result = map<std::variant<double>>(v,
	  [](int& x) { x *= 2; return x * 1.5; }
	); // Modifies v and returns variant containing 126.0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1ad0df97865b75c205b0ae131de8b81688:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		std::variant<Ts...>&& var,
		F&& f
	)

Apply a function to the value held by an rvalue variant.

.. ref-code-block:: cpp

	// Example of mapping over rvalue variant
	auto result = map<std::variant<std::string>>(
	  std::variant<int, std::string>(42),
	  [](auto&& x) { return :ref:`std::to_string <doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60>`(x); }
	); // Returns variant containing "42"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1aee9e6afdbc445efd38008f88863d64f1:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	map(
		const Container& input,
		Func&& f
	)

Apply a function to each element in a container and return results in a new vector.

.. ref-code-block:: cpp

	// Example usage of map on containers
	std::vector<int> nums = {1, 2, 3};
	auto squares = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(nums, [](int x) { return x * x; }); // Returns {1, 4, 9}
	
	std::list<std::string> strs = {"a", "bb", "ccc"};
	auto lengths = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(strs, [](const auto& s) { return s.length(); }); //
	Returns {1, 2, 3}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- Func

		- Function type to apply to elements

	*
		- input

		- Input container

	*
		- f

		- Function to apply to each element



.. rubric:: Returns:

Vector containing results of applying f to each element

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1a04bfaacdb77bebe2eecb48d3ae6167da:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename Predicate
	>
	CPP_TOOLBOX_EXPORT auto
	filter(
		const Container& input,
		Predicate&& p
	)

Filter elements from a container that satisfy a predicate.

.. ref-code-block:: cpp

	// Example usage of filter
	std::vector<int> nums = {1, 2, 3, 4, 5};
	auto evens = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(nums, [](int x) { return x % 2 == 0; }); // Returns {2,
	4}
	
	std::list<std::string> strs = {"a", "bb", "ccc"};
	auto long_strs = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(strs, [](const auto& s) { return s.length() > 1; });
	// Returns {"bb", "ccc"}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- Predicate

		- Predicate function type

	*
		- input

		- Input container

	*
		- p

		- Predicate function returning bool



.. rubric:: Returns:

Vector containing elements that satisfy predicate p

.. index:: pair: function; reduce
.. _doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename T,
		typename BinaryOp
	>
	CPP_TOOLBOX_EXPORT auto
	reduce(
		const Container& input,
		T identity,
		BinaryOp&& op
	)

Reduce container elements using a binary operation with initial value.

.. ref-code-block:: cpp

	// Example usage of reduce with identity value
	std::vector<int> nums = {1, 2, 3, 4};
	auto sum = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, 0, std::plus<int>()); // Returns 10
	auto product = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, 1, std::multiplies<int>()); // Returns 24
	
	std::list<std::string> strs = {"a", "b", "c"};
	auto concat = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(strs, std::string(), std::plus<std::string>()); //
	Returns "abc"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- T

		- Initial value and result type

	*
		- BinaryOp

		- Binary operation function type

	*
		- input

		- Input container

	*
		- identity

		- Initial value (identity element)

	*
		- op

		- Binary operation to combine elements



.. rubric:: Returns:

Result of reduction

.. index:: pair: function; reduce
.. _doxid-namespacetoolbox_1_1functional_1a066d6a37ea371a922c192f5502bb1a26:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename BinaryOp
	>
	CPP_TOOLBOX_EXPORT auto
	reduce(
		const Container& input,
		BinaryOp&& op
	)

Reduce non-empty container elements using a binary operation.

.. ref-code-block:: cpp

	// Example usage of reduce without identity value
	std::vector<int> nums = {1, 2, 3, 4};
	auto sum = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, std::plus<int>()); // Returns 10
	
	std::list<std::string> strs = {"a", "b", "c"};
	auto concat = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(strs, std::plus<std::string>()); // Returns "abc"
	
	// This will throw:
	std::vector<int> empty;
	auto result = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(empty, std::plus<int>()); // throws
	std::invalid_argument



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- BinaryOp

		- Binary operation function type

	*
		- input

		- Input container (must be non-empty)

	*
		- op

		- Binary operation to combine elements

	*
		- std::invalid_argument

		- if input container is empty



.. rubric:: Returns:

Result of reduction

.. index:: pair: function; zip
.. _doxid-namespacetoolbox_1_1functional_1a14a774e92ad2ff34a0d6fb26e1929527:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename... Containers>
	CPP_TOOLBOX_EXPORT auto
	zip(const Containers&... containers)

Zip multiple containers into a vector of tuples.

.. ref-code-block:: cpp

	// Example usage of zip with multiple containers
	std::vector<int> nums = {1, 2, 3};
	std::vector<std::string> strs = {"a", "b", "c"};
	std::array<double> dbls = {1.1, 2.2, 3.3};
	
	auto zipped = :ref:`zip <doxid-namespacetoolbox_1_1functional_1a14a774e92ad2ff34a0d6fb26e1929527>`(nums, strs, dbls);
	// Returns vector of tuples: {
	//   {1, "a", 1.1},
	//   {2, "b", 2.2},
	//   {3, "c", 3.3}
	// }

.. note::

	The length of result is determined by the shortest input container



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Containers

		- Container types, must provide size() and begin()/end()

	*
		- containers

		- Input containers



.. rubric:: Returns:

Vector of tuples containing references to elements from each container

.. index:: pair: function; zip_to_unordered_map
.. _doxid-namespacetoolbox_1_1functional_1a4fb6acc3f4d90809e63e519440e55b7c:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ContainerKeys,
		typename ContainerValues,
		typename Key = typename std::decay_t<ContainerKeys>::value_type,
		typename Value = typename std::decay_t<ContainerValues>::value_type,
		typename Hash = std::hash<Key>,
		typename KeyEqual = std::equal_to<Key>,
		typename Alloc = std::allocator<std::pair<const Key, Value>>
	>
	CPP_TOOLBOX_EXPORT auto
	zip_to_unordered_map(
		const ContainerKeys& keys,
		const ContainerValues& values
	)

Zip two sequences into an unordered_map.

.. ref-code-block:: cpp

	// Example usage of zip_to_unordered_map
	std::vector<std::string> keys = {"a", "b", "c"};
	std::vector<int> values = {1, 2, 3};
	
	auto :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>` = :ref:`zip_to_unordered_map <doxid-namespacetoolbox_1_1functional_1a4fb6acc3f4d90809e63e519440e55b7c>`(keys, values);
	// Returns unordered_map: {
	//   {"a", 1},
	//   {"b", 2},
	//   {"c", 3}
	// }

.. note::

	If keys container has duplicates, only the first occurrence is used



.. note::

	The number of pairs is determined by the shorter container



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ContainerKeys

		- Key container type

	*
		- ContainerValues

		- Value container type

	*
		- Key

		- Map key type (default: ContainerKeys::value_type)

	*
		- Value

		- Map value type (default: ContainerValues::value_type)

	*
		- Hash

		- Key hash function type

	*
		- KeyEqual

		- Key equality comparison function type

	*
		- Alloc

		- Map allocator type

	*
		- keys

		- Container of keys

	*
		- values

		- Container of values



.. rubric:: Returns:

Unordered map containing key-value pairs

.. index:: pair: function; memoize
.. _doxid-namespacetoolbox_1_1functional_1a8c3f8127f603901d466318d6aaff381f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Signature,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	memoize(Func&& f)

Create a memoized function with explicit signature.

.. ref-code-block:: cpp

	// Example usage of memoize with explicit signature
	auto memoized_add = memoize<int(int,int)>([](int a, int b) { return a + b;
	}); int result1 = memoized_add(2, 3); // Computes and caches int result2 =
	memoized_add(2, 3); // Returns cached result



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Signature

		- Function signature type

	*
		- Func

		- Function object type

	*
		- f

		- Function to memoize



.. rubric:: Returns:

MemoizedFunction object

.. index:: pair: function; memoize_explicit
.. _doxid-namespacetoolbox_1_1functional_1ae451cc75b6a39fd16d3c1c680988706e:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename R,
		typename... Args,
		typename Func
	>
	CPP_TOOLBOX_EXPORT auto
	memoize_explicit(Func&& f)

Create a memoized function with explicit return and argument types.

.. ref-code-block:: cpp

	// Example usage of memoize_explicit
	auto fib = memoize_explicit<int, int>([&](int n) {
	  if (n <= 1) return n;
	  return fib(n-1) + fib(n-2);
	});
	int result = fib(10); // Efficiently computes fibonacci

.. note::

	Thread-safe implementation using shared state



.. note::

	Arguments must form valid map key after decay



.. note::

	Return type must be copyable



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- R

		- Return type

	*
		- Args

		- Argument types

	*
		- Func

		- Function object type

	*
		- f

		- Function to memoize



.. rubric:: Returns:

std::function with memoization

.. index:: pair: function; compose
.. _doxid-namespacetoolbox_1_1functional_1a73614e7a6d879b7e2ff3be342d01f832:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename G,
		typename F
	>
	auto
	compose(
		G&& g,
		F&& f
	)

Composes two functions into a single function.

.. ref-code-block:: cpp

	// Example of function composition
	auto add_one = [](int x) { return x + 1; };
	auto multiply_two = [](int x) { return x * 2; };
	auto composed = :ref:`compose <doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`(multiply_two, add_one);
	int result = composed(5); // Returns 12: ((5 + 1) * 2)



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- G

		- Type of outer function

	*
		- F

		- Type of inner function

	*
		- g

		- Outer function

	*
		- f

		- Inner function



.. rubric:: Returns:

A function that applies f then g

.. index:: pair: function; compose
.. _doxid-namespacetoolbox_1_1functional_1a62235f5b9cd7bdf5f6754cb243e703a4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename F1,
		typename... FRest
	>
	auto
	compose(
		F1&& f1,
		FRest&&... rest
	)

Composes multiple functions into a single function.

.. ref-code-block:: cpp

	// Example of multiple function composition
	auto add_one = [](int x) { return x + 1; };
	auto multiply_two = [](int x) { return x * 2; };
	auto square = [](int x) { return x * x; };
	auto composed = :ref:`compose <doxid-namespacetoolbox_1_1functional_1af8b28e1bd761680b1a9dcc57201c19ee>`(square, multiply_two, add_one);
	int result = composed(5); // Returns 144: ((5 + 1) * 2)^2



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F1

		- Type of first (outermost) function

	*
		- FRest

		- Types of remaining functions

	*
		- f1

		- First function

	*
		- rest

		- Remaining functions



.. rubric:: Returns:

A function that applies functions from right to left

.. index:: pair: function; bind_first
.. _doxid-namespacetoolbox_1_1functional_1a1e0ae6022a29dcda8ebca974573bf70a:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename F,
		typename Arg1
	>
	auto
	bind_first(
		F&& f,
		Arg1&& arg1
	)

Binds the first argument of a function.

.. ref-code-block:: cpp

	// Example of binding first argument
	auto divide = [](int x, int y) { return x / y; };
	auto divide_10_by = :ref:`bind_first <doxid-namespacetoolbox_1_1functional_1aecc9e3b337e5a7e2d3d9eaac80939002>`(divide, 10);
	int result = divide_10_by(2); // Returns 5



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- F

		- Function type

	*
		- Arg1

		- Type of first argument

	*
		- f

		- Function to bind

	*
		- arg1

		- Value to bind as first argument



.. rubric:: Returns:

Function with first argument bound

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a4ab57c3803f4ac4bb491400a3b6a4233:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	auto
	map(
		const std::optional<T>& opt,
		F&& f
	)

Maps a function over an optional value.

.. ref-code-block:: cpp

	// Example of mapping over optional
	std::optional<int> opt(5);
	auto times_two = [](int x) { return x * 2; };
	auto result = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(opt, times_two); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type

	*
		- opt

		- Optional to map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Optional containing result of f if opt has value, empty optional otherwise

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a86d8ff17712cc38ece6f1a84548dda26:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	auto
	map(
		std::optional<T>&& opt,
		F&& f
	)

Maps a function over an optional rvalue.

.. ref-code-block:: cpp

	// Example of mapping over optional rvalue
	auto result = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(std::optional<int>(5), [](int x) { return x * 2; }); //
	Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type

	*
		- opt

		- Optional rvalue to map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Optional containing result of f if opt has value, empty optional otherwise

.. index:: pair: function; flatMap
.. _doxid-namespacetoolbox_1_1functional_1a424450bf0ce67fb53903b20dee8b1765:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	auto
	flatMap(
		const std::optional<T>& opt,
		F&& f
	)

Flat maps a function over an optional value.

.. ref-code-block:: cpp

	// Example of flat mapping over optional
	std::optional<int> opt(5);
	auto maybe_double = [](int x) -> std::optional<int> {
	  return x < 10 ? std::optional(x * 2) : std::nullopt;
	};
	auto result = :ref:`flatMap <doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd>`(opt, maybe_double); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning optional

	*
		- opt

		- Optional to flat map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Result of f if opt has value, empty optional otherwise

.. index:: pair: function; flatMap
.. _doxid-namespacetoolbox_1_1functional_1a20f50536887abe0448cb0484c4ec3abb:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	auto
	flatMap(
		std::optional<T>&& opt,
		F&& f
	)

Flat maps a function over an optional rvalue.

.. ref-code-block:: cpp

	// Example of flat mapping over optional rvalue
	auto result = :ref:`flatMap <doxid-namespacetoolbox_1_1functional_1abea94163fa9d90b3c3a81a6f82fd31cd>`(std::optional<int>(5),
	                     [](int x) -> std::optional<int> {
	                       return x < 10 ? std::optional(x * 2) : std::nullopt;
	                     }); // Contains 10



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning optional

	*
		- opt

		- Optional rvalue to flat map over

	*
		- f

		- Function to apply



.. rubric:: Returns:

Result of f if opt has value, empty optional otherwise

.. index:: pair: function; orElse
.. _doxid-namespacetoolbox_1_1functional_1ad0b4c848e551d917eebb0bebfa50fc66:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename U
	>
	auto
	orElse(
		const std::optional<T>& opt,
		U&& default_value
	)

Returns the contained value or a default.

.. ref-code-block:: cpp

	// Example of providing default value
	std::optional<int> opt;
	int result = :ref:`orElse <doxid-namespacetoolbox_1_1functional_1a50e389bf7d4e1f5788548cd2784de28f>`(opt, 42); // Returns 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- U

		- Type of default value

	*
		- opt

		- Optional to get value from

	*
		- default_value

		- Value to return if optional is empty



.. rubric:: Returns:

Contained value or default

.. index:: pair: function; orElseGet
.. _doxid-namespacetoolbox_1_1functional_1ae1baad6982df3cde2d03724aa870e274:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename F
	>
	auto
	orElseGet(
		const std::optional<T>& opt,
		F&& default_func
	)

Returns the contained value or calls function for default.

.. ref-code-block:: cpp

	// Example of providing default via function
	std::optional<int> opt;
	auto get_default = []() { return 42; };
	int result = :ref:`orElseGet <doxid-namespacetoolbox_1_1functional_1a4a7a4acdd8ba7485698f9ea3e64920e9>`(opt, get_default); // Returns 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- F

		- Function type returning default value

	*
		- opt

		- Optional to get value from

	*
		- default_func

		- Function to call for default value



.. rubric:: Returns:

Contained value or result of default_func

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1a68ecec782b192847390f333b4af52dc3:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename P
	>
	auto
	filter(
		const std::optional<T>& opt,
		P&& p
	)

Filters an optional based on a predicate.

.. ref-code-block:: cpp

	// Example of filtering optional
	std::optional<int> opt(42);
	auto is_even = [](int x) { return x % 2 == 0; };
	auto result = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(opt, is_even); // Contains 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- P

		- Predicate type

	*
		- opt

		- Optional to filter

	*
		- p

		- Predicate to apply



.. rubric:: Returns:

Optional if predicate returns true, empty optional otherwise

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1adb40cb4be7d2e760e5cf545c558aa87f:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename T,
		typename P
	>
	auto
	filter(
		std::optional<T>&& opt,
		P&& p
	)

Filters an optional rvalue based on a predicate.

.. ref-code-block:: cpp

	// Example of filtering optional rvalue
	auto result = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(std::optional<int>(42), [](int x) { return x % 2 == 0;
	}); // Contains 42



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- T

		- Type contained in optional

	*
		- P

		- Predicate type

	*
		- opt

		- Optional rvalue to filter

	*
		- p

		- Predicate to apply



.. rubric:: Returns:

Optional if predicate returns true, empty optional otherwise

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1adfc20d2de6408ff5e8bda0b7174c2bf9:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	match(
		const std::variant<Ts...>& var,
		Fs&&... visitors
	)

Pattern matches on a variant using visitor functions.

.. ref-code-block:: cpp

	// Example of pattern matching on variant
	std::variant<int, std::string> v = 42;
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(v,
	  [](int i) { return i * 2; },
	  [](const std::string& s) { return s.length(); }
	); // Returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1a31150fb88996d76590ccc35593c7b70d:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	match(
		std::variant<Ts...>& var,
		Fs&&... visitors
	)

Match variant with visitor functions for non-const lvalue variant.

.. ref-code-block:: cpp

	// Example of pattern matching on lvalue variant
	std::variant<int, std::string> v = 42;
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(v,
	  [](int& i) { i *= 2; return i; },
	  [](std::string& s) { s += "!"; return s.length(); }
	); // Modifies v and returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; match
.. _doxid-namespacetoolbox_1_1functional_1a48db12c30c97eddfdbe5bb80179b94f4:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename... Ts,
		typename... Fs
	>
	auto
	match(
		std::variant<Ts...>&& var,
		Fs&&... visitors
	)

Match variant with visitor functions for rvalue variant.

.. ref-code-block:: cpp

	// Example of pattern matching on rvalue variant
	auto result = :ref:`match <doxid-namespacetoolbox_1_1functional_1a76f1f065e91d8b2571f14cc350983a02>`(std::variant<int, std::string>(42),
	  [](int&& i) { return i * 2; },
	  [](std::string&& s) { return s + "!"; }
	); // Returns 84



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Ts

		- Types in variant

	*
		- Fs

		- Visitor function types

	*
		- var

		- Variant to match on

	*
		- visitors

		- Visitor functions for each type



.. rubric:: Returns:

Result of matched visitor

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a65a3a49ee3d8b5063e93e806d350db95:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	map(
		const std::variant<Ts...>& var,
		F&& f
	)

Apply a function to the value held by a variant and return result in a new variant.

.. ref-code-block:: cpp

	// Example of mapping over variant
	std::variant<int, std::string> v = 42;
	auto result = map<std::variant<double, size_t>>(v,
	  [](const auto& x) -> std::variant<double, size_t> {
	    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
	      return x * 1.5; // int -> double
	    } else {
	      return x.length(); // string -> size_t
	    }
	  }
	); // Returns variant containing 63.0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value

	*
		- std::bad_variant_access

		- if var is valueless_by_exception

	*
		- Any

		- exception thrown by f

	*
		- std::bad_variant_access

		- if f's result cannot construct any ResultVariant type



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1abe0946f6ce3008057d1e5a6c5ee9b120:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	map(
		std::variant<Ts...>& var,
		F&& f
	)

Apply a function to the value held by a non-const lvalue variant.

.. ref-code-block:: cpp

	// Example of mapping over lvalue variant
	std::variant<int, std::string> v = 42;
	auto result = map<std::variant<double>>(v,
	  [](int& x) { x *= 2; return x * 1.5; }
	); // Modifies v and returns variant containing 126.0



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1a6ec605443bd2b2ee732a30baf2427805:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ResultVariant,
		typename... Ts,
		typename F
	>
	auto
	map(
		std::variant<Ts...>&& var,
		F&& f
	)

Apply a function to the value held by an rvalue variant.

.. ref-code-block:: cpp

	// Example of mapping over rvalue variant
	auto result = map<std::variant<std::string>>(
	  std::variant<int, std::string>(42),
	  [](auto&& x) { return :ref:`std::to_string <doxid-namespacetoolbox_1_1container_1_1string_1a093a4692179779a03018bc34fb696b60>`(x); }
	); // Returns variant containing "42"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ResultVariant

		- The target variant type to return

	*
		- Ts

		- Types in input variant

	*
		- F

		- Function type to apply

	*
		- var

		- Input variant

	*
		- f

		- Function to apply to variant value



.. rubric:: Returns:

ResultVariant containing f's result

.. index:: pair: function; map
.. _doxid-namespacetoolbox_1_1functional_1aba27921b0ebcf50a284153a64c156a57:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename Func
	>
	auto
	map(
		const Container& input,
		Func&& f
	)

Apply a function to each element in a container and return results in a new vector.

.. ref-code-block:: cpp

	// Example usage of map on containers
	std::vector<int> nums = {1, 2, 3};
	auto squares = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(nums, [](int x) { return x * x; }); // Returns {1, 4, 9}
	
	std::list<std::string> strs = {"a", "bb", "ccc"};
	auto lengths = :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>`(strs, [](const auto& s) { return s.length(); }); //
	Returns {1, 2, 3}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- Func

		- Function type to apply to elements

	*
		- input

		- Input container

	*
		- f

		- Function to apply to each element



.. rubric:: Returns:

Vector containing results of applying f to each element

.. index:: pair: function; filter
.. _doxid-namespacetoolbox_1_1functional_1aaaf8da56d5d7e9e09986db066318b5cf:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename Predicate
	>
	auto
	filter(
		const Container& input,
		Predicate&& p
	)

Filter elements from a container that satisfy a predicate.

.. ref-code-block:: cpp

	// Example usage of filter
	std::vector<int> nums = {1, 2, 3, 4, 5};
	auto evens = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(nums, [](int x) { return x % 2 == 0; }); // Returns {2,
	4}
	
	std::list<std::string> strs = {"a", "bb", "ccc"};
	auto long_strs = :ref:`filter <doxid-namespacetoolbox_1_1functional_1a4e0c6694011c494298113d01b2b3e9de>`(strs, [](const auto& s) { return s.length() > 1; });
	// Returns {"bb", "ccc"}



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- Predicate

		- Predicate function type

	*
		- input

		- Input container

	*
		- p

		- Predicate function returning bool



.. rubric:: Returns:

Vector containing elements that satisfy predicate p

.. index:: pair: function; reduce
.. _doxid-namespacetoolbox_1_1functional_1aea3f2ee7ec3d87022b31c2f07743c4e6:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename T,
		typename BinaryOp
	>
	auto
	reduce(
		const Container& input,
		T identity,
		BinaryOp&& op
	)

Reduce container elements using a binary operation with initial value.

.. ref-code-block:: cpp

	// Example usage of reduce with identity value
	std::vector<int> nums = {1, 2, 3, 4};
	auto sum = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, 0, std::plus<int>()); // Returns 10
	auto product = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, 1, std::multiplies<int>()); // Returns 24
	
	std::list<std::string> strs = {"a", "b", "c"};
	auto concat = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(strs, std::string(), std::plus<std::string>()); //
	Returns "abc"



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- T

		- Initial value and result type

	*
		- BinaryOp

		- Binary operation function type

	*
		- input

		- Input container

	*
		- identity

		- Initial value (identity element)

	*
		- op

		- Binary operation to combine elements



.. rubric:: Returns:

Result of reduction

.. index:: pair: function; reduce
.. _doxid-namespacetoolbox_1_1functional_1a10b525e7afccde1000246e289b1469af:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Container,
		typename BinaryOp
	>
	auto
	reduce(
		const Container& input,
		BinaryOp&& op
	)

Reduce non-empty container elements using a binary operation.

.. ref-code-block:: cpp

	// Example usage of reduce without identity value
	std::vector<int> nums = {1, 2, 3, 4};
	auto sum = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(nums, std::plus<int>()); // Returns 10
	
	std::list<std::string> strs = {"a", "b", "c"};
	auto concat = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(strs, std::plus<std::string>()); // Returns "abc"
	
	// This will throw:
	std::vector<int> empty;
	auto result = :ref:`reduce <doxid-namespacetoolbox_1_1functional_1a28234663d88d2ee0d2a8fc72044235e3>`(empty, std::plus<int>()); // throws
	std::invalid_argument



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Container

		- Input container type, must provide begin() and end() iterators

	*
		- BinaryOp

		- Binary operation function type

	*
		- input

		- Input container (must be non-empty)

	*
		- op

		- Binary operation to combine elements

	*
		- std::invalid_argument

		- if input container is empty



.. rubric:: Returns:

Result of reduction

.. index:: pair: function; zip
.. _doxid-namespacetoolbox_1_1functional_1a5b667889be637834a733f0dbf6a145e7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <typename... Containers>
	auto
	zip(const Containers&... containers)

Zip multiple containers into a vector of tuples.

.. ref-code-block:: cpp

	// Example usage of zip with multiple containers
	std::vector<int> nums = {1, 2, 3};
	std::vector<std::string> strs = {"a", "b", "c"};
	std::array<double> dbls = {1.1, 2.2, 3.3};
	
	auto zipped = :ref:`zip <doxid-namespacetoolbox_1_1functional_1a14a774e92ad2ff34a0d6fb26e1929527>`(nums, strs, dbls);
	// Returns vector of tuples: {
	//   {1, "a", 1.1},
	//   {2, "b", 2.2},
	//   {3, "c", 3.3}
	// }

.. note::

	The length of result is determined by the shortest input container



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Containers

		- Container types, must provide size() and begin()/end()

	*
		- containers

		- Input containers



.. rubric:: Returns:

Vector of tuples containing references to elements from each container

.. index:: pair: function; zip_to_unordered_map
.. _doxid-namespacetoolbox_1_1functional_1a04b877b2a05cd06d327adb9f9bd273ea:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename ContainerKeys,
		typename ContainerValues,
		typename Key,
		typename Value,
		typename Hash,
		typename KeyEqual,
		typename Alloc
	>
	auto
	zip_to_unordered_map(
		const ContainerKeys& keys,
		const ContainerValues& values
	)

Zip two sequences into an unordered_map.

.. ref-code-block:: cpp

	// Example usage of zip_to_unordered_map
	std::vector<std::string> keys = {"a", "b", "c"};
	std::vector<int> values = {1, 2, 3};
	
	auto :ref:`map <doxid-namespacetoolbox_1_1functional_1a17a6accc8bba74382744d1e09aa210a7>` = :ref:`zip_to_unordered_map <doxid-namespacetoolbox_1_1functional_1a4fb6acc3f4d90809e63e519440e55b7c>`(keys, values);
	// Returns unordered_map: {
	//   {"a", 1},
	//   {"b", 2},
	//   {"c", 3}
	// }

.. note::

	If keys container has duplicates, only the first occurrence is used



.. note::

	The number of pairs is determined by the shorter container



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- ContainerKeys

		- Key container type

	*
		- ContainerValues

		- Value container type

	*
		- Key

		- Map key type (default: ContainerKeys::value_type)

	*
		- Value

		- Map value type (default: ContainerValues::value_type)

	*
		- Hash

		- Key hash function type

	*
		- KeyEqual

		- Key equality comparison function type

	*
		- Alloc

		- Map allocator type

	*
		- keys

		- Container of keys

	*
		- values

		- Container of values



.. rubric:: Returns:

Unordered map containing key-value pairs

.. index:: pair: function; memoize
.. _doxid-namespacetoolbox_1_1functional_1a67ae7d48b6cf42eb8a27dbf3a970167b:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename Signature,
		typename Func
	>
	auto
	memoize(Func&& f)

Create a memoized function with explicit signature.

.. ref-code-block:: cpp

	// Example usage of memoize with explicit signature
	auto memoized_add = memoize<int(int,int)>([](int a, int b) { return a + b;
	}); int result1 = memoized_add(2, 3); // Computes and caches int result2 =
	memoized_add(2, 3); // Returns cached result



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- Signature

		- Function signature type

	*
		- Func

		- Function object type

	*
		- f

		- Function to memoize



.. rubric:: Returns:

MemoizedFunction object

.. index:: pair: function; memoize_explicit
.. _doxid-namespacetoolbox_1_1functional_1af4ca279b16162dbbf7042fc2681bdba7:

.. ref-code-block:: cpp
	:class: doxyrest-title-code-block

	template <
		typename R,
		typename... Args,
		typename Func
	>
	auto
	memoize_explicit(Func&& f)

Create a memoized function with explicit return and argument types.

.. ref-code-block:: cpp

	// Example usage of memoize_explicit
	auto fib = memoize_explicit<int, int>([&](int n) {
	  if (n <= 1) return n;
	  return fib(n-1) + fib(n-2);
	});
	int result = fib(10); // Efficiently computes fibonacci

.. note::

	Thread-safe implementation using shared state



.. note::

	Arguments must form valid map key after decay



.. note::

	Return type must be copyable



.. rubric:: Parameters:

.. list-table::
	:widths: 20 80

	*
		- R

		- Return type

	*
		- Args

		- Argument types

	*
		- Func

		- Function object type

	*
		- f

		- Function to memoize



.. rubric:: Returns:

std::function with memoization


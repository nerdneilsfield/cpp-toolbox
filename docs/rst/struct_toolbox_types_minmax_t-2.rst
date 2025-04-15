.. index:: pair: struct; toolbox::types::minmax_t<point_t<T>>
.. _doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4:

template struct toolbox::types::minmax_t<point_t<T>>
====================================================

.. toctree::
	:hidden:

Partial specialization of :ref:`minmax_t <doxid-structtoolbox_1_1types_1_1minmax__t>` for point_t<T>.


.. ref-code-block:: cpp
	:class: doxyrest-overview-code-block

	#include <minmax.hpp>
	
	template <typename T>
	struct minmax_t<point_t<T>> {
		// fields
	
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T> :target:`min<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1ae278fe9204e6fc84e0625dffc8e6ff80>`;
		:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T> :target:`max<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1a508977c45d2c8a6da75c81076f922619>`;
		bool :target:`initialized_<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1ad7a2654ca9a193bcee82ccfd95b55d2e>` = false;

		// construction
	
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1ace7496bac5f36669d5dd1c2ccb343d4a>`();
	
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1ae464184e2631cd1311c64810354e1a67>`(
			:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T> initial_min,
			:ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T> initial_max
		);
	
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1a6bb14f7db6d39ad663a0fbb9ebdbcc8c>`(const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&);
		:target:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1a892929fc5c9876cc1e68bfab0e5fde4d>`(:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&&);
		:target:`~minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1adedcf7dd0b0f5f7411744ea62aa27c6a>`();

		// methods
	
		:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&
		:target:`operator=<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1aa8ee092d6a2551bc9f93b2f7ccd78091>`(const :ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&);
	
		:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&
		:target:`operator=<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1a73bf2ccbec024b7295b336b007aa0c31>`(:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&&);
	
		:ref:`minmax_t<doxid-structtoolbox_1_1types_1_1minmax__t>`&
		:target:`operator+=<doxid-structtoolbox_1_1types_1_1minmax__t_3_01point__t_3_01T_01_4_01_4_1a716010dfdf38ab9b3e33b2109af1bc0e>`(const :ref:`point_t<doxid-structtoolbox_1_1types_1_1point__t>`<T>& value);
	};

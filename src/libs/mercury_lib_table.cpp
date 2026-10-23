#include "mercury_lib_table.hpp"
#include"../mercury.hpp"
#include"../mercury_error.hpp"

#include <malloc.h>
#include <cstring>

void mercury_lib_table_copy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //basically the same as array.copy
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable tab_var;
	mercury_popstack(M,&tab_var);
	if (tab_var.type != M_TYPE_TABLE) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tab_var, M_TYPE_TABLE, 1);
		mercury_release_var(&tab_var);
		return;
	}

	mercury_variable new_tab_var;

	mercury_table* newtab = mercury_newtable();
	mercury_table* oldtab = (mercury_table*)tab_var.data.p;

	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++){
		mercury_subtable st_n = newtab->data[t];
		mercury_subtable st_o = oldtab->data[t];

		st_n.values=(mercury_variable*)malloc(sizeof(mercury_variable) * st_o.size);
		if (!st_n.values) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		
		st_n.keys = (mercury_variable*)malloc(sizeof(mercury_variable) * st_o.size);
		if (!st_n.keys) {
			free(st_n.values);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}

		st_n.size = st_o.size;

		for (mercury_int i = 0; i < st_o.size; i++) {
			mercury_increment_variable_refrence_count(st_o.keys + i);
			mercury_increment_variable_refrence_count(st_o.values + i);
			st_n.keys[i] = st_o.keys[i];
			st_n.values[i] = st_o.values[i];
		}

		newtab->data[t] = st_n;
	}
	newtab->refrences = 1;

	new_tab_var.type = M_TYPE_TABLE;
	new_tab_var.data.p = newtab;
	mercury_pushstack(M, &new_tab_var);

	mercury_release_var(&tab_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_table_flush(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //basically the same as array.flush
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable tab_var;
	mercury_popstack(M, &tab_var);
	if (tab_var.type != M_TYPE_TABLE) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tab_var, M_TYPE_TABLE, 1);
		mercury_release_var(&tab_var);
		return;
	}
	mercury_table* tab = (mercury_table*)tab_var.data.p;
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable st = tab->data[t];
		if (!st.size)continue;
		mercury_int subtracted_size = 0;
		for (mercury_int i = 0; i < st.size; i++) { //for each value
			if (!st.values[i].type) { //that is nil
				for (mercury_int i2 = (st.size- subtracted_size); i2 > i; i2--) { //look through each value, start high
					if (st.values[i2].type) { //then go lower to find a non-nil value
						st.values[i] = st.values[i2];
						mercury_decrement_variable_refrence_count(st.keys+i);  //if we do, place the value at our position, and free our key
						mercury_release_var(st.keys+i);
						st.keys[i] = st.keys[i2];
						break;
					}
				}
				subtracted_size++; //either way increment the counter.
			}
		}
		st.size -= subtracted_size;
		void* nptr=realloc(st.keys, sizeof(mercury_variable) * st.size);
		if (!nptr && st.size) { //we're decreasing the size (or keeping the same) so this should theoretically never fail. but still. be safe.
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_release_var(&tab_var);
			return;
		}
		st.keys = (mercury_variable*)nptr;
		nptr = realloc(st.values, sizeof(mercury_variable) * st.size);
		if (!nptr && st.size) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_release_var(&tab_var);
			return;
		}
		st.values = (mercury_variable*)nptr;
		tab->data[t] = st;
	}

	mercury_release_var(&tab_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void mercury_lib_table_getkeys(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //array of all keys
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable tab_var;
	mercury_popstack(M, &tab_var);
	if (tab_var.type != M_TYPE_TABLE) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tab_var, M_TYPE_TABLE, 1);
		mercury_release_var(&tab_var);
		return;
	}
	mercury_int counter = 0;
	mercury_table* tab = (mercury_table*)tab_var.data.p;
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable st = tab->data[t];
		if (!st.size)continue;
		for (mercury_int i = 0; i < st.size; i++) { //for each value
			if (!mercury_setarray(arr, st.keys + i, counter)) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				mercury_destroyarray(arr);
				mercury_release_var(&tab_var);
				return;
			}
			counter++;
		}
	}

	mercury_release_var(&tab_var);

	mercury_variable out;
	out.type = M_TYPE_ARRAY;
	out.data.p = arr;
	mercury_pushstack(M, &out);
	if (args_out > 1) {
		out.type = M_TYPE_INT;
		out.data.i = counter;
		mercury_pushstack(M, &out);
	}
	

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,2);
}

void mercury_lib_table_getvalues(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //array of all keys
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable tab_var;
	mercury_popstack(M, &tab_var);
	if (tab_var.type != M_TYPE_TABLE) {
		mercury_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tab_var, M_TYPE_TABLE, 1);
		mercury_release_var(&tab_var);
		return;
	}
	mercury_int counter = 0;
	mercury_table* tab = (mercury_table*)tab_var.data.p;
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable st = tab->data[t];
		if (!st.size)continue;
		for (mercury_int i = 0; i < st.size; i++) { //for each value
			if (!mercury_setarray(arr, st.values + i, counter)) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				mercury_destroyarray(arr);
				mercury_release_var(&tab_var);
				return;
			}
			counter++;
		}
	}

	mercury_release_var(&tab_var);

	mercury_variable out;
	out.type = M_TYPE_ARRAY;
	out.data.p = arr;
	mercury_pushstack(M, &out);
	if (args_out > 1) {
		out.type = M_TYPE_INT;
		out.data.i = counter;
		mercury_pushstack(M, &out);
	}


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 2);
}


void mercury_lib_table_haskey(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //array of all keys
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable key_var;
	mercury_popstack(M, &key_var);

	mercury_variable tab_var;
	mercury_popstack(M, &tab_var);

	mercury_variable out;

	mercury_table* tab = (mercury_table*)tab_var.data.p;
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable st = tab->data[t];
		if (!st.size)continue;
		for (mercury_int i = 0; i < st.size; i++) {
			if (mercury_vars_equal(st.keys + i, &key_var)) {
				out.type = M_TYPE_BOOL;
				out.data.i = 1;
				mercury_pushstack(M, &out);
				if (args_out > 1) {
					mercury_pushstack(M, st.values+i);
				}
				return;
			}
		}
	}

	out.type = M_TYPE_BOOL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
	if (args_out > 1) {
		out.type = M_TYPE_NIL;
		mercury_pushstack(M, &out);
	}

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 2);
}


void mercury_lib_table_hasvalue(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //array of all keys
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable key_var;
	mercury_popstack(M, &key_var);

	mercury_variable tab_var;
	mercury_popstack(M, &tab_var);

	mercury_variable out;

	mercury_table* tab = (mercury_table*)tab_var.data.p;
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable st = tab->data[t];
		if (!st.size)continue;
		for (mercury_int i = 0; i < st.size; i++) {
			if (mercury_vars_equal(st.values + i, &key_var)) {
				out.type = M_TYPE_BOOL;
				out.data.i = 1;
				mercury_pushstack(M, &out);
				if (args_out > 1) {
					mercury_pushstack(M, st.keys + i);
				}
				return;
			}
		}
	}

	out.type = M_TYPE_BOOL;
	out.data.i = 0;
	mercury_pushstack(M, &out);
	if (args_out > 1) {
		out.type = M_TYPE_NIL;
		mercury_pushstack(M, &out);
	}

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 2);
}
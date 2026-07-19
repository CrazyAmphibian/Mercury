#include "mercury_lib_table.hpp"
#include"../mercury.hpp"
#include"../mercury_error.hpp"

#include <malloc.h>
#include <cstring>

void mercury_lib_table_copy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //basically the same as array.copy
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1);
	if (!args_out) {
		return;
	}

	mercury_variable tab_var;
	mercury_popstack(M,&tab_var);
	if (tab_var.type != M_TYPE_TABLE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, tab_var.type, M_TYPE_TABLE, 1);
		return;
	}

	mercury_variable new_tab_var;

	mercury_table* newtab = mercury_newtable();
	mercury_table* oldtab = (mercury_table*)tab_var.data.p;

	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++){
		mercury_subtable* st_n = newtab->data[t];
		mercury_subtable* st_o = oldtab->data[t];

		st_n->values=(mercury_variable*)malloc(sizeof(mercury_variable) * st_o->size);
		if (!st_n->values) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		
		st_n->keys = (mercury_variable*)malloc(sizeof(mercury_variable) * st_o->size);
		if (!st_n->keys) {
			free(st_n->values);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}

		st_n->size = st_o->size;

		for (mercury_int i = 0; i < st_o->size; i++) {
			mercury_clonevariable(st_o->keys+i, st_n->keys+i);
			mercury_clonevariable(st_o->values+i, st_n->values+i);
		}
	}
	newtab->refrences = 1;

	new_tab_var.type = M_TYPE_TABLE;
	new_tab_var.data.p = newtab;
	mercury_pushstack_unrefed(M, &new_tab_var);

	mercury_free_var(&tab_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}
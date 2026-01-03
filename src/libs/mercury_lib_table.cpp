#include "mercury_lib_table.hpp"
#include"../mercury.hpp"
#include"../mercury_error.hpp"

#include <malloc.h>
#include <cstring>

void mercury_lib_table_copy(mercury_state* M, mercury_int args_in, mercury_int args_out) { //basically the same as array.copy
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* tab_var = mercury_popstack(M);
	if (tab_var->type != M_TYPE_TABLE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)tab_var->type, (void*)M_TYPE_TABLE);
		return;
	}

	mercury_variable* new_tab_var = mercury_assign_var(M);
	if (!new_tab_var) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	mercury_table* newtab = mercury_newtable();
	mercury_table* oldtab = (mercury_table*)tab_var->data.p;

	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++){
		mercury_subtable* st_n = newtab->data[t];
		mercury_subtable* st_o = oldtab->data[t];

		st_n->values=(mercury_variable**)malloc(sizeof(mercury_variable*) * st_o->size);
		if (!st_n->values) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		
		st_n->keys = (mercury_rawdata*)malloc(sizeof(mercury_rawdata) * st_o->size);
		if (!st_n->keys) {
			free(st_n->values);
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}

		st_n->size = st_o->size;
		memcpy(st_n->values, st_o->values, sizeof(mercury_variable*) * st_o->size);

		if (t == M_TYPE_STRING) {
			for (mercury_int i = 0; i < st_n->size; i++) {
				st_n->keys[i].p = mercury_copystring((mercury_stringliteral*)st_o->keys[i].p);
			}
		}
		else {
			memcpy(st_n->keys, st_o->keys, sizeof(mercury_rawdata) * st_o->size);
		}
		
	}
	newtab->refrences = 1;

	new_tab_var->type = M_TYPE_TABLE;
	new_tab_var->data.p = newtab;
	mercury_pushstack(M, new_tab_var);

	mercury_unassign_var(M, tab_var);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}
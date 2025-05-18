#include "mercury_lib_array.h"
#include"../mercury.h"
#include"../mercury_error.h"
#include "malloc.h"
#include <stdlib.h>
#include <math.h>

#include <stdio.h>

void mercury_lib_array_flush(mercury_state* M, mercury_int args_in, mercury_int args_out) { //discards nil values.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
		return;
	};
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* arr_var = mercury_popstack(M);
	if (arr_var->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)arr_var->type, (void*)M_TYPE_ARRAY);
		return;
	}


	mercury_array* arr = (mercury_array*)arr_var->data.p;


	mercury_array* newarr=mercury_newarray();
	if (!newarr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}

	mercury_int arraysize=0;
	mercury_variable*** p = arr->values;
	for (mercury_int block = 0; block < arr->size;block++) {
		if (!p[block])continue;
		for (mercury_int pos = 0; pos < (1 << MERCURY_ARRAY_BLOCKSIZE); pos++) {
			if (!p[block][pos])continue;
			if (p[block][pos]->type == M_TYPE_NIL) {
				mercury_free_var(p[block][pos]);
				continue;
			}
			mercury_setarray(newarr,arr->values[block][pos] ,arraysize);
			arraysize++;
		}
		free(p[block]);
	}
	free(p);
	arr->values = newarr->values;
	arr->size = newarr->size;
	free(newarr);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_array_copy(mercury_state* M, mercury_int args_in, mercury_int args_out) { //creates a copy of all values. not recursive. arrays are refrences so this will make a new one.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
		return;
	};
	if (args_out < 1) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* arr_var = mercury_popstack(M);
	if (arr_var->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)arr_var->type, (void*)M_TYPE_ARRAY);
		return;
	}

	mercury_variable* new_arr_var = mercury_assign_var(M);
	if (!new_arr_var) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}

	mercury_array* arr2= (mercury_array*)malloc(sizeof(mercury_array));
	if (!arr2) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}

	mercury_array* arr1 = (mercury_array*)arr_var->data.p;


	arr2->refrences = 1;
	arr2->size = arr1->size;
	arr2->values = (mercury_variable***)malloc(sizeof(mercury_variable**)*arr1->size );

	if (!arr2->values) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}


	for (mercury_int i1 = 0; i1 < arr1->size; i1++) {
		if (arr1->values[i1]) {
			arr2->values[i1] = (mercury_variable**)malloc(sizeof(mercury_variable*) * (1 << MERCURY_ARRAY_BLOCKSIZE));
			if (!arr2->values[i1]) {
				for (mercury_int n = 0; n<i1; n++) {
					free(arr2->values[n]);
				}
				mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
				return;
			}
			for (mercury_int i2 = 0; i2 < (1 << MERCURY_ARRAY_BLOCKSIZE); i2++) {
				if (arr1->values[i2][i2]) {
					mercury_variable* cvar = (mercury_variable*)malloc(sizeof(mercury_variable));
					if (!cvar)continue;
					cvar->type = arr1->values[i2][i2]->type;
					cvar->data.i = arr1->values[i2][i2]->data.i;
					arr2->values[i1][i2] = cvar;
				}
				else {
					arr2->values[i1][i2] = nullptr;
				}
			}
		}
		else {
			arr2->values[i1] = nullptr;
		}
	}




	new_arr_var->type = M_TYPE_ARRAY;
	new_arr_var->data.p = arr2;
	mercury_pushstack(M, new_arr_var);
	

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_array_insert(mercury_state* M, mercury_int args_in, mercury_int args_out) { //add at index, shifts values forward.
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
		return;
	};

	for (mercury_int i = 3; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* len_var = nullptr;
	if (args_in > 2) {
		len_var = mercury_popstack(M);
		if (len_var->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)len_var->type, (void*)M_TYPE_INT);
			return;
		}
	}
	mercury_variable* var_to_ins = mercury_popstack(M);

	mercury_variable* arr_var = mercury_popstack(M);
	if (arr_var->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)arr_var->type, (void*)M_TYPE_ARRAY);
		return;
	}

	mercury_array* arr = (mercury_array*)arr_var->data.p;
	mercury_int cur_len=mercury_array_len(arr);

	if (len_var) { //if len, shift values ahead to make space.
		mercury_int target = len_var->data.i;
		for (mercury_int i = cur_len; i >= target; i--) {
			mercury_setarray(arr, mercury_getarray(arr, i), i + 1);
		}
		mercury_setarray(arr, var_to_ins, target);
	}
	else { //if len is not specified, add to the end.
		mercury_setarray(arr, var_to_ins, cur_len);
	}


	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_array_remove(mercury_state* M, mercury_int args_in, mercury_int args_out) { //gets rid of a value and shifts ones after down.
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)2);
		return;
	};

	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* len_var = mercury_popstack(M);
	if (len_var->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)len_var->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_variable* arr_var = mercury_popstack(M);
	if (arr_var->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)arr_var->type, (void*)M_TYPE_ARRAY);
		return;
	}


	mercury_int target = len_var->data.i;
	mercury_array* arr = (mercury_array*)arr_var->data.p;
	mercury_int cur_len = mercury_array_len(arr);


	mercury_unassign_var(M, mercury_getarray(arr, target));
	for (mercury_int i = target; i <= cur_len; i++) {
		mercury_setarray(arr, mercury_getarray(arr, i+1), i);
	}

	/*
	if (!(cur_len & ((1 << MERCURY_ARRAY_BLOCKSIZE) - 1))) { //if we remove the first element in the block, remove it.
		free(arr->values[cur_len >> MERCURY_ARRAY_BLOCKSIZE]);
		arr->values[cur_len >> MERCURY_ARRAY_BLOCKSIZE] = nullptr;
	}
	arr->size--;
	*/


	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_array_swap(mercury_state* M, mercury_int args_in, mercury_int args_out) { //duh.
	if (args_in < 3) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
		return;
	};

	for (mercury_int i = 3; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* pos_var1 = mercury_popstack(M);
	if (pos_var1->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)pos_var1->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_variable* pos_var2 = mercury_popstack(M);
	if (pos_var2->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)pos_var2->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_variable* var_array = mercury_popstack(M);
	if (var_array->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_array->type, (void*)M_TYPE_ARRAY);
		return;
	}

	mercury_array* ar = (mercury_array*)var_array->data.p;
	mercury_int p1 = pos_var1->data.i;
	mercury_int p2 = pos_var2->data.i;

	mercury_variable* tv=mercury_getarray(ar,p1);
	mercury_setarray(ar, mercury_getarray(ar, p2), p1);
	mercury_setarray(ar, tv,p2);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




mercury_function* SORTING_M_FUNCTION = nullptr;
mercury_state* SORTING_M_STATE = nullptr;

int mercury_sort_use_mercury_function(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	mercury_state* M=mercury_newstate(SORTING_M_STATE);
	M->bytecode.instructions = SORTING_M_FUNCTION->instructions;
	M->bytecode.numberofinstructions = SORTING_M_FUNCTION->numberofinstructions;
	mercury_pushstack(M,var_b);
	mercury_pushstack(M,var_a);
	while (mercury_stepstate(M));
	mercury_variable* var_o=mercury_popstack(M);
	mercury_destroystate(M);
	if (var_o->type == M_TYPE_INT) {
		mercury_free_var(var_o);
		return var_o->data.i;
	}
	mercury_free_var(var_o);
	return 0;
}


void mercury_lib_array_sort(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)2);
		return;
	};
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* var_func = mercury_popstack(M);
	if (var_func->type != M_TYPE_FUNCTION && var_func->type != M_TYPE_CFUNC) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_func->type, (void*)M_TYPE_FUNCTION);
		return;
	}

	mercury_variable* var_array = mercury_popstack(M);
	if (var_array->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_array->type, (void*)M_TYPE_ARRAY);
		return;
	}

	mercury_array* arr = (mercury_array*)var_array->data.p;
	mercury_int arr_size=mercury_array_len(arr);
	mercury_variable** tlist=(mercury_variable**)malloc(sizeof(mercury_variable*)*arr_size);
	if (!tlist) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}
	for (mercury_int i = 0; i < arr_size; i++) { //this could probably be optimized with a memcpy. oh well.
		tlist[i] = mercury_getarray(arr, i);
	}

	if (var_func->type == M_TYPE_CFUNC) {
		qsort(tlist, arr_size, sizeof(mercury_variable*), (int (*)(const void*, const void*))(var_func->data.p) ); //be careful with C, dummy.
	}
	else {
		SORTING_M_FUNCTION = (mercury_function*)var_func->data.p;
		SORTING_M_STATE = M;
		qsort(tlist, arr_size, sizeof(mercury_variable*), mercury_sort_use_mercury_function); //surely this will work.
	}

	for (mercury_int i = 0; i < arr_size; i++) { //see above.
		mercury_setarray(arr, tlist[i],i);
	}
	free(tlist);
	
	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




int mercury_sort_greater_to_lesser(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;
	if (var_a->type != M_TYPE_INT && var_a->type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b->type != M_TYPE_INT && var_b->type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a->type = M_TYPE_INT) {
		if (var_b->type = M_TYPE_INT) {
			return (var_a->data.i < var_b->data.i) - (var_a->data.i > var_b->data.i);
		}
		else {
			return (var_a->data.i < var_b->data.f) - (var_a->data.i > var_b->data.f);
		}
	}
	else {
		if (var_b->type = M_TYPE_INT) {
			return (var_a->data.f < var_b->data.i) - (var_a->data.f > var_b->data.i);
		}
		else {
			return (var_a->data.f < var_b->data.f) - (var_a->data.f > var_b->data.f);
		}
	}
	return 0;
}

int mercury_sort_lesser_to_greater(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	if (var_a->type != M_TYPE_INT && var_a->type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b->type != M_TYPE_INT && var_b->type != M_TYPE_FLOAT) {
		return 0;
	}
	
	if (var_a->type = M_TYPE_INT) {
		if (var_b->type = M_TYPE_INT) {
			return (var_a->data.i > var_b->data.i) - (var_a->data.i < var_b->data.i);
		}
		else {
			return (var_a->data.i > var_b->data.f) - (var_a->data.i < var_b->data.f);
		}
	}
	else {
		if (var_b->type = M_TYPE_INT) {
			return (var_a->data.f > var_b->data.i) - (var_a->data.f < var_b->data.i);
		}
		else {
			return (var_a->data.f > var_b->data.f) - (var_a->data.f < var_b->data.f);
		}
	}
	return 0;
}

int mercury_sort_greater_to_lesser_absolute(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	if (var_a->type != M_TYPE_INT && var_a->type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b->type != M_TYPE_INT && var_b->type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a->type = M_TYPE_INT) {
		if (var_b->type = M_TYPE_INT) {
			return (abs(var_a->data.i) < abs(var_b->data.i)) - (abs(var_a->data.i) > abs(var_b->data.i));
		}
		else {
			return (abs(var_a->data.i) < fabs(var_b->data.f) ) - (abs(var_a->data.i) > fabs(var_b->data.f));
		}
	}
	else {
		if (var_b->type = M_TYPE_INT) {
			return (fabs(var_a->data.f) < abs(var_b->data.i)) - (fabs(var_a->data.f) > abs(var_b->data.i));
		}
		else {
			return (fabs(var_a->data.f) < fabs(var_b->data.f)) - (fabs(var_a->data.f) > fabs(var_b->data.f));
		}
	}
	return 0;
}




int mercury_sort_lesser_to_greater_absolute(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	if (var_a->type != M_TYPE_INT && var_a->type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b->type != M_TYPE_INT && var_b->type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a->type = M_TYPE_INT) {
		if (var_b->type = M_TYPE_INT) {
			return (abs(var_a->data.i) > abs(var_b->data.i)) - (abs(var_a->data.i) < abs(var_b->data.i));
		}
		else {
			return (abs(var_a->data.i) > fabs(var_b->data.f)) - (abs(var_a->data.i) < fabs(var_b->data.f));
		}
	}
	else {
		if (var_b->type = M_TYPE_INT) {
			return (fabs(var_a->data.f) > abs(var_b->data.i)) - (fabs(var_a->data.f) < abs(var_b->data.i));
		}
		else {
			return (fabs(var_a->data.f) > fabs(var_b->data.f)) - (fabs(var_a->data.f) < fabs(var_b->data.f));
		}
	}
	return 0;
}


int mercury_sort_alphabet_az(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	if (var_a->type != M_TYPE_STRING) {
		return 0;
	}
	if (var_b->type != M_TYPE_STRING) {
		return 0;
	}

	mercury_stringliteral* str_a = (mercury_stringliteral*)var_a->data.p;
	mercury_stringliteral* str_b = (mercury_stringliteral*)var_b->data.p;
	
	mercury_int s_a = str_a->size;
	mercury_int s_b = str_b->size;

	mercury_int size = s_a > s_b ? s_a : s_b;

	const char* string_a = str_a->ptr;
	const char* string_b = str_b->ptr;

	for (mercury_int c = 0; c < size; c++) {
		char a = c > s_a ? -1: string_a[c];
		char b = c > s_b ? -1 : string_b[c];
		if (a > b) {
			return 1;
		} else if (b > a) {
			return -1;
		}
	}
	
	return 0;
}

int mercury_sort_alphabet_za(const void* a, const void* b) {
	mercury_variable* var_a = *(mercury_variable**)a;
	mercury_variable* var_b = *(mercury_variable**)b;

	if (var_a->type != M_TYPE_STRING) {
		return 0;
	}
	if (var_b->type != M_TYPE_STRING) {
		return 0;
	}

	mercury_stringliteral* str_a = (mercury_stringliteral*)var_a->data.p;
	mercury_stringliteral* str_b = (mercury_stringliteral*)var_b->data.p;

	mercury_int s_a = str_a->size;
	mercury_int s_b = str_b->size;

	mercury_int size = s_a > s_b ? s_a : s_b;

	const char* string_a = str_a->ptr;
	const char* string_b = str_b->ptr;

	for (mercury_int c = 0; c < size; c++) {
		char a = c > s_a ? -1 : string_a[c];
		char b = c > s_b ? -1 : string_b[c];
		if (a > b) {
			return -1;
		}
		else if (b > a) {
			return 1;
		}
	}

	return 0;
}




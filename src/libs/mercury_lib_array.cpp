#include "mercury_lib_array.hpp"
#include"../mercury.hpp"
#include"../mercury_error.hpp"
#include "malloc.h"
#include <stdlib.h>
#include <math.h>

#include <stdio.h>

void mercury_lib_array_flush(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //discards nil values.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M,args_in,1)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}

	mercury_variable arr_var;
	mercury_popstack(M,&arr_var);
	if (arr_var.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE,arr_var.type, M_TYPE_ARRAY,1);
		return;
	}


	mercury_array* arr = (mercury_array*)arr_var.data.p;


	mercury_array* newarr=mercury_newarray();
	if (!newarr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	mercury_int len = 0;
	mercury_int len_neg = -1;
	
	//iterate through the array and push each non-nil variable into a new table
	if (arr->values) {
#ifdef MERCURY_64BIT
		//this can't be the best way to do it. i mean... just look at this piece of shit.
		for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1>>1; i1++) { //bitshift right once because we are ignoring negative values, and those start with 1
			mercury_variable***** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
				mercury_variable**** const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
					mercury_variable*** const st3 = st2[i3];
					if (!st3)continue;
					for (int i4 = 0; i4 < MERCURY_SIZE_SUBARRAY_4; i4++) {
						mercury_variable** const st4 = st3[i4];
						if (!st4)continue;
						for (int i5 = 0; i5 < MERCURY_SIZE_SUBARRAY_5; i5++) {
							mercury_variable* const st5 = st4[i5];
							if (!st5)continue;
							for (int i6 = 0; i6 < MERCURY_SIZE_SUBARRAY_6; i6++) {
								mercury_variable var = st5[i6];
								if (var.type) {
									mercury_setarray(newarr, st5+i6, len);
									len++;
									st5[i6].type=M_TYPE_NIL; //set to nil so that the var isn't freed later
								}
							}
						}
					}
				}
			}
		}
		for (int i1 = MERCURY_SIZE_SUBARRAY_1-1; i1 > (MERCURY_SIZE_SUBARRAY_1-1) >> 1; i1--) { //now do the negative values. these count backwards
			mercury_variable***** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = MERCURY_SIZE_SUBARRAY_2; i2 >=0 ; i2--) {
				mercury_variable**** const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = MERCURY_SIZE_SUBARRAY_3; i3 >= 0; i3--) {
					mercury_variable*** const st3 = st2[i3];
					if (!st3)continue;
					for (int i4 = MERCURY_SIZE_SUBARRAY_4; i4 >= 0; i4--) {
						mercury_variable** const st4 = st3[i4];
						if (!st4)continue;
						for (int i5 = MERCURY_SIZE_SUBARRAY_5; i5 >= 0; i5--) {
							mercury_variable* const st5 = st4[i5];
							if (!st5)continue;
							for (int i6 = MERCURY_SIZE_SUBARRAY_6; i6 >= 0; i6--) {
								mercury_variable var = st5[i6];
								if (var.type) {
									mercury_setarray(newarr, st5+i6, len_neg);
									len_neg--;
									st5[i6].type=M_TYPE_NIL; //set to nil so that the var isn't freed later
								}
							}
						}
					}
				}
			}
		}
#else
		//it's less shit here but still not great.
		for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1>>1; i1++) { //bitshift right once because we are ignoring negative values, and those start with 1
			mercury_variable** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
				mercury_variable* const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
					mercury_variable var = st2[i3];
					if (var.type) {
						mercury_setarray(newarr, st2+i3, len);
						len++;
						st2[i3].type = M_TYPE_NIL; //set to nil so that the var isn't freed later
					}
				}
			}
		}
		for (int i1 = MERCURY_SIZE_SUBARRAY_1 - 1; i1 > (MERCURY_SIZE_SUBARRAY_1 - 1) >> 1; i1--) { //now do the negative values. these count backwards
			mercury_variable** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = MERCURY_SIZE_SUBARRAY_2; i2 >= 0; i2--) {
				mercury_variable* const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = MERCURY_SIZE_SUBARRAY_3; i3 >= 0; i3--) {
					mercury_variable var = st2[i3];
					if (var.type) {
						mercury_setarray(newarr, st2 + i3, len);
						len_neg--;
						st2[i3].type = M_TYPE_NIL; //set to nil so that the var isn't freed later
					}
				}
			}
		}
#endif
	}

	mercury_array intermediate = *newarr; //swap the new array data with the old array
	newarr->values = arr->values;
	arr->values = intermediate.values;
	mercury_destroyarray(newarr); //then free the array with the old data in it.

	mercury_free_var(&arr_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void mercury_lib_array_copy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //creates a copy of all values. not recursive. arrays are refrences so this will make a new one.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}
	if (args_out < 1) {
		return;
	}

	mercury_variable arr_var;
	mercury_popstack(M,&arr_var);
	if (arr_var.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, arr_var.type, M_TYPE_ARRAY, 1);
		return;
	}

	mercury_array* arr1 = (mercury_array*)arr_var.data.p;
	mercury_array* arr2 = mercury_newarray();
	if (!arr2) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}



	if (arr1->values) {
#ifdef MERCURY_64BIT
		arr2->values = (mercury_variable******)calloc(MERCURY_SIZE_SUBARRAY_1, sizeof(void*));
		if (!arr2->values) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
			mercury_variable***** const st1_1 = arr1->values[i1];
			if (!st1_1)continue;
			mercury_variable***** st1_2 = (mercury_variable*****)calloc(MERCURY_SIZE_SUBARRAY_2,sizeof(mercury_variable****));
			if (!st1_2) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				return;
			}
			arr2->values[i1] = st1_2;
			for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
				mercury_variable**** const st2_1 = st1_1[i2];
				if (!st2_1)continue;
				mercury_variable**** st2_2 = (mercury_variable****)calloc(MERCURY_SIZE_SUBARRAY_3, sizeof(mercury_variable***));
				if (!st2_2) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				st1_2[i2] = st2_2;
				for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_4; i3++) {
					mercury_variable*** const st3_1 = st2_1[i3];
					if (!st3_1)continue;
					mercury_variable*** st3_2 = (mercury_variable***)calloc(MERCURY_SIZE_SUBARRAY_4, sizeof(mercury_variable**));
					if (!st3_2) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						return;
					}
					st2_2[i3] = st3_2;
					for (int i4 = 0; i4 < MERCURY_SIZE_SUBARRAY_4; i4++) {
						mercury_variable** const st4_1 = st3_1[i4];
						if (!st4_1)continue;
						mercury_variable** st4_2 = (mercury_variable**)calloc(MERCURY_SIZE_SUBARRAY_5, sizeof(mercury_variable*));
						if (!st4_2) {
							mercury_raise_error(M, M_ERROR_ALLOCATION);
							return;
						}
						st3_2[i4] = st4_2;
						for (int i5 = 0; i5 < MERCURY_SIZE_SUBARRAY_5; i5++) {
							mercury_variable* const st5_1 = st4_1[i5];
							if (!st5_1)continue;
							mercury_variable* st5_2 = (mercury_variable*)calloc(MERCURY_SIZE_SUBARRAY_6, sizeof(mercury_variable));
							if (!st5_2) {
								mercury_raise_error(M, M_ERROR_ALLOCATION);
								return;
							}
							st4_2[i5] = st5_2;
							for (int i6 = 0; i6 < MERCURY_SIZE_SUBARRAY_6; i6++) {
								if (st5_1[i6].type)mercury_clonevariable(st5_1+i6, st5_2+i6);
							}
						}
					}
				}
			}
		}
#else
		arr2->values = (mercury_variable***)calloc(MERCURY_SIZE_SUBARRAY_1, sizeof(void*));
		if (!arr2->values) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
			mercury_variable** const st1_1 = arr1->values[i1];
			if (!st1_1)continue;
			mercury_variable** st1_2 = (mercury_variable**)calloc(MERCURY_SIZE_SUBARRAY_2, sizeof(mercury_variable*));
			if (!st1_2) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				return;
			}
			arr2->values[i1] = st1_2;
			for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
				mercury_variable* const st2_1 = st1_1[i2];
				if (!st2_1)continue;
				mercury_variable* st2_2 = (mercury_variable*)calloc(MERCURY_SIZE_SUBARRAY_3, sizeof(mercury_variable));
				if (!st2_2) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				st1_2[i2] = st2_2;
				for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
					if (st2_1[i3].type)mercury_clonevariable(st2_1+i3, st2_2+i3);
				}
			}
		}
#endif
	}


	mercury_free_var(&arr_var);
	arr_var.constant = false;
	arr_var.type = M_TYPE_ARRAY;
	arr_var.data.p = arr2;
	mercury_pushstack_unrefed(M, &arr_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}



void mercury_lib_array_insert(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //add at index, shifts values forward.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2,3)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}

	mercury_variable len_var;
	if (args_in > 2) {
		mercury_popstack(M,&len_var);
		if (len_var.type != M_TYPE_INT) {
			mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, len_var.type, M_TYPE_INT, 3);
			return;
		}
	}
	else {
		len_var.type = M_TYPE_NIL;
	}
	mercury_variable var_to_ins;
	mercury_popstack(M,&var_to_ins);

	mercury_variable arr_var;
	mercury_popstack(M, &arr_var);
	if (arr_var.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, arr_var.type, M_TYPE_ARRAY, 1);
		return;
	}

	mercury_array* arr = (mercury_array*)arr_var.data.p;
	mercury_int cur_len=mercury_array_len(arr);

	if (len_var.type) { //if len, shift values ahead to make space.
		mercury_int target = len_var.data.i;
		for (mercury_int i = cur_len; i >= target; i--) {
			mercury_variable v;
			mercury_getarray(arr, i, &v);
			mercury_setarray(arr, &v, i + 1);
		}
		mercury_setarray(arr, &var_to_ins, target);
	}
	else { //if len is not specified, add to the end.
		mercury_setarray(arr, &var_to_ins, cur_len);
	}

	mercury_free_var(&arr_var);
	mercury_free_var(&var_to_ins);
	if(len_var.type)mercury_free_var(&len_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}

void mercury_lib_array_remove(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //gets rid of a value and shifts ones after down.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}

	mercury_variable len_var;
	mercury_popstack(M, &len_var);
	if (len_var.type != M_TYPE_INT) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, len_var.type, M_TYPE_INT, 2);
		return;
	}

	mercury_variable arr_var;
	mercury_popstack(M, &arr_var);
	if (arr_var.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, arr_var.type, M_TYPE_ARRAY, 1);
		return;
	}

	mercury_int target = len_var.data.i;
	mercury_array* arr = (mercury_array*)arr_var.data.p;
	mercury_int cur_len = mercury_array_len(arr);


	mercury_variable v;
	mercury_getarray(arr, target, &v);
	mercury_free_var(&v );
	for (mercury_int i = target; i <= cur_len; i++) {
		mercury_getarray(arr, i + 1, &v);
		mercury_setarray(arr, &v, i);
	}

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void mercury_lib_array_swap(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //duh.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 3)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}

	mercury_variable pos_var1;
	mercury_popstack(M, &pos_var1);
	if (pos_var1.type != M_TYPE_INT) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, pos_var1.type, M_TYPE_INT, 3);
		return;
	}

	mercury_variable pos_var2;
	mercury_popstack(M, &pos_var2);
	if (pos_var2.type != M_TYPE_INT) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, pos_var2.type, M_TYPE_INT, 2);
		return;
	}

	mercury_variable arr_var;
	mercury_popstack(M, &arr_var);
	if (arr_var.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, arr_var.type, M_TYPE_ARRAY, 1);
		return;
	}

	mercury_array* ar = (mercury_array*)arr_var.data.p;
	mercury_int p1 = pos_var1.data.i;
	mercury_int p2 = pos_var2.data.i;

	mercury_variable tv;
	mercury_getarray(ar, p1,&tv);
	mercury_variable tv2;
	mercury_getarray(ar, p2, &tv2);
	mercury_setarray(ar, &tv2, p1);
	mercury_setarray(ar, &tv,p2);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
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
	mercury_variable var_o;
	mercury_popstack(M,&var_o);
	mercury_destroystate(M);
	if (var_o.type == M_TYPE_INT) {
		mercury_free_var(&var_o);
		return (int)var_o.data.i;
	}
	mercury_free_var(&var_o);
	return 0;
}


void mercury_lib_array_sort(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2);

	mercury_variable var_func;
	mercury_popstack(M,&var_func);
	if (var_func.type != M_TYPE_FUNCTION && var_func.type != M_TYPE_CFUNC) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, var_func.type, M_TYPE_FUNCTION, 2);
		return;
	}

	mercury_variable var_array;
	mercury_popstack(M,&var_array);
	if (var_array.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, var_array.type, M_TYPE_ARRAY, 1);
		return;
	}

	mercury_array* arr = (mercury_array*)var_array.data.p;
	mercury_int arr_size=mercury_array_len(arr)+1;
	mercury_variable* tlist=(mercury_variable*)malloc(sizeof(mercury_variable)*arr_size);
	if (!tlist) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (mercury_int i = 0; i < arr_size; i++) { //this could probably be optimized with a memcpy. oh well.
		mercury_getarray(arr, i, tlist+i);
	}

	if (var_func.type == M_TYPE_CFUNC) {
		qsort(tlist, arr_size, sizeof(mercury_variable), (int (*)(const void*, const void*))(var_func.data.p) ); //be careful with C, dummy.
	}
	else {
		SORTING_M_FUNCTION = (mercury_function*)var_func.data.p;
		SORTING_M_STATE = M;
		qsort(tlist, arr_size, sizeof(mercury_variable), mercury_sort_use_mercury_function); //surely this will work.
	}

	for (mercury_int i = 0; i < arr_size; i++) { //see above.
		mercury_setarray(arr, tlist+i,i);
	}
	free(tlist);
	
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
}




int mercury_sort_greater_to_lesser(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;
	if (var_a.type != M_TYPE_INT && var_a.type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b.type != M_TYPE_INT && var_b.type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a.type == M_TYPE_INT) {
		if (var_b.type == M_TYPE_INT) {
			return (var_a.data.i < var_b.data.i) - (var_a.data.i > var_b.data.i);
		}
		else {
			return (var_a.data.i < var_b.data.f) - (var_a.data.i > var_b.data.f);
		}
	}
	else {
		if (var_b.type == M_TYPE_INT) {
			return (var_a.data.f < var_b.data.i) - (var_a.data.f > var_b.data.i);
		}
		else {
			return (var_a.data.f < var_b.data.f) - (var_a.data.f > var_b.data.f);
		}
	}
	return 0;
}

int mercury_sort_lesser_to_greater(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;

	if (var_a.type != M_TYPE_INT && var_a.type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b.type != M_TYPE_INT && var_b.type != M_TYPE_FLOAT) {
		return 0;
	}
	
	if (var_a.type == M_TYPE_INT) {
		if (var_b.type == M_TYPE_INT) {
			return (var_a.data.i > var_b.data.i) - (var_a.data.i < var_b.data.i);
		}
		else {
			return (var_a.data.i > var_b.data.f) - (var_a.data.i < var_b.data.f);
		}
	}
	else {
		if (var_b.type == M_TYPE_INT) {
			return (var_a.data.f > var_b.data.i) - (var_a.data.f < var_b.data.i);
		}
		else {
			return (var_a.data.f > var_b.data.f) - (var_a.data.f < var_b.data.f);
		}
	}
	return 0;
}

int mercury_sort_greater_to_lesser_absolute(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;

	if (var_a.type != M_TYPE_INT && var_a.type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b.type != M_TYPE_INT && var_b.type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a.type == M_TYPE_INT) {
		if (var_b.type == M_TYPE_INT) {
			return (abs(var_a.data.i) < abs(var_b.data.i)) - (abs(var_a.data.i) > abs(var_b.data.i));
		}
		else {
			return (abs(var_a.data.i) < fabs(var_b.data.f) ) - (abs(var_a.data.i) > fabs(var_b.data.f));
		}
	}
	else {
		if (var_b.type == M_TYPE_INT) {
			return (fabs(var_a.data.f) < abs(var_b.data.i)) - (fabs(var_a.data.f) > abs(var_b.data.i));
		}
		else {
			return (fabs(var_a.data.f) < fabs(var_b.data.f)) - (fabs(var_a.data.f) > fabs(var_b.data.f));
		}
	}
	return 0;
}




int mercury_sort_lesser_to_greater_absolute(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;

	if (var_a.type != M_TYPE_INT && var_a.type != M_TYPE_FLOAT) {
		return 0;
	}
	if (var_b.type != M_TYPE_INT && var_b.type != M_TYPE_FLOAT) {
		return 0;
	}

	if (var_a.type == M_TYPE_INT) {
		if (var_b.type == M_TYPE_INT) {
			return (abs(var_a.data.i) > abs(var_b.data.i)) - (abs(var_a.data.i) < abs(var_b.data.i));
		}
		else {
			return (abs(var_a.data.i) > fabs(var_b.data.f)) - (abs(var_a.data.i) < fabs(var_b.data.f));
		}
	}
	else {
		if (var_b.type == M_TYPE_INT) {
			return (fabs(var_a.data.f) > abs(var_b.data.i)) - (fabs(var_a.data.f) < abs(var_b.data.i));
		}
		else {
			return (fabs(var_a.data.f) > fabs(var_b.data.f)) - (fabs(var_a.data.f) < fabs(var_b.data.f));
		}
	}
	return 0;
}


int mercury_sort_alphabet_az(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;

	if (var_a.type != M_TYPE_STRING) {
		return 0;
	}
	if (var_b.type != M_TYPE_STRING) {
		return 0;
	}

	mercury_string* str_a = (mercury_string*)var_a.data.p;
	mercury_string* str_b = (mercury_string*)var_b.data.p;
	
	mercury_int s_a = str_a->size;
	mercury_int s_b = str_b->size;

	mercury_int size = s_a > s_b ? s_a : s_b;

	const char* string_a = str_a->ptr;
	const char* string_b = str_b->ptr;

	for (mercury_int c = 0; c < size; c++) {
		short a = c > s_a ? -1 : string_a[c];
		short b = c > s_b ? -1 : string_b[c];
		if (a > b) {
			return 1;
		} else if (b > a) {
			return -1;
		}
	}
	
	return 0;
}

int mercury_sort_alphabet_za(const void* a, const void* b) {
	mercury_variable var_a = *(mercury_variable*)a;
	mercury_variable var_b = *(mercury_variable*)b;

	if (var_a.type != M_TYPE_STRING) {
		return 0;
	}
	if (var_b.type != M_TYPE_STRING) {
		return 0;
	}

	mercury_string* str_a = (mercury_string*)var_a.data.p;
	mercury_string* str_b = (mercury_string*)var_b.data.p;

	mercury_int s_a = str_a->size;
	mercury_int s_b = str_b->size;

	mercury_int size = s_a > s_b ? s_a : s_b;

	const char* string_a = str_a->ptr;
	const char* string_b = str_b->ptr;

	for (mercury_int c = 0; c < size; c++) {
		short a = c > s_a ? -1 : string_a[c];
		short b = c > s_b ? -1 : string_b[c];
		if (a > b) {
			return -1;
		}
		else if (b > a) {
			return 1;
		}
	}

	return 0;
}



void mercury_lib_array_concat(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}
	mercury_variable var_str;
	mercury_popstack(M,&var_str);
	if (var_str.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, var_str.type, M_TYPE_STRING, 2);
		return;
	}

	mercury_variable var_array;
	mercury_popstack(M,&var_array);
	if (var_array.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, var_array.type, M_TYPE_ARRAY, 1);
		return;
	}
	if (!args_out)return;

	mercury_string* out_str=mercury_cstring_to_mstring((char*)"",0);

	mercury_int limit=mercury_array_len((mercury_array*)var_array.data.p)+1;
	for (mercury_int i = 0; i < limit; i++) {
		mercury_variable var;
		mercury_getarray((mercury_array*)var_array.data.p, i, &var);
		if (var.type) {
			mercury_mstrings_append(out_str, mercury_tostring(&var));
		}
		

		if(i!=limit-1)mercury_mstrings_append(out_str, (mercury_string*)var_str.data.p);
	}


	mercury_free_var(&var_array);
	mercury_free_var(&var_str);
	var_str.constant = false;
	var_str.type = M_TYPE_STRING;
	var_str.data.p = out_str;

	mercury_pushstack(M, &var_str);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}
#pragma once
#include"../mercury.h"

#define MERCURY_LIB_ARRAY


void mercury_lib_array_flush(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_array_copy(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_array_insert(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_array_remove(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_array_swap(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_array_sort(mercury_state* M, mercury_int args_in, mercury_int args_out);

int mercury_sort_greater_to_lesser(const void* a, const void* b);
int mercury_sort_lesser_to_greater(const void* a, const void* b);
int mercury_sort_greater_to_lesser_absolute(const void* a, const void* b);
int mercury_sort_lesser_to_greater_absolute(const void* a, const void* b);
int mercury_sort_alphabet_az(const void* a, const void* b);
int mercury_sort_alphabet_za(const void* a, const void* b);

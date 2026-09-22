#pragma once
#include"../mercury.hpp"

#define MERCURY_LIB_ARRAY


void mercury_lib_array_flush(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_copy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_insert(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_remove(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_swap(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_sort(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_array_concat(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

const mercury_int m_sort_greater_to_lesser = 1;
const mercury_int m_sort_lesser_to_greater = 2;
const mercury_int m_sort_greater_to_lesser_absolute = 3;
const mercury_int m_sort_lesser_to_greater_absolute = 4;
const mercury_int m_sort_alphabet_az = 5;
const mercury_int m_sort_alphabet_za = 6;
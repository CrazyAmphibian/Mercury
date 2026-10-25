#pragma once
#include"../orcus.hpp"

#define ORCUS_LIB_ARRAY


void orcus_lib_array_flush(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_copy(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_insert(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_remove(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_swap(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_sort(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_array_concat(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

const orcus_int m_sort_greater_to_lesser = 1;
const orcus_int m_sort_lesser_to_greater = 2;
const orcus_int m_sort_greater_to_lesser_absolute = 3;
const orcus_int m_sort_lesser_to_greater_absolute = 4;
const orcus_int m_sort_alphabet_az = 5;
const orcus_int m_sort_alphabet_za = 6;
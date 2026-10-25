#pragma once
#include"../orcus.hpp"

#define ORCUS_LIB_TABLE

void orcus_lib_table_copy(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_flush(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_getkeys(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_getvalues(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_haskey(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_hasvalue(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_table_keyswithvalue(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

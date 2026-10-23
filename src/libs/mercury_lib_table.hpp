#pragma once
#include"../mercury.hpp"

#define MERCURY_LIB_TABLE

void mercury_lib_table_copy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_flush(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_getkeys(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_getvalues(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_haskey(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_hasvalue(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_table_keyswithvalue(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

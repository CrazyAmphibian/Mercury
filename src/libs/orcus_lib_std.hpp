#pragma once
/*
the standard library.
contains things for basic debugging and some misc. utility functions.

*/
#include "../orcus.hpp"


#define ORCUS_LIB_STD

void orcus_lib_std_print(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_iterate(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_restricted_call(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_dump(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_compile(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_type(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_tostring(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_tonumber(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_dynamic_library_load(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_toint(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_tofloat(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_deepcopy(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_error(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_addlibs(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_std_sleep(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

const orcus_int orcus_const_type_nil = ORCUS_TYPE_NIL;
const orcus_int orcus_const_type_int = ORCUS_TYPE_INT;
const orcus_int orcus_const_type_float = ORCUS_TYPE_FLOAT;
const orcus_int orcus_const_type_bool = ORCUS_TYPE_BOOL;
const orcus_int orcus_const_type_table = ORCUS_TYPE_TABLE;
const orcus_int orcus_const_type_string = ORCUS_TYPE_STRING;
const orcus_int orcus_const_type_cfunc = ORCUS_TYPE_CFUNC;
const orcus_int orcus_const_type_array = ORCUS_TYPE_ARRAY;
const orcus_int orcus_const_type_function = ORCUS_TYPE_FUNCTION;
const orcus_int orcus_const_type_file = ORCUS_TYPE_FILE;
const orcus_int orcus_const_type_thread = ORCUS_TYPE_THREAD;

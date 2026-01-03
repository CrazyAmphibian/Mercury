#pragma once
/*
the standard library.
contains things for basic debugging and some misc. utility functions.

*/
#include "../mercury.hpp"


#define MERCURY_LIB_STD

void mercury_lib_std_print(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_iterate(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_restricted_call(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_dump(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_compile(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_type(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_tostring(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_tonumber(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_dynamic_library_load(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_toint(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_std_tofloat(mercury_state* M, mercury_int args_in, mercury_int args_out);

const mercury_int m_const_type_nil = M_TYPE_NIL;
const mercury_int m_const_type_int = M_TYPE_INT;
const mercury_int m_const_type_float = M_TYPE_FLOAT;
const mercury_int m_const_type_bool = M_TYPE_BOOL;
const mercury_int m_const_type_table = M_TYPE_TABLE;
const mercury_int m_const_type_string = M_TYPE_STRING;
const mercury_int m_const_type_cfunc = M_TYPE_CFUNC;
const mercury_int m_const_type_array = M_TYPE_ARRAY;
const mercury_int m_const_type_function = M_TYPE_FUNCTION;
const mercury_int m_const_type_file = M_TYPE_FILE;
const mercury_int m_const_type_thread = M_TYPE_THREAD;

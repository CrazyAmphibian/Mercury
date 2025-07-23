#pragma once
/*
 string library.
 contains functions for string manipulation.
*/
#include "../mercury.h"

#define MERCURY_LIB_STRING



void mercury_lib_string_sub(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_reverse(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_find(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_replace(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_count(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_toarray(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_fromarray(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_separate(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_upper(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_lower(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_format(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_p_find(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_p_extract(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_p_replace(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_string_p_count(mercury_state* M, mercury_int args_in, mercury_int args_out);

#pragma once
/*
 string library.
 contains functions for string manipulation.
*/
#include "../mercury.hpp"

#define MERCURY_LIB_STRING



void mercury_lib_string_sub(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_reverse(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_find(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_replace(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_count(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_toarray(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_fromarray(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_separate(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_upper(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_lower(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_format(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_p_find(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_p_extract(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_p_replace(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_p_count(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_escape_mercury(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_escape_url(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_escape_c(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_escape_html(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_string_copy_string(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

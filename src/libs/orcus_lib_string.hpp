#pragma once
/*
 string library.
 contains functions for string manipulation.
*/
#include "../orcus.hpp"

#define ORCUS_LIB_STRING



void orcus_lib_string_sub(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_reverse(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_find(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_replace(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_count(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_toarray(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_fromarray(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_separate(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_upper(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_lower(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_format(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_p_find(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_p_extract(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_p_replace(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_p_count(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_escape_orcus(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_escape_url(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_escape_c(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_escape_html(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_copy_string(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_match_function(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_p_match_function(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_repeat_chars(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_string_set_chars(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);


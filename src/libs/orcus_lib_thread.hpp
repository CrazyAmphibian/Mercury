#pragma once
/*
 thread library.
functions to multithread, safety not gaurenteed.
*/
#include "../orcus.hpp"

#define ORCUS_LIB_THREAD



void orcus_lib_thread_new(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_checkfinish(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_getvalue(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_abort(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_getnumvalues(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_waitfor(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_checkrunning(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_break(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_thread_check_error(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);









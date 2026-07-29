#pragma once
/*
 thread library.
functions to multithread, safety not gaurenteed.
*/
#include "../mercury.hpp"

#define MERCURY_LIB_THREAD



void mercury_lib_thread_new(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_checkfinish(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_getvalue(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_abort(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_getnumvalues(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_waitfor(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_checkrunning(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_break(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_thread_check_error(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);









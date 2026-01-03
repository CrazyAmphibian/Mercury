#pragma once
/*
 thread library.
functions to multithread, safety not gaurenteed.
*/
#include "../mercury.hpp"

#define MERCURY_LIB_THREAD



void mercury_lib_thread_new(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_checkfinish(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_getvalue(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_abort(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_getnumvalues(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_waitfor(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_thread_checkrunning(mercury_state* M, mercury_int args_in, mercury_int args_out);









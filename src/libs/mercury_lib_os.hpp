#pragma once

#define MERCURY_LIB_OS
#include "../mercury.hpp"

void mercury_lib_os_time(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_os_execute(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_os_call(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_os_clock(mercury_state* M, mercury_int args_in, mercury_int args_out);

#if defined(_WIN32) || defined(_WIN64)
const mercury_int m_os_isposix=0;
#else
const mercury_int m_os_isposix = 1;
#endif

#ifdef MERCURY_64BIT
const mercury_int m_os_is64bit = 1;
#else
const mercury_int m_os_is64bit = 0;
#endif


//void mercury_lib_os_isposix(mercury_state* M, mercury_int args_in, mercury_int args_out);

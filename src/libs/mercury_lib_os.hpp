#pragma once

#define MERCURY_LIB_OS
#include "../mercury.hpp"

void mercury_lib_os_time(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_os_execute(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_os_call(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_os_clock(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_os_getdate(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_os_gettime(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

#ifdef _WIN32
const mercury_int m_os_isposix=0;
#else
const mercury_int m_os_isposix = 1;
#endif

#ifdef MERCURY_64BIT
const mercury_int m_os_is64bit = 1;
#else
const mercury_int m_os_is64bit = 0;
#endif


//void mercury_lib_os_isposix(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

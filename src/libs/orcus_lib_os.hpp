#pragma once

#define ORCUS_LIB_OS
#include "../orcus.hpp"

void orcus_lib_os_time(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_execute(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_call(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_clock(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_getdate(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_gettime(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_os_exit(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

#ifdef _WIN32
const orcus_int orcus_os_isposix=0;
#else
const orcus_int orcus_os_isposix = 1;
#endif

#ifdef ORCUS_64BIT
const orcus_int orcus_os_is64bit = 1;
#else
const orcus_int orcus_os_is64bit = 0;
#endif


//void orcus_lib_os_isposix(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

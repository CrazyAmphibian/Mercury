/*
debug library. it helps debug things. not that useful for a normal user.
*/
#include "../mercury.hpp"


#define MERCURY_LIB_DEBUG

void mercury_lib_debug_stack_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_state_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_enviroment_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_constants_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_bytecode_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_bytecode_rawbinary_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_refcount_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
void mercury_lib_debug_dump_debug_info_dbg(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);
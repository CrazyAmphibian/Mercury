/*
debug library. it helps debug things. not that useful for a normal user.
*/
#include "../orcus.hpp"


#define ORCUS_LIB_DEBUG

void orcus_lib_debug_stack_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_state_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_enviroment_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_constants_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_bytecode_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_bytecode_rawbinary_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_refcount_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_dump_debug_info_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);
void orcus_lib_debug_get_traceback(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out);

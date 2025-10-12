/*
debug library. it helps debug things. not that useful for a normal user.
*/
#include "../mercury.h"


#define MERCURY_LIB_DEBUG

void mercury_lib_debug_stack_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_debug_state_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_debug_enviroment_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_debug_constants_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_debug_bytecode_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out);

#pragma once
#include "stdint.h"
#include "mercury.hpp"
#include "mercury_compiler.hpp"


enum M_ERROR_TYPES:uint32_t {
	M_ERROR_NONE = 0,			// args: 
	M_ERROR_ALLOCATION = 1,		//args:
	M_ERROR_WRONG_TYPE = 2,		//args: expected, provided, arg
	M_ERROR_DIV_ZERO = 3,		//args:
	M_ERROR_INVALID_INDEX = 4, // args: expected, provided
	M_ERROR_INSTRUCTION_FAILIURE = 5, //args:
	M_ERROR_CALL_NOT_FUNCTION = 6 ,// args: provided
	M_ERROR_INDEX_INVALID_TYPE = 7, // args: provided
	M_ERROR_NOT_ENOUGH_ARGS = 8, //args: expected, provided
	M_ERROR_CUSTOM_STRING = 9 //args: string
};

enum M_COMPILER_ERRORS :uint32_t {
	M_COMPERR_NONE=0,
	M_COMPERR_DID_NOT_CALL_OR_SET=1,
	M_COMPERR_INVALID_SYMBOL=2,
	M_COMPERR_ENDS_TOO_SOON=3,
	M_COMPERR_EXPECTED_VARIABLE=4,
	M_COMPERR_IF_NEEDS_THEN=5,
	M_COMPERR_MEMORY_ALLOCATION=6,
	M_COMPERR_WHILE_NEEDS_DO=7,
	M_COMPERR_UNKNOWN=8,
	M_COMPERR_ELSEIF_NEEDS_THEN = 9,
	M_COMPERR_KEYWORD_REQUIRES_LOOP = 10,
	M_COMPERR_PAREN_NOT_CLOSED=11,
};

mercury_stringliteral* mercury_generate_error_string(mercury_state* M, uint32_t errorcode, void* data1 = nullptr, void* data2 = nullptr, void* data3 = nullptr);
void mercury_raise_error(mercury_state* M, uint32_t errorcode, void* data1 = nullptr, void* data2 = nullptr, void* data3 = nullptr);

mercury_stringliteral* mercury_generate_compiler_error_string(compiler_token** tokens, uint32_t errorcode, mercury_int token_err, mercury_int token_max);


//helper functions for making c functions. returns true if there's a problem, false if nothing went wrong. if()return;

//variadic functions need not apply. also if you want to take optional args, you should specify max_args as the largest number you want to take. this function ensures that additional args will be discarded, and if args are missing, an error will be thrown.
inline bool MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(mercury_state* M, mercury_int args_in, mercury_int min_args, mercury_int max_args=0) {
	if (!max_args) {
		max_args = min_args; //by default, args are specific
	}
	if (args_in < min_args) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)min_args);
		return true;
	}
	for (mercury_int i = max_args; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	return false;
}

//pretty simple will output any args that haven't been outputted. is nil.
inline bool MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(mercury_state* M, mercury_int args_out, mercury_int sent_args=0) {
	for (mercury_int a = sent_args; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		if (mv) {
			mv->type = M_TYPE_NIL;
			mv->data.i = 0;
			mv->constant = false;
			mercury_pushstack(M, mv);
		}
		else {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return true;
		}
	}
	return false;
}

#pragma once
#include <stdint.h>
#include <malloc.h>
#include "mercury.hpp"
#include "mercury_compiler.hpp"


enum M_ERROR_TYPES:uint32_t {
	M_ERROR_NONE = 0,			// args: 
	M_ERROR_ALLOCATION = 1,		//args:
	M_ERROR_WRONG_TYPE = 2,		//args: provided, expected, arg number
	M_ERROR_DIV_ZERO = 3,		//args:
	M_ERROR_INVALID_INDEX = 4, // args: expected, provided
	M_ERROR_INSTRUCTION_FAILIURE = 5, //args:
	M_ERROR_CALL_NOT_FUNCTION = 6 ,// args: provided
	M_ERROR_INDEX_INVALID_TYPE = 7, // args: provided
	M_ERROR_NOT_ENOUGH_ARGS = 8, //args: provided, expected
	M_ERROR_CUSTOM_STRING = 9, //args: string
	M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER, //args: provided, arg number. for functions that can take ints or floats
	M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION, //args: provided, arg number. for functions that can take C and M functions
	M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE,//args: provided, arg number. for functions that can take arrays or tables.
};

mercury_stringliteral* mercury_generate_error_string(mercury_state* M, const uint32_t errorcode, const mercury_int* data1 = nullptr, const mercury_int* data2 = nullptr, const mercury_int* data3 = nullptr);
void mercury_raise_error(mercury_state* M, const uint32_t errorcode, const mercury_int* data1 = nullptr, const mercury_int* data2 = nullptr, const mercury_int* data3 = nullptr);


inline void mercury_raise_error_nonpointer(mercury_state* M, const uint32_t errorcode, const mercury_int data1 = 0, const mercury_int data2 = 0, const mercury_int data3 = 0) {
	mercury_raise_error(M, errorcode, &data1, &data2, &data3);
}


//helper functions for making c functions. returns true if there's a problem, false if nothing went wrong. if()return;

//variadic functions need not apply. also if you want to take optional args, you should specify max_args as the largest number you want to take. this function ensures that additional args will be discarded, and if args are missing, an error will be thrown.
inline bool MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int min_args, mercury_int max_args=0) {
	if (!max_args) {
		max_args = min_args; //by default, args are specific
	}
	if (args_in < min_args) {
		mercury_raise_error_nonpointer(M, M_ERROR_NOT_ENOUGH_ARGS, args_in, min_args);
		return true;
	}
	for (mercury_int i = (const mercury_int)max_args; i < args_in; i++) {
		mercury_discard_top_of_stack(M);
	}
	return false;
}

//pretty simple will output any args that haven't been outputted. is nil.
inline bool MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(mercury_state* const M_CPP_restrict M, const mercury_int args_out, const mercury_int sent_args=0) {
	mercury_int diff = args_out - sent_args;
	if (diff > 0) {
		if (M->allocatedstacksize < M->sizeofstack + diff) {
			mercury_variable* nptr = (mercury_variable*)realloc(M->stack, sizeof(mercury_variable) * (M->sizeofstack + diff));
			if (!nptr) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				return false;
			}
			M->stack = nptr;
			M->allocatedstacksize = M->sizeofstack + diff;
		}
		memset(M->stack + M->sizeofstack, '\0', diff*sizeof(mercury_variable));
		M->sizeofstack += diff;
		
		return true;
	}
	return true;
}

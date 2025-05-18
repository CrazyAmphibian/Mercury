#pragma once
#include "mercury.h"


struct compiler_token {
	mercury_int line_num = 0;
	mercury_int line_col = 0;
	char* chars = nullptr;
	int num_chars = 0;
	int token_flags = 0;
};

struct compiler_function {
	uint32_t* instructions = nullptr;
	mercury_int* instruction_tokens = nullptr; //which token each instruction points to
	mercury_int number_instructions = 0;
	mercury_int token_error_num = 0;
	int errorcode = 0;
};


MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_compile_mstring(mercury_stringliteral* str);
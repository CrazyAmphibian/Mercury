#include "mercury_error.hpp"
#include "mercury_compiler.hpp"
#include "mercury.hpp"

#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include <stdarg.h>



const char* typetostring[256] = {
	"nil",
	"integer",
	"float",
	"boolean",
	"table",
	"string",
	"c function",
	"array",
	"function",
	"file",
	"thread"
};


mercury_stringliteral* mercury_generate_error_string(mercury_state* M, uint32_t errorcode, void* data1, void* data2, void* data3) {
	char* buffer = (char*)calloc(255,sizeof(char));
	if (!buffer)return nullptr;
	//char buffer[255] = {0};
	int result = 0;

	char header[255] = {0};
	if (M->bytecode.debug_info) {

		const char* fallback = "";

		mercury_debug_token T = M->bytecode.debug_info[M->programcounter-1];
		char* tchars=(char*)malloc(T.num_chars+1);
		if (tchars) {
			memcpy(tchars, T.chars, T.num_chars);
			tchars[T.num_chars] = '\0';
		}
#if defined(DEBUG) || defined(_DEBUG)
		snprintf(header, 255, "line %zi col %zi instruction #%zi (%04X) at \"%s\"", T.line + 1, T.col + 1, M->programcounter-1, M->bytecode.instructions[M->programcounter-1].opcode, tchars ? tchars : fallback);
#else
		snprintf(header, 255, "line %zi col %zi at \"%s\"", T.line + 1, T.col + 1, tchars ? tchars : fallback);
#endif
	}
	else {
		snprintf(header, 255, "line ? col ? at ? - instruction #%zi (%04X)",M->programcounter-1,M->bytecode.instructions[M->programcounter-1].opcode);
	}

	switch (errorcode) {
	case M_ERROR_NONE:
		return nullptr;
	case M_ERROR_ALLOCATION:
		result=snprintf(buffer, 255, "%s: memory allocation error\n",header);
		return mercury_cstring_const_to_mstring(buffer,strlen(buffer));
	case M_ERROR_WRONG_TYPE:
		result = snprintf(buffer, 255, "%s: arg %i wrong type. expected %s, got %s\n", header ,(int)data3 , typetostring[(int)data2] , typetostring[(int)data1]);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_DIV_ZERO:
		result = snprintf(buffer, 255, "%s: integer division by 0\n", header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INSTRUCTION_FAILIURE:
		result = snprintf(buffer, 255, "%s: failiure to execute instruction\n", header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_CALL_NOT_FUNCTION:
		result = snprintf(buffer, 255, "%s: attempt to call non-function value %s \n", header , typetostring[(int)data1] );
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INDEX_INVALID_TYPE:
		result = snprintf(buffer, 255, "%s: attempt to index invalid variable type %s \n", header, typetostring[(int)data1]);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_NOT_ENOUGH_ARGS:
		result = snprintf(buffer, 255, "%s: incorrect number of args. expected %i, got %i \n", header, (int)data2, (int)data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_CUSTOM_STRING:
		result = snprintf(buffer, 255, "%s: %s \n", header, (char*)data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	default:
		result = snprintf(buffer, 255, "%s: unknown error\n",header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	}
}

void mercury_raise_error(mercury_state* M, uint32_t errorcode, void* data1, void* data2, void* data3) {

	mercury_stringliteral* str = mercury_generate_error_string(M,errorcode, data1, data2, data3);


	if (str != nullptr) {
		for (mercury_int i = 0; i < str->size; i++) {
			//printf("%c",str->ptr[i]);
			putchar(str->ptr[i]);
		}
	}

	putchar('\n');
	free(str->ptr);
	free(str);

	while (M) {
		M->programcounter = M->bytecode.numberofinstructions; //push to end to stop execution
		M = M->parentstate;
	}
}
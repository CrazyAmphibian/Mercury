#include "mercury_error.hpp"
#include "mercury_compiler.hpp"
#include "mercury.hpp"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
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

inline const char* get_type_string(uint8_t type) {
	if (type >= M_NUMBER_OF_TYPES) {
		return "unknown";
	}
	return typetostring[type];
}

mercury_string* mercury_generate_error_string(mercury_state* M, const uint32_t errorcode, const mercury_int* data1, const mercury_int* data2, const mercury_int* data3) {
	const unsigned int buffer_size=1000;
	char* buffer = (char*)calloc(buffer_size,sizeof(char));
	if (!buffer)return nullptr;
	//char buffer[255] = {0};
	int result = 0;

	char header[buffer_size] = {0};
	unsigned int used_space = 0;
start:

	if (M->bytecode.debug_info) {
		
		const char* fallback = "";
		mercury_debug_token T = M->bytecode.debug_info[M->programcounter-1];
		char* tchars=(char*)malloc(T.num_chars+1);
		if (tchars) {
			memcpy(tchars, T.chars, T.num_chars);
			tchars[T.num_chars] = '\0';
		}
#if defined(DEBUG) || defined(_DEBUG)
		used_space+=snprintf(header + used_space, buffer_size- used_space, "line %zi col %zi instruction #%zi (%04X) at \"%s\"", T.line + 1, T.col + 1, M->programcounter-1, M->bytecode.instructions[M->programcounter-1], tchars ? tchars : fallback);
#else
		used_space += snprintf(header + used_space, buffer_size - used_space, "line %zi col %zi at \"%s\"", T.line + 1, T.col + 1, tchars ? tchars : fallback);
#endif
	}
	else {
		used_space += snprintf(header+ used_space, buffer_size - used_space, "line ? col ? at ? - instruction #%zi (%04X)",M->programcounter-1,M->bytecode.instructions[M->programcounter-1]);
	}

	M = M->parentstate;
	if (M) {
		if (used_space < buffer_size + 10) {
			header[used_space++] = ' ';
			header[used_space++] = 'i';
			header[used_space++] = 'n';
			header[used_space++] = '\n';
		}
		goto start;
	}

	switch (errorcode) {
	case M_ERROR_NONE:
		return nullptr;
	case M_ERROR_ALLOCATION:
		result=snprintf(buffer, buffer_size, "%s: memory allocation error\n",header);
		return mercury_cstring_const_to_mstring(buffer,strlen(buffer));
	case M_ERROR_WRONG_TYPE:
		result = snprintf(buffer, buffer_size, "%s: arg %zi wrong type. expected %s, got %s\n", header ,*data3 , get_type_string((uint8_t)*data2) , get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_DIV_ZERO:
		result = snprintf(buffer, buffer_size, "%s: integer division by 0\n", header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INSTRUCTION_FAILIURE:
		result = snprintf(buffer, buffer_size, "%s: failiure to execute instruction\n", header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_CALL_NOT_FUNCTION:
		result = snprintf(buffer, buffer_size, "%s: attempt to call non-function value %s \n", header , get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INDEX_INVALID_TYPE:
		result = snprintf(buffer, buffer_size, "%s: attempt to index invalid variable type %s \n", header, get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_NOT_ENOUGH_ARGS:
		result = snprintf(buffer, buffer_size, "%s: incorrect number of args. expected %zi, got %zi \n", header, *data2, *data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_CUSTOM_STRING:
		result = snprintf(buffer, buffer_size, "%s: %s \n", header, (char*)data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER:
		result = snprintf(buffer, buffer_size, "%s: arg %zi wrong type. expected a number, got %s\n", header, *data2, get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION:
		result = snprintf(buffer, buffer_size, "%s: arg %zi wrong type. expected any function type, got %s\n", header, *data2, get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE:
		result = snprintf(buffer, buffer_size, "%s: arg %zi wrong type. expected an array or table, got %s\n", header, *data2, get_type_string((uint8_t)*data1));
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	default:
		result = snprintf(buffer, buffer_size, "%s: unknown error\n",header);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	}
}

void mercury_raise_error(mercury_state* M, const uint32_t errorcode, const mercury_int* data1, const mercury_int* data2, const mercury_int* data3) {

	mercury_string* str = mercury_generate_error_string(M,errorcode, data1, data2, data3);


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
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
	mercury_string* out = (mercury_string*)malloc(sizeof(mercury_string));
	if (!out)return out;
	memset(out, NULL, sizeof(mercury_string));
	const size_t buffer_size = 1000;
	char buffer[buffer_size];
	memset(buffer, '\0', buffer_size);

	while (M) {
		mercury_string* prepend = (mercury_string*)malloc(sizeof(mercury_string));
		if (!prepend) {
			mercury_mstring_delete(out);
			return nullptr;
		}
		memset(prepend, NULL, sizeof(mercury_string));

		if (M->bytecode.debug_info) {
			mercury_debug_token T = M->bytecode.debug_info[M->programcounter - 1];
			char* tchars = (char*)malloc(T.num_chars + 1);
			if (!tchars) {
				mercury_mstring_delete(out);
				mercury_mstring_delete(prepend);
				return nullptr;
			}
			memcpy(tchars, T.chars, T.num_chars);
			tchars[T.num_chars] = '\0';
			snprintf(buffer, buffer_size, "line %zi col %zi at %s",T.line,T.col,tchars);
			free(tchars);
		}
		else {
			snprintf(buffer, buffer_size, "line ? col ? at ? (debug information not avalible)");
		}
		if (M != M->masterstate) {
			mercury_mstring_addchars(prepend, " in:\n", 5);
		}
		mercury_mstring_addchars(prepend, buffer, strlen(buffer));
		mercury_mstrings_append(prepend, out);
		mercury_mstring_delete(out);
		out = prepend;

		M = M->parentstate;
	}
	
	switch (errorcode) {
		case M_ERROR_ALLOCATION:
			mercury_mstring_addchars(out, ": memory allocation failiure.", 29);
			break;
		case M_ERROR_WRONG_TYPE:
			snprintf(buffer, buffer_size, ": arg %zi wrong type. expected %s, got %s\n", *data3, get_type_string((uint8_t)*data2), get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_DIV_ZERO:
			mercury_mstring_addchars(out, ": integer division by 0.", 23);
			break;
		case M_ERROR_INSTRUCTION_FAILIURE:
			mercury_mstring_addchars(out, ": failiure to execute instruction.", 23);
			break;
		case M_ERROR_CALL_NOT_FUNCTION:
			snprintf(buffer, buffer_size, ": attempt to call non-function value %s.", get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_INDEX_INVALID_TYPE:
			snprintf(buffer, buffer_size, ": attempt to index invalid variable type %s.", get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_NOT_ENOUGH_ARGS:
			snprintf(buffer, buffer_size, ": incorrect number of args. expected %zi, got %zi.", *data2, *data1);
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_CUSTOM_STRING:
			mercury_mstring_addchars(out,": ", 2);
			mercury_mstring_addchars(out, (char*)data1, strlen((char*)data1));
			break;
		case M_ERROR_WRONG_TYPE_EXPECTS_ANY_NUMBER:
			snprintf(buffer, buffer_size, ": arg %zi wrong type. expected a number, got %s.", *data2, get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION:
			snprintf(buffer, buffer_size, ": arg %zi wrong type. expected any function type, got %s.", *data2, get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		case M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE:
			snprintf(buffer, buffer_size, ": arg %zi wrong type. expected an array or table, got %s\n", *data2, get_type_string((uint8_t)*data1));
			mercury_mstring_addchars(out, buffer, strlen(buffer));
			break;
		default:
			mercury_mstring_addchars(out, ": unknown error.",16);
			break;
	}


	return out;
}

void mercury_raise_error(mercury_state* M, const uint32_t errorcode, const mercury_int* data1, const mercury_int* data2, const mercury_int* data3) {

	mercury_string* str = mercury_generate_error_string(M,errorcode, data1, data2, data3);


	if (str) {
		for (mercury_int i = 0; i < str->size; i++) {
			//printf("%c",str->ptr[i]);
			putchar(str->ptr[i]);
		}
		putchar('\n');
		mercury_mstring_delete(str);
	}

	while (M) {
		M->errorcode = errorcode;
		M->programcounter = M->bytecode.numberofinstructions; //push to end to stop execution
		M = M->parentstate;
	}
}
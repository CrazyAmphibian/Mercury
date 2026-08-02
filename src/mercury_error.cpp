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

		if (M->bytecode.instruction_dbg_lookup) {
			mercury_uint dbg_tok_num=M->bytecode.instruction_dbg_lookup[M->programcounter - 1];
			mercury_debug_token T,T_n,T_p;
			bool next=false;
			bool prev=false;
			if (dbg_tok_num)prev = true;
			if (dbg_tok_num+1 < M->bytecode.num_dbg_tokens)next = true;
			T=M->bytecode.dbg_tokens[dbg_tok_num];
			if(prev)T_p= M->bytecode.dbg_tokens[dbg_tok_num-1];
			if(next)T_n= M->bytecode.dbg_tokens[dbg_tok_num+1];


			char* tchars = (char*)malloc(T.num_chars + 1);
			if (!tchars) {
				mercury_mstring_delete(out);
				mercury_mstring_delete(prepend);
				return nullptr;
			}
			memcpy(tchars, T.chars, T.num_chars);
			tchars[T.num_chars] = '\0';

			char* n_tchars = nullptr;
			if (next) {
				n_tchars = (char*)malloc(T_n.num_chars + 1);
				if (!n_tchars) {
					free(tchars);
					mercury_mstring_delete(out);
					mercury_mstring_delete(prepend);
					return nullptr;
				}
				memcpy(n_tchars, T_n.chars, T_n.num_chars);
				n_tchars[T_n.num_chars] = '\0';
			}

			char* p_tchars = nullptr;
			if (next) {
				p_tchars = (char*)malloc(T_p.num_chars + 1);
				if (!p_tchars) {
					free(tchars);
					mercury_mstring_delete(out);
					mercury_mstring_delete(prepend);
					return nullptr;
				}
				memcpy(p_tchars, T_p.chars, T_p.num_chars);
				p_tchars[T_p.num_chars] = '\0';
			}

			if (next && prev) {
				snprintf(buffer, buffer_size, "line %zi col %zi at %s%s%s", T.line, T.col, p_tchars,tchars, n_tchars);
				free(p_tchars);
				free(n_tchars);
			}
			else if (next) {
				snprintf(buffer, buffer_size, "line %zi col %zi at %s%s", T.line, T.col, tchars,n_tchars);
				free(n_tchars);
			}
			else if (prev) {
				snprintf(buffer, buffer_size, "line %zi col %zi at %s%s", T.line, T.col, p_tchars,tchars);
				free(p_tchars);
			}
			else {
				snprintf(buffer, buffer_size, "line %zi col %zi at %s", T.line, T.col, tchars);
			}
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
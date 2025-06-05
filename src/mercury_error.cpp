#include "mercury_error.h"
#include "mercury_compiler.h"
#include "mercury.h"

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


		mercury_debug_token T = M->bytecode.debug_info[M->programcounter-1];
		snprintf(header,255,"line %lli col %lli at \"%s%s%s%s%s\"",T.line+1,T.col+1 ,T.token_prev_prev, T.token_prev , T.token, T.token_next, T.token_next_next);
	}
	else {
		snprintf(header, 255, "line ? col ? at ????? - instruction #%lli (%04X)",M->programcounter,0xFFFF&M->bytecode.instructions[M->programcounter]);
	}

	switch (errorcode) {
	case M_ERROR_NONE:
		return nullptr;
	case M_ERROR_ALLOCATION:
		result=snprintf(buffer, 255, "%s: memory allocation error\n",header);
		return mercury_cstring_const_to_mstring(buffer,strlen(buffer));
	case M_ERROR_WRONG_TYPE:
		result = snprintf(buffer, 255, "%s: wrong type. got %s, expected %s\n", header , typetostring[(int)data1] , typetostring[(int)data2]);
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

	M->programcounter = M->bytecode.numberofinstructions; //push to end to stop execution
}



mercury_stringliteral* mercury_generate_compiler_error_string(compiler_token** tokens, uint32_t errorcode, mercury_int token_err, mercury_int token_max) {
	char buffer[255] = { 0 };
	int result = 0;

	compiler_token* ct = tokens[token_err];

	if (ct == nullptr) {
		errorcode = M_COMPERR_UNKNOWN;
	}


	switch (errorcode) {
	case M_COMPERR_DID_NOT_CALL_OR_SET:
		result = snprintf(buffer, 255, "line %lli col %lli:expected expression, got %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_INVALID_SYMBOL:
		result = snprintf(buffer, 255, "line %lli col %lli:unknown symbol %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_ENDS_TOO_SOON:
		result = snprintf(buffer, 255, "line %lli col %lli:file ends too soon at %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_EXPECTED_VARIABLE:
		result = snprintf(buffer, 255, "line %lli col %lli:expected variable, got %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_IF_NEEDS_THEN:
		result = snprintf(buffer, 255, "line %lli col %lli:expected 'then' to match 'if', got %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_ELSEIF_NEEDS_THEN:
		result = snprintf(buffer, 255, "line %lli col %lli:expected 'then' to match 'elseif', got %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_MEMORY_ALLOCATION:
		result = snprintf(buffer, 255, "line %lli col %lli:failed to allocate memory while at token %s\n", ct->line_num + 1, ct->line_col, ct->chars);
		break;
	case M_COMPERR_WHILE_NEEDS_DO:
		result = snprintf(buffer, 255, "line %lli col %lli:expected 'do' to match 'while', got %s\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	case M_COMPERR_UNKNOWN:
		result = snprintf(buffer, 255, "invalid token data\n");
		break;
	case M_COMPERR_KEYWORD_REQUIRES_LOOP:
		result = snprintf(buffer, 255, "line %lli col %lli: %s requires loop\n", ct->line_num + 1, ct->line_col + 1, ct->chars);
		break;
	default:
		result = snprintf(buffer, 255, "line %lli col %lli:unspecified error at %s\n", ct->line_num+1, ct->line_col + 1, ct->chars );
	}
	return mercury_cstring_to_mstring(buffer, strlen(buffer));
}

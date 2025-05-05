#include "mercury_error.h"
#include "mercury_compiler.h"
#include "mercury.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"




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


mercury_stringliteral* mercury_generate_error_string(mercury_state* M, uint32_t errorcode , void* data1, void* data2, void* data3) {
	char buffer[255] = {0};
	int result = 0;

	switch (errorcode) {
	case M_ERROR_NONE:
		return nullptr;
	case M_ERROR_ALLOCATION:
		result=snprintf(buffer, 255, "instruction %li: memory allocation error\n", (long)data1);
		return mercury_cstring_const_to_mstring(buffer,strlen(buffer));
	case M_ERROR_WRONG_TYPE:
		result = snprintf(buffer, 255, "instruction %li: wrong type. got %s, expected %s\n", (long)data1 , typetostring[(int)data3] , typetostring[(int)data2]);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_DIV_ZERO:
		result = snprintf(buffer, 255, "instruction %li: integer division by 0\n", (long)data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INSTRUCTION_FAILIURE:
		result = snprintf(buffer, 255, "instruction %li: failiure to execute instruction\n", (long)data1);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_CALL_NOT_FUNCTION:
		result = snprintf(buffer, 255, "instruction %li: attempt to call non-function value %s \n", (long)data1 , typetostring[(int)data2] );
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	case M_ERROR_INDEX_INVALID_TYPE:
		result = snprintf(buffer, 255, "instruction %li: attempt to index invalid variable type %s \n", (long)data1, typetostring[(int)data2]);
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	default:
		result = snprintf(buffer, 255, "unknown error\n");
		return mercury_cstring_const_to_mstring(buffer, strlen(buffer));
	}
}


void mercury_raise_error(mercury_state* M, uint32_t errorcode, void* data1, void* data2, void* data3) {
	M->programcounter = M->numberofinstructions; //push to end to stop execution


	mercury_stringliteral* str = mercury_generate_error_string(M,errorcode,data1 , data2, data3);
	if (str != nullptr) {
		for (mercury_int i = 0; i < str->size; i++) {
			printf("%c",str->ptr[i]);
		}
	}

	
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
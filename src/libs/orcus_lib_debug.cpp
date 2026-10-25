#include "orcus_lib_debug.hpp"
#include"../orcus.hpp"
#include"../orcus_error.hpp"
#include"../orcus_bytecode.hpp"

#include <malloc.h>

//dumps length and contents of stack
void orcus_lib_debug_stack_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0);

	printf("current state: 0x%p size of stack: %zi (allocated: %zi)\n",M,M->sizeofstack,M->allocatedstacksize);
	for (orcus_uint i = 0; i < M->sizeofstack; i++) {
		orcus_variable v = M->stack[i];
		const char* typestr = "unknown";
		switch (v.type) {
		case M_TYPE_NIL:
			typestr = "nil";
			break;
		case M_TYPE_INT:
			typestr = "int";
			break;
		case M_TYPE_FLOAT:
			typestr = "float";
			break;
		case M_TYPE_BOOL:
			typestr = "bool";
			break;
		case M_TYPE_TABLE:
			typestr = "table";
			break;
		case M_TYPE_STRING:
			typestr = "string";
			break;
		case M_TYPE_CFUNC:
			typestr = "c function";
			break;
		case M_TYPE_ARRAY:
			typestr = "array";
			break;
		case M_TYPE_FUNCTION:
			typestr = "function";
			break;
		case M_TYPE_FILE:
			typestr = "file";
			break;
		case M_TYPE_THREAD:
			typestr = "thread";
			break;
		}
		printf("\t[%zu] = (%s %hhu) = i:%zi f:%f p:%p \n",i, typestr, v.type, v.data.i,v.data.f,v.data.p);
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}

void m_output_state(orcus_state* M, orcus_int* l,bool is_cur) {
	if (M->parentstate) {
		m_output_state(M->parentstate, l,false);
	}
	for (orcus_int i = 0; i < *l; i++) {
		putchar('\t');
	}
	printf("state 0x%p instruction pool: 0x%p [%zu/%zu] %s %s\n",M,M->bytecode->instructions,M->programcounter,M->bytecode->numberofinstructions,M==M->masterstate?"<MASTER> " :"", is_cur?"<CURRENT>" :"");
	(*l)++;
}

void orcus_lib_debug_state_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0);

	orcus_int c = 0;
	m_output_state(M, &c,true);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


char* m_var_to_string(uint8_t type,orcus_rawdata data) {
	char* out = (char*)malloc(sizeof(char)*2048 );
	if (!out)return nullptr;
	switch (type) {
	case M_TYPE_NIL:
		sprintf(out,"nil (%zi)",data.i);
		break;
	case M_TYPE_INT:
		sprintf(out, "%zi", data.i);
		break;
	case M_TYPE_FLOAT:
		sprintf(out, "%f", data.f);
		break;
	case M_TYPE_BOOL:
		sprintf(out, "%s", data.i?"true" :"false");
		break;
	case M_TYPE_TABLE:
		sprintf(out, "table 0x%p", data.p);
		break;
	case M_TYPE_STRING:
	{
		orcus_string* s = (orcus_string*)data.p;
		out[0] = '\"';
		orcus_int m_size = s->size>2040 ? 2040 : s->size;
		for (orcus_int i = 0; i < m_size; i++) {
			out[i+1] = s->ptr[i];
		}
		out[m_size+1] = '\"';
		out[m_size+2] = '\0';

	}
		break;
	case M_TYPE_CFUNC:
		sprintf(out, "c function 0x%p", data.p);
		break;
	case M_TYPE_ARRAY:
		sprintf(out, "array 0x%p", data.p);
		break;
	case M_TYPE_FUNCTION:
		sprintf(out, "function 0x%p", data.p);
		break;
	case M_TYPE_FILE:
		sprintf(out, "file 0x%p", data.p);
		break;
	case M_TYPE_THREAD:
		sprintf(out, "thread 0x%p", data.p);
		break;
	default:
		sprintf(out, "unknown %zi %f 0x%p", data.i, data.f, data.p);
		break;
	}
	return out;
}

void orcus_lib_debug_enviroment_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0);
	const orcus_state* S = M;
	while (S) {
		orcus_table* e = S->enviroment;
		printf("state 0x%p, evniroment 0x%p\n", S, e);
		for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
			orcus_subtable st = e->data[t];
			for (orcus_int i = 0; i < st.size; i++) {
				printf("\t%s = %s\n", m_var_to_string(t, st.keys[i].data), m_var_to_string(st.values[i].type, st.values[i].data));
			}

		}
		S = S->parentstate;
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void orcus_lib_debug_constants_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0);

	printf("state 0x%p has %zu constants\n", M,M->num_constants);

	for (orcus_uint i = 0; i < M->num_constants; i++) {
		orcus_variable c=M->constants[i];
		printf("\t%zu : %s\n", i, m_var_to_string(c.type, c.data));
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}



void orcus_lib_debug_bytecode_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0,1);

	if (args_in) { //read bytecode from function
		orcus_variable in;
		orcus_popstack(M,&in);
		if (in.type == M_TYPE_FUNCTION) {
			printf("variable function %p bytecode (%zu)\n", in.data.p, ((orcus_function*)in.data.p)->numberofinstructions );
			orcus_string* l = orcus_get_bytecode_debug( ((orcus_function*)in.data.p) );
			for (orcus_int i = 0; i < l->size; i++) {
				putchar(l->ptr[i]);
			}
			putchar('\n');
		}
		else {
			printf("variable (type %hhu) is not a function (type %hhu). failed to dump bytecode.",in.type,M_TYPE_FUNCTION);
		}
		orcus_release_var(&in);
	}
	else {

		printf("state %p bytecode (%zi/%zu)\n", M, M->programcounter, M->bytecode->numberofinstructions);
		orcus_string* l = orcus_get_bytecode_debug(M->bytecode);
		for (orcus_int i = 0; i < l->size; i++) {
			putchar(l->ptr[i]);
		}
		putchar('\n');
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void orcus_lib_debug_bytecode_rawbinary_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0, 1);

	if (args_in == 1) { //read bytecode from function
		orcus_variable in;
		orcus_popstack(M, &in);
		if (in.type == M_TYPE_FUNCTION) {
			printf("variable function %p bytecode (%zu)\n", in.data.p, ((orcus_function*)in.data.p)->numberofinstructions);
			orcus_string* l = orcus_get_bytecode_rawbinary_debug(((orcus_function*)in.data.p));
			for (orcus_int i = 0; i < l->size; i++) {
				putchar(l->ptr[i]);
			}
			putchar('\n');
		}
		else {
			printf("variable (type %hhu) is not a function (type %hhu). failed to dump bytecode.", in.type, M_TYPE_FUNCTION);
		}
		orcus_release_var(&in);
	}
	else {

		printf("state %p bytecode (%zi/%zu)\n", M, M->programcounter, M->bytecode->numberofinstructions);
		orcus_string* l = orcus_get_bytecode_rawbinary_debug(M->bytecode);
		for (orcus_int i = 0; i < l->size; i++) {
			putchar(l->ptr[i]);
		}
		putchar('\n');
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void orcus_lib_debug_refcount_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1);
	orcus_variable in;
	orcus_popstack(M, &in);

	switch (in.type) {
		case M_TYPE_STRING:
			printf("string has %zu refrences.\n", ((orcus_string*)in.data.p)->refrences);
			break;
		case M_TYPE_TABLE:
			printf("table has %zu refrences.\n", ((orcus_table*)in.data.p)->refrences);
			break;
		case M_TYPE_ARRAY:
			printf("array has %zu refrences.\n", ((orcus_array*)in.data.p)->refrences);
			break;
		case M_TYPE_FUNCTION:
			printf("function has %zu refrences.\n", ((orcus_function*)in.data.p)->refrences);
			break;
		case M_TYPE_THREAD:
			printf("thread has %zu refrences.\n", ((orcus_threadholder*)in.data.p)->refrences);
			break;
		case M_TYPE_FILE:
			printf("file has %zu refrences.\n", ((orcus_filewrapper*)in.data.p)->refrences);
			break;
		default:
			printf("type is not refcounted.\n");
	}
	orcus_release_var(&in);


	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}



void orcus_lib_debug_dump_debug_info_dbg(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0, 1);

	if (args_in) { //read from function
		orcus_variable in;
		orcus_popstack(M, &in);
		if (in.type == M_TYPE_FUNCTION) {
			orcus_function* func = (orcus_function*)in.data.p;
			orcus_debug_token* toks = func->dbg_tokens;
			printf("variable function %p debug info:\n", in.data.p);

			if (toks) {
				for (orcus_uint i = 0; i < func->num_dbg_tokens; i++) {
					orcus_debug_token t = toks[i];
					printf("%zu] ln:%zi col:%zi ", i,t.line,t.col);
					for (orcus_int c = 0; c < t.num_chars; c++) {
						putchar(t.chars[c]);
					}
					putchar('\n');
				}

			}

		}
		else {
			printf("variable (type %hhu) is not a function (type %hhu). failed to dump bytecode.",in.type, M_TYPE_FUNCTION);
		}
		orcus_release_var(&in);
	}
	else {

		orcus_debug_token* toks = M->bytecode->dbg_tokens;
		printf("state %p debug info:\n", M);
		if (toks) {
			for (orcus_uint i = 0; i < M->bytecode->num_dbg_tokens; i++) {
				orcus_debug_token t = toks[i];
				printf("%zu] ln:%zi col:%zi ", i, t.line, t.col);
				for (orcus_int c = 0; c < t.num_chars; c++) {
					putchar(t.chars[c]);
				}
				putchar('\n');
			}

		}
		
	}

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}


void orcus_lib_debug_get_traceback(orcus_state* const M_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0);
	if (!args_out)return;

	orcus_variable out;
	out.type = M_TYPE_STRING;
	out.data.p=orcus_get_state_traceback(M);
	if (!out.data.p) {
		orcus_raise_error_nonpointer(M, M_ERROR_ALLOCATION);
		return;
	}
	orcus_pushstack(M, &out);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

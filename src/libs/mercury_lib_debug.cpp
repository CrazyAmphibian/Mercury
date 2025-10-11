#include "mercury_lib_debug.h"
#include"../mercury.h"
#include"../mercury_error.h"
#include"../mercury_bytecode.h"

#include <malloc.h>

//dumps length and contents of stack
void mercury_lib_debug_stack_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int a = 0; a < args_in; a++) {
		mercury_unassign_var(M,mercury_pullstack(M));
	}


	printf("current state: 0x%p size of stack: %li (allocated: %li) [unassigned: %li allocated: %li]\n",M,M->sizeofstack,M->actualstacksize,M->numunassignedstack,M->sizeunassignedstack);
	for (mercury_int i = 0; i < M->sizeofstack; i++) {
		mercury_variable* v = M->stack[i];
		const char* typestr = "unknown";
		switch (v->type) {
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
		printf("\t[%i] 0x%p = (%s%s %i) = i:%i f:%f p:%p \n",i,v,v->constant ? "<CONSTANT> " : " ", typestr, v->type, v->data.i,v->data.f,v->data.p);
	}


	for (mercury_int a = 0; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}

void m_output_state(mercury_state* M, mercury_int* l,bool is_cur) {
	if (M->parentstate) {
		m_output_state(M->parentstate, l,false);
	}
	for (mercury_int i = 0; i < *l; i++) {
		putchar('\t');
	}
	printf("state 0x%p instruction pool: 0x%p [%i/%i] %s %s\n",M,M->bytecode.instructions,M->programcounter,M->bytecode.numberofinstructions,M==M->masterstate?"<MASTER> " :"", is_cur?"<CURRENT>" :"");
	(*l)++;
}

void mercury_lib_debug_state_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int a = 0; a < args_in; a++) {
		mercury_unassign_var(M, mercury_pullstack(M));
	}

	mercury_int c = 0;
	m_output_state(M, &c,true);

	for (mercury_int a = 0; a < args_out; a++) {
		M_BYTECODE_NNIL(M, 0);
	}
}


char* m_var_to_string(uint8_t type,mercury_rawdata data) {
	char* out = (char*)malloc(sizeof(char)*2048 );
	if (!out)return nullptr;
	switch (type) {
	case M_TYPE_NIL:
		sprintf(out,"nil (%i)",data.i);
		break;
	case M_TYPE_INT:
		sprintf(out, "%i", data.i);
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
		mercury_stringliteral* s = (mercury_stringliteral*)data.p;
		out[0] = '\"';
		mercury_int m_size = s->size>2040 ? 2040 : s->size;
		for (mercury_int i = 0; i < m_size; i++) {
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
		sprintf(out, "unknown %i %f 0x%p", data.i, data.f, data.p);
		break;
	}
	return out;
}

void mercury_lib_debug_enviroment_dbg(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	while (M) {
		mercury_table* e = M->enviroment;
		printf("state 0x%p, evniroment 0x%p\n", M, e);
		for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
			mercury_subtable* st = e->data[t];
			for (mercury_int i = 0; i < st->size; i++) {

				printf("\t%s = %s\n", m_var_to_string(t, st->keys[i]), m_var_to_string(st->values[i]->type, st->values[i]->data));
			}

		}
		M = M->parentstate;
	}
}

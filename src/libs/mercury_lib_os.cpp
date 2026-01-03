#include "../mercury.hpp"
#include "../mercury_error.hpp"

#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>



void mercury_lib_os_time(mercury_state* M, mercury_int args_in, mercury_int args_out) { //gets the current unix/epoch time.
	for (mercury_int i = 0; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	if (!args_out) {
		return;
	}

	mercury_variable* out = mercury_assign_var(M);

	mercury_int t=time(NULL);
	out->type = M_TYPE_INT;
	out->data.i = t;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_os_execute(mercury_state* M, mercury_int args_in, mercury_int args_out) { //dangerous!
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* cvar = mercury_popstack(M);
	if (cvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)cvar->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* code = (mercury_stringliteral*)cvar->data.p;

	char* c_code = (char*)malloc(sizeof(char) * (code->size + 1));
	if (!c_code) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (mercury_int i = 0; i < code->size; i++) {
		c_code[i] = code->ptr[i];
	}
	c_code[code->size] = '\0'; //end will null for c string compatability.

	//system(c_code);
#ifdef _WIN32
	FILE* a = _popen(c_code, "r");
#else
	FILE* a = popen(c_code, "r");
#endif 

	char* out_c = nullptr;
	mercury_int size_c = 0;
	mercury_int allocated_c = 0;

	if (a) {
		int c = 0;
		while (1) {
			c = fgetc(a);
			if (c == EOF)break;


			if (size_c == allocated_c) {
				void* o = realloc(out_c, allocated_c + 128);
				if (!o) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				out_c = (char*)o;
				allocated_c += 128;
			}
			out_c[size_c] = (char)c;
			size_c++;
		}
#ifdef _WIN32
		_pclose(a);
#else
		pclose(a);
#endif
	}

	if (!args_out) {
		for (mercury_int i = 0; i < size_c; i++) {
			putchar(out_c[i]);
		}
		free(out_c);
		return;
	}


	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	
	out->data.p= mercury_cstring_const_to_mstring(out_c, size_c);
	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_os_call(mercury_state* M, mercury_int args_in, mercury_int args_out) { //dangerous!
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* cvar = mercury_popstack(M);
	if (cvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)cvar->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* code = (mercury_stringliteral*)cvar->data.p;

	char* c_code = (char*)malloc(sizeof(char) * (code->size + 1));
	if (!c_code) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (mercury_int i = 0; i < code->size; i++) {
		c_code[i] = code->ptr[i];
	}
	c_code[code->size] = '\0'; //end will null for c string compatability.

	//system(c_code);

	int r=system(c_code);

	if (!args_out) {
		return;
	}


	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_INT;
	out->data.i = r;
	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_os_clock(mercury_state* M, mercury_int args_in, mercury_int args_out) { //gets the time since program startup.
	for (mercury_int i = 0; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}
	if (!args_out) {
		return;
	}

	mercury_variable* out = mercury_assign_var(M);

	mercury_float t = ((mercury_float)clock()) / CLOCKS_PER_SEC;
	out->type = M_TYPE_FLOAT;
	out->data.f = t;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

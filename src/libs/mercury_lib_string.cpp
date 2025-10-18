#include "mercury_lib_string.h"
#include "../mercury.h"
#include "../mercury_error.h"

#include "malloc.h"
#ifndef _WIN32
#include <string.h>
#endif

void mercury_lib_string_sub(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 3) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 3; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* var_end = mercury_popstack(M);
	if (var_end->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_end->type, (void*)M_TYPE_INT, (void*)3);
		return;
	}
	mercury_variable* var_start = mercury_popstack(M);
	if (var_start->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_start->type, (void*)M_TYPE_INT, (void*)2);
		return;
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* os=mercury_mstring_substring((mercury_stringliteral*)var_string->data.p, var_start->data.i, var_end->data.i);
	mercury_variable* out =mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = os;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




void mercury_lib_string_reverse(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char)*str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int c = 0; c < str->size; c++) {
		os->ptr[str->size - c - 1] = str->ptr[c];
	}
	os->size = str->size;

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = os;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}

}



void mercury_lib_string_find(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 3; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* startat = nullptr;
	if (args_in > 2) {
		startat = mercury_popstack(M);
		if (startat->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)startat->type, (void*)M_TYPE_INT, (void*)3);
			return;
		}
	}

	mercury_variable* searchforvar = mercury_popstack(M);
	if (searchforvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)searchforvar->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* str = (mercury_stringliteral*)strvar->data.p;
	mercury_stringliteral* search = (mercury_stringliteral*)searchforvar->data.p;

	mercury_int located_at = -1;

	for (mercury_int c = (startat ? startat->data.i : 0); c < str->size-search->size; c++) {
		for (mercury_int i = 0; i < search->size; i++) {
			if (str->ptr[c + i] != search->ptr[i]) {
				goto next;
			}
		}
		located_at = c;
		break;
		next:
		{}
	}

	mercury_variable* out = mercury_assign_var(M);

	if (located_at != -1) {
		out->type = M_TYPE_INT;
		out->data.i = located_at;
	}
	else {
		out->type = M_TYPE_NIL;
		out->data.i = 0;
	}

	mercury_pushstack(M, out);
	
	//second one is end char.
	if (args_out > 1) {
		mercury_variable* out2 = mercury_assign_var(M);

		if (located_at != -1) {
			out2->type = M_TYPE_INT;
			out2->data.i = located_at+search->size-1;
		}
		else {
			out2->type = M_TYPE_NIL;
			out2->data.i = 0;
		}
	}

	if (startat)mercury_unassign_var(M, startat);
	mercury_unassign_var(M, searchforvar);
	mercury_unassign_var(M, strvar);

	for (mercury_int a = 2; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_string_replace(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 3) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 4; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}


	mercury_variable* max_replacments = nullptr;
	if (args_in > 3) {
		max_replacments = mercury_popstack(M);
		if (max_replacments->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)max_replacments->type, (void*)M_TYPE_INT, (void*)4);
			return;
		}
	}
	mercury_variable* replace_var = mercury_popstack(M);
	if (replace_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)replace_var->type, (void*)M_TYPE_STRING, (void*)3);
		return;
	}

	mercury_variable* search_var = mercury_popstack(M);
	if (search_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)search_var->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* outstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outstr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	outstr->size = 0;
	outstr->ptr = nullptr;
	
	mercury_stringliteral* str = (mercury_stringliteral*)str_var->data.p;
	mercury_stringliteral* search = (mercury_stringliteral*)search_var->data.p;
	mercury_stringliteral* replace = (mercury_stringliteral*)replace_var->data.p;

	mercury_int replacments = 0;

	for (mercury_int c = 0; c < str->size;) {
		if (max_replacments&& replacments>=max_replacments->data.i)goto next;
		if (c >= str->size - search->size)goto next;
		for (mercury_int i = 0; i < search->size; i++) {
			if (str->ptr[c + i] != search->ptr[i]) {
				goto next;
			}
		}
		//string found
		replacments++;
		mercury_mstrings_append(outstr, replace);
		c += search->size;
		continue;
		next:
		//string not found.
		mercury_mstring_addchars(outstr, str->ptr+c,1);
		c++;
	}

	mercury_unassign_var(M, str_var);
	mercury_unassign_var(M, search_var);
	mercury_unassign_var(M, replace_var);

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = outstr;

	mercury_pushstack(M, out);

	if (args_out > 1) {
		mercury_variable* out2 = mercury_assign_var(M);
		out2->type = M_TYPE_INT;
		out2->data.i = replacments;
		mercury_pushstack(M, out2);
	}

	for (mercury_int a = 2; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_count(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* search_var = mercury_popstack(M);
	if (search_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)search_var->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}
	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}


	mercury_stringliteral* str = (mercury_stringliteral*)str_var->data.p;
	mercury_stringliteral* search = (mercury_stringliteral*)search_var->data.p;

	mercury_int replacments = 0;

	for (mercury_int c = 0; c < str->size;) {
		if (c >= str->size - search->size)goto next;
		for (mercury_int i = 0; i < search->size; i++) {
			if (str->ptr[c + i] != search->ptr[i]) {
				goto next;
			}
		}
		//string found
		replacments++;
		continue;
	next:
		c++;
	}

	mercury_unassign_var(M, str_var);
	mercury_free_var(search_var,true);

	search_var->type = M_TYPE_INT;
	search_var->data.i = replacments;

	mercury_pushstack(M, search_var);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_toarray(mercury_state* M, mercury_int args_in, mercury_int args_out) { //converts the bytes to an array of ints.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)str_var->data.p;

	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int i = 0; i < str->size; i++) {
		mercury_variable* v=(mercury_variable*)malloc(sizeof(mercury_variable));
		if (!v)continue;
		v->type = M_TYPE_INT;
		v->data.i = str->ptr[i];
		mercury_setarray(arr, v, i);
	}

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_ARRAY;
	out->data.p = arr;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_fromarray(mercury_state* M, mercury_int args_in, mercury_int args_out) { //converts an array of ints into a string
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_ARRAY) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_ARRAY, (void*)1);
		return;
	}
	mercury_array* arr = (mercury_array*)str_var->data.p;

	mercury_int len=mercury_array_len(arr);

	mercury_stringliteral* st = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral*));
	if (!st) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st->ptr=(char*)malloc(len * sizeof(char));
	if (!st->ptr) {
		free(st);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	st->size = len;

	for (mercury_int i = 0; i < len; i++) {
		mercury_variable* v =mercury_getarray(arr, i);
		if (v->type == M_TYPE_INT) {
			st->ptr[i] = v->data.i & 0xFF;
		}
		else {
			st->ptr[i] = '\0';
		}
	}

	mercury_variable* out=mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = st;
	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_separate(mercury_state* M, mercury_int args_in, mercury_int args_out) { //seperates a string into an array of smaller strings.
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}


	mercury_variable* sep_var = mercury_popstack(M);
	if (sep_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)sep_var->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}


	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	mercury_stringliteral* str = (mercury_stringliteral*)str_var->data.p;
	mercury_stringliteral* sep = (mercury_stringliteral*)sep_var->data.p;

	mercury_int arr_elems = 0;
	mercury_int last_find = 0;

	for (mercury_int c = 0; c < str->size;) {
		if (c >= str->size - sep->size)goto next;
		for (mercury_int i = 0; i < sep->size; i++) {
			if (str->ptr[c + i] != sep->ptr[i]) {
				goto next;
			}
		}
		//string found
		{
			mercury_stringliteral* ns = mercury_cstring_to_mstring(str->ptr + last_find, c- last_find);
			if (!ns)continue;
			mercury_variable* v=(mercury_variable*)malloc(sizeof(mercury_variable));
			if (!v)continue;
			v->type = M_TYPE_STRING;
			v->data.p = ns;
			mercury_setarray(arr, v, arr_elems);
			arr_elems++;
			last_find = c+sep->size;
		}
		c += sep->size < 1 ? 1 : sep->size;
		continue;
	next:
		//string not found.
		c++;
	}

	{
		mercury_stringliteral* ns = mercury_cstring_to_mstring(str->ptr + last_find, str->size - last_find);
		if (!ns)return;
		mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!v)return;
		v->type = M_TYPE_STRING;
		v->data.p = ns;
		mercury_setarray(arr, v, arr_elems);
		arr_elems++;
	}


	mercury_unassign_var(M, sep_var);
	mercury_unassign_var(M, str_var);
	
	mercury_variable* out=mercury_assign_var(M);
	out->type = M_TYPE_ARRAY;
	out->data.p = arr;
	mercury_pushstack(M, out);


	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_upper(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char) * str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int c = 0; c < str->size; c++) {
		char cc = str->ptr[c];
		if (cc >= 'a' && cc <= 'z') {
			os->ptr[c] = cc-0x20;
		}
		else {
			os->ptr[c] = cc;
		}
		
	}
	os->size = str->size;

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = os;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}

}


void mercury_lib_string_lower(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char) * str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int c = 0; c < str->size; c++) {
		char cc = str->ptr[c];
		if (cc >= 'A' && cc <= 'Z') {
			os->ptr[c] = cc + 0x20;
		}
		else {
			os->ptr[c] = cc;
		}

	}
	os->size = str->size;

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = os;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}

}

enum stringformat_argpositons {
	ARG_WIDTH = 1,
	ARG_PERCISION = 2,
	ARG_PADDINGISZERO = 3,
	ARG_LEFTALIGN = 4,
	ARG_FORCEPLUS = 5,
	ARG_ALTERNATENUM = 6
};

char* getprintfstring(mercury_int* args, bool* args_def, const char* type) {
	char* str=(char*)malloc(sizeof(char) * 256);
	if (!str)return nullptr;
	int ptr = 1;
	str[0] = '%';

	if (args_def[ARG_FORCEPLUS]) {
		switch (args[ARG_FORCEPLUS]) {
		case 1:
			str[ptr] = '+';
			break;
		case 2:
			str[ptr] = ' ';
			break;
		}
		ptr++;
	}
	if (args_def[ARG_LEFTALIGN]) {
		str[ptr] = '-';
		ptr++;
	}
	if (args_def[ARG_ALTERNATENUM]) {
		str[ptr] = '#';
		ptr++;
	}
	if (args_def[ARG_PADDINGISZERO]) {
		str[ptr] = '0';
		ptr++;
	}

	if (args_def[ARG_WIDTH]) {
		str[ptr] = '*';
		ptr++;
	}

	if (args_def[ARG_PERCISION]) {
		str[ptr] = '.';
		str[ptr+1] = '*';
		ptr+=2;
	}

	memcpy(str+ptr,type,strlen(type));
	ptr+=strlen(type);

	str[ptr] = '\0';
	return str;
}


union m_rawdata_snowflake_printf { //because printf promotes floats to doubles for some fucking reason. why?
	double f;
	mercury_int i;
	mercury_uint u;
	void* p;
};

// TODO: add a, x, g, e, p, u, o
int m_readformat(mercury_stringliteral* str, mercury_int offset, mercury_stringliteral* str_out, mercury_variable** v_arr, mercury_int* num_vars) {
	mercury_int add_off = 0;

	unsigned char nargs = 0;
	bool args_def[256] = { false }; //if the arg is defined.
	mercury_int args[256] = {0}; // the value of that arg.
	char mode = 0;

	char printfbuffer[1024] = {'0'};

	const char* type=nullptr;
	m_rawdata_snowflake_printf value;
	value.i = 0;

	while (true) {
		if (add_off + offset >= str->size)break;
		char c = str->ptr[add_off + offset];
		add_off++;
		switch (c) {
		case '%':
			mercury_mstring_addchars(str_out,(char*)"%",1);
			//add_off++;
			goto exit;
		case 'I':
		case 'i': // integer
			if (!type) {
#ifdef MERCURY_64BIT
				type = "lli";
#else
				type = "i";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.i = mercury_checkint(v_arr[*num_vars]);
				}		
			}
			goto output;
		case 'x': // hex int
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llx";
#else
				type = "x";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.i = mercury_checkint(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'X': // hex int (capital)
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llX";
#else
				type = "X";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.i = mercury_checkint(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'f':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llf";
#else
				type = "f";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			
			goto output;
		case 'F':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llF";
#else
				type = "F";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'e':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "lle";
#else
				type = "e";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'E':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llE";
#else
				type = "E";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'g':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llg";
#else
				type = "g";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'G':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llG";
#else
				type = "G";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'a':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "lla";
#else
				type = "a";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'A':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llA";
#else
				type = "A";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value.f = mercury_checkfloat(v_arr[*num_vars]);
				}
			}
			goto output;
		case 'p':
		case 'P':
			if (!type) {
				type = "p";
				if (*num_vars) {
					(*num_vars)--;
					value.p = mercury_checkpointer(v_arr[*num_vars]);
				}
			}
			goto output;
			output:
			{

			mercury_int l = 0;
			mercury_int p = 0;
			if (args_def[ARG_WIDTH])l = args[ARG_WIDTH];
			if (args_def[ARG_PERCISION])p = args[ARG_PERCISION];

			char* printf_fstr = getprintfstring(args,args_def,type);
			if (!printf_fstr)break;

			if (args_def[ARG_WIDTH]) {
				if (args_def[ARG_PERCISION]) {
					snprintf(printfbuffer, 1024, printf_fstr,l,p, value);
				}
				else {
					snprintf(printfbuffer, 1024, printf_fstr, l, value);
				}
			}
			else {
				if(args_def[ARG_PERCISION]) {
					snprintf(printfbuffer, 1024, printf_fstr, p, value);
				}
				else {
					snprintf(printfbuffer, 1024, printf_fstr, value);
				}
			}
			free(printf_fstr);
			mercury_mstring_addchars(str_out, printfbuffer, strlen(printfbuffer));
			goto exit;
			}
		

		case 'S':
		case 's': // string
		{
			mercury_stringliteral* v = nullptr;
			if (*num_vars) {
				(*num_vars)--;
				mercury_variable* var = v_arr[*num_vars];
				mercury_variable* sv=mercury_tostring(var);
				v = (mercury_stringliteral*)sv->data.p;
				free(sv);

			}
			if (v) {
				mercury_mstrings_append(str_out, v);
				free(v);
			}
			add_off++;
			goto exit;
		}
		case '-':
			if (!mode) {
				args_def[ARG_LEFTALIGN] = true;
				args[ARG_LEFTALIGN] = 1;
			}
			break;
		case '#':
			if (!mode) {
				args_def[ARG_ALTERNATENUM] = true;
				args[ARG_ALTERNATENUM] = 1;
			}
			break;
		case '+':
			if (!mode) {
				args_def[ARG_FORCEPLUS] = true;
				args[ARG_FORCEPLUS] = 1;
			}
			break;
		case ' ':
			if (!mode && !args[ARG_FORCEPLUS]) {
				args_def[ARG_FORCEPLUS] = true;
				args[ARG_FORCEPLUS] = 2;
			}
			break;
		case '.': // X.Y notation. swap to another mode.
			mode = ARG_PERCISION;
			//add_off++;
			break;
		case '0':
			if (!mode) {
				args_def[ARG_PADDINGISZERO]=true;
				args[ARG_PADDINGISZERO] = 1;
			}
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (!mode) { mode = ARG_WIDTH; }
			args_def[mode] = true;
			args[mode] *= 10;
			args[mode] += (c-'0');
			//add_off++;
			break;
		default:
			//break;
			goto exit;
		}
	}
	exit:

	return add_off;
}



void mercury_lib_string_format(mercury_state* M, mercury_int args_in, mercury_int args_out) { //c-style printf formatting.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}

	mercury_int var_s = args_in - 1;
	mercury_variable** var_t = (mercury_variable**)malloc(sizeof(mercury_variable*)*args_in-1);
	if (!var_t) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int i = 1; i < args_in; i++) {
		var_t[i-1]=mercury_popstack(M);
	}

	mercury_variable* strvar=mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}
	mercury_stringliteral* s = (mercury_stringliteral*)strvar->data.p;

	mercury_int pc = 0;
	char* buffer = (char*)malloc(sizeof(char) * 200);
	mercury_int sbuf = 200;
	mercury_int cc = 0;

	mercury_stringliteral* outstr = mercury_cstring_const_to_mstring((char*)"",0);
	
	/*
	ARG1 = 1
	ARG2 = 2
	*/

	while (pc<s->size) {
		mercury_int na1 = 0; //numeric argument 1
		mercury_int na2 = 0; //numeric argument 2
		unsigned char setargs = 0;
		unsigned char tacker = 0;
		char c = s->ptr[pc];
		if (c == '%') {
			pc++;
			if (pc < s->size) {
				pc+=m_readformat(s,pc, outstr,var_t,&var_s);
			}
		}
		else {
			mercury_mstring_addchars(outstr, s->ptr+pc, 1);
			pc++;
		}
	next_char:
		{}
	}


	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_STRING;
	out->data.p = outstr; //mercury_cstring_to_mstring(buffer, cc);

	free(buffer);


	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



/*
	patterns!

	selectors:
	%n - number
	%l - letter
	%w - lowercase letters
	%u - uppercase letters
	%a - alphanumberic
	%s - symbols
	%  - whitespace
	%. - wildcard
	%% - % literal

	specifiers:
	%* - 0 to infinity matches, greedy
	%~ - 0 to infinity matches
	%- - 1 to infinity matches
	%+ - 1 to infinity matches, greedy
	%? - 0 or 1 matches
	%! - 1 match (implicit behavior made explicit)
	%/ - 0 matches, next char
	%\ - 0 matches, greedy

*/


enum M_PAT_MATCH {
	//count flags
	MATCH_SINGLE = (1<<0), //we only want a single char, and stop as soon as we find what we want.
	MATCH_AT_LEAST_ONE = (1<<1), //we can match many chars, and keep going until the rule stops being true.
	//behavior flags
	MATCH_OPTIONAL = (1<<2), //if we aren't required, just set passed to true. still iterates through chars to advance filters
	MATCH_GREEDY = (1<<3), //if flag, do not allow traversal through first_good_char
	MATCH_INVERTED = (1<<4), //we do NOT want this char.
};

struct M_PATTERN{
	bool allowed_chars[256] = { false };
	int match_type = MATCH_SINGLE;
	mercury_int first_good_char = 0; //succesive checks will start at this +1 if not MATCH_GREEDY
	mercury_int last_good_char = 0; //and end at this.
	bool passed = false;
};

void m_initialize_pattern(M_PATTERN* P) {
	if (!P)return;
	P->first_good_char = 0;
	P->last_good_char = 0;
	P->match_type = MATCH_SINGLE;
	P->passed = false;
	int i = 0xFF;
	while (i>=0) {
		P->allowed_chars[i] = false;
		i--;
	}
}


inline void m_init_pattern_filter_numbers(M_PATTERN* P){
	if (!P)return;
	int i = '0';
	while (i <= '9') {
		P->allowed_chars[i] = true;
		i++;
	}
}
inline void m_init_pattern_filter_letter_upper(M_PATTERN* P) {
	if (!P)return;
	int i = 'A';
	while (i <= 'Z') {
		P->allowed_chars[i] = true;
		i++;
	}
}

inline void m_init_pattern_filter_letter_lower(M_PATTERN* P) {
	if (!P)return;
	int i = 'a';
	while (i <= 'z') {
		P->allowed_chars[i] = true;
		i++;
	}
}

inline void m_init_pattern_filter_wildcard(M_PATTERN* P) {
	if (!P)return;
	int i = '\x00';
	while (i <= (unsigned char)'\xFF') {
		P->allowed_chars[i] = true;
		i++;
	}
}

inline void m_init_pattern_filter_symbols(M_PATTERN* P) {
	if (!P)return;
	int i = '\x21';
	while (i <= '\x2F') {
		P->allowed_chars[i] = true;
		i++;
	}
	i = '\x3A';
	while (i <= '\x40') {
		P->allowed_chars[i] = true;
		i++;
	}
	i = '\x5B';
	while (i <= '\x60') {
		P->allowed_chars[i] = true;
		i++;
	}
	i = '\x7B';
	while (i <= '\x7E') {
		P->allowed_chars[i] = true;
		i++;
	}
}

inline void m_init_pattern_filter_whitespace(M_PATTERN* P) {
	if (!P)return;
	P->allowed_chars[' '] = true;
	P->allowed_chars['\t'] = true;
	P->allowed_chars['\n'] = true;
	P->allowed_chars['\r'] = true;
}

inline void m_init_mattern_invert_filter_chars(M_PATTERN* P) {
	if (!P)return;
	int i = '\x00';
	while (i <= (unsigned char)'\xFF') {
		P->allowed_chars[i] = !P->allowed_chars[i];
		i++;
	}
}

M_PATTERN* m_patternize_string(mercury_stringliteral* str,mercury_int* num_out) {
	mercury_int n = 0;
	M_PATTERN* out = nullptr;


	for (mercury_int c = 0; c < str->size; c++) {
		char cc = str->ptr[c];
		//printf("%i %c\n",c,cc);
		M_PATTERN* nptr = (M_PATTERN*)realloc(out, sizeof(M_PATTERN) * (n + 1));
		if (!nptr)return nullptr;
		out = nptr;
		M_PATTERN* p = out+n;
		n++;
		m_initialize_pattern(p);
		
		//look for the first part of the pattern, the character set.
		if (cc == '%' && (c + 1) < str->size) {
			char nc = str->ptr[c + 1];
			c++;
			switch (nc) {
			case 'n':
				m_init_pattern_filter_numbers(p);
				break;
			case 'l':
				m_init_pattern_filter_letter_upper(p);
				m_init_pattern_filter_letter_lower(p);
				break;
			case 'u':
				m_init_pattern_filter_letter_upper(p);
				break;
			case 'w':
				m_init_pattern_filter_letter_lower(p);
				break;
			case 'a':
				m_init_pattern_filter_numbers(p);
				m_init_pattern_filter_letter_upper(p);
				m_init_pattern_filter_letter_lower(p);
				break;
			case 's':
				m_init_pattern_filter_symbols(p);
				break;
			case ' ':
				m_init_pattern_filter_whitespace(p);
				break;
			case '.':
				m_init_pattern_filter_wildcard(p);
				break;
			case '%': //explicitly define this as intented behavior. if you're not escaping %, then you're just asking for trouble later down the line.
			default:
				p->allowed_chars[nc] = true;
			}
		}
		else {
			p->allowed_chars[cc] = true;
		}

		p->match_type = MATCH_SINGLE;

		//next, look for the second half, the selector.
		if (c + 1 >= str->size)continue;
		
		cc = str->ptr[c+1];
		if (cc == '%' && (c + 2) < str->size) {
			char nc = str->ptr[c + 2];
			c+=2;
			switch (nc) {
			case '+':
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_GREEDY;
				break;
			case '-':
				p->match_type = MATCH_AT_LEAST_ONE;
				break;
			case '*':
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_OPTIONAL | MATCH_GREEDY;
				break;
			case '~':
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_OPTIONAL;
				break;
			case '?':
				p->match_type = MATCH_SINGLE | MATCH_OPTIONAL;
				break;
			case '!':
				p->match_type = MATCH_SINGLE;
				break;
			case '/':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_OPTIONAL | MATCH_GREEDY;
				break;
			case '\\':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_OPTIONAL;
				break;
			case '#':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_AT_LEAST_ONE | MATCH_GREEDY;
				break;
			case '@':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_AT_LEAST_ONE;
				break;
			case '|':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_SINGLE | MATCH_OPTIONAL;
				break;
			case '=':
				m_init_mattern_invert_filter_chars(p);
				p->match_type = MATCH_SINGLE;
				break;
			case '%': //see above.
			default:
				c -= 2; //invalid selector? we drop the character advance, so we can treat it like a normal character sequence.
			}
		}


	}

	if (num_out)*num_out = n;
	return out;
}


bool m_evaluate_patterns(mercury_stringliteral* str, M_PATTERN* patterns, mercury_int num_patterns, mercury_int str_start, mercury_int* pattern_str_start, mercury_int* pattern_str_end) {

#if defined(DEBUG) || defined(_DEBUG)

	for (mercury_int i = 0; i < num_patterns; i++) {

		M_PATTERN P = patterns[i];
		printf("pattern %i: ",i);

		if (P.match_type & MATCH_SINGLE) {
			printf("MATCH_SINGLE ");
		}
		if (P.match_type & MATCH_AT_LEAST_ONE) {
			printf("MATCH_AT_LEAST_ONE ");
		}
		if (P.match_type & MATCH_GREEDY) {
			printf("MATCH_GREEDY ");
		}
		if (P.match_type & MATCH_OPTIONAL) {
			printf("MATCH_OPTIONAL ");
		}
		if (P.match_type & MATCH_INVERTED) {
			printf("MATCH_INVERTED ");
		}

		for (int n = 0; n <= 0xFF; n++) {
			if (P.allowed_chars[n])putchar(n);
		}
		putchar('\n');
	}

#endif



	*pattern_str_start = 0;
	*pattern_str_end = 0;
	if (str_start >= str->size)return false;
	str_start--;
	if (!num_patterns)return true;

	mercury_int advanced_from = 0;
	mercury_int current_pattern = 0;
	advance:
	str_start++;
	if (str_start >= str->size)return false;
	*pattern_str_start = str_start;
	advanced_from = 0;
	current_pattern = 0;
	for (mercury_int n = 0; n < num_patterns; n++) {
		patterns[n].passed = false;
		patterns[n].first_good_char = 0;
		patterns[n].last_good_char = 0;
	}
	for (mercury_int c = str_start; c < str->size; c++) {
		char cc = str->ptr[c];
		
		
		M_PATTERN* p = patterns+current_pattern;
		if (!current_pattern && !p->passed)*pattern_str_start = c;
		if (p->allowed_chars[cc]) {
			if (p->match_type & MATCH_SINGLE) {
				current_pattern++;
			}
			else {
				if (!p->passed) {
					p->first_good_char = c;
				}
				p->last_good_char = c;
			}
			p->passed = true;
		}
		else {
			if (p->passed) {
				if (!(p->match_type & MATCH_GREEDY)) {
					advanced_from = c;
					c = p->first_good_char + 1;
				}
				else {
					c--;
				}
				current_pattern++;
			}else if (p->match_type & MATCH_OPTIONAL) {
				current_pattern++;
				c--;
			}

			if(c>=advanced_from && !(p->match_type & MATCH_OPTIONAL) && !p->passed ) {
				goto advance;
			}
		}	

		*pattern_str_end = c;

		if (current_pattern == num_patterns) {
			bool anypass = false;
			mercury_int passed = 0;
			mercury_int req_pas = 0;
			for (mercury_int n = 0; n < num_patterns; n++) {
				if ((patterns[n].match_type & MATCH_INVERTED) ? (!patterns[n].passed) : patterns[n].passed) {
					if (!(patterns[n].match_type & MATCH_OPTIONAL)) {
						passed++;
					}
					anypass = true;
				}
				if (!(patterns[n].match_type & MATCH_OPTIONAL)) {
					req_pas++;
				}
			}
			//printf("%i %i %i\n", anypass, passed, req_pas);
			if (anypass && passed >= req_pas) {
				break;
			}
			else {
				goto advance;
			}
		}
		if (c+1 == str->size && p->match_type&MATCH_AT_LEAST_ONE) {
			current_pattern++;
			c = p->first_good_char;
		}

	}


	bool anypass = false;
	mercury_int passed = 0;
	mercury_int req_pas = 0;
	for (mercury_int n = 0; n < num_patterns; n++) {
		if ((patterns[n].match_type & MATCH_INVERTED) ? (!patterns[n].passed) : patterns[n].passed) {
			if (!(patterns[n].match_type & MATCH_OPTIONAL)) {
				passed++;
			}
			anypass = true;
		}
		if (!(patterns[n].match_type & MATCH_OPTIONAL)) {
			req_pas++;
		}
	}
	if (anypass && passed >= req_pas) {
		return true;
	}
	else {
		goto advance;
	}

}



void mercury_lib_string_p_find(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 3; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* startat = nullptr;
	if (args_in > 2) {
		startat = mercury_popstack(M);
		if (startat->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)startat->type, (void*)M_TYPE_INT,(void*)3 );
			return;
		}
	}

	mercury_variable* searchforvar = mercury_popstack(M);
	if (searchforvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)searchforvar->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1 );
		return;
	}


	mercury_stringliteral* str = (mercury_stringliteral*)strvar->data.p;
	mercury_stringliteral* pat_str = (mercury_stringliteral*)searchforvar->data.p;

	mercury_int num_pats = 0;
	M_PATTERN* P = m_patternize_string(pat_str,&num_pats);


	mercury_int start = 0;
	mercury_int end = 0;
	bool found=m_evaluate_patterns(str,P,num_pats, startat ? startat->data.i : 0,&start,&end);

	free(P);
	if(startat)mercury_unassign_var(M,startat);

	if (args_out < 2) {
		mercury_unassign_var(M, strvar);
	}
	else {
		mercury_free_var(strvar, true);
	}

	mercury_free_var(searchforvar, true);

	if (found) {
		searchforvar->type = M_TYPE_INT;
		searchforvar->data.i = start;
		strvar->type = M_TYPE_INT;
		strvar->data.i = end;
	}
	else {
		searchforvar->type = M_TYPE_NIL;
		searchforvar->data.i = 0;
		strvar->type = M_TYPE_NIL;
		strvar->data.i = 0;
	}

	mercury_pushstack(M, searchforvar);
	if (args_out > 1) {
		mercury_pushstack(M, strvar);
	}

	for (mercury_int a = 2; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_p_extract(mercury_state* M, mercury_int args_in, mercury_int args_out) { //returns all instances of a match in an array.
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* matchvar = mercury_popstack(M);
	if (matchvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)matchvar->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* str = (mercury_stringliteral*)strvar->data.p;
	mercury_stringliteral* match = (mercury_stringliteral*)matchvar->data.p;

	mercury_int num_pats = 0;
	M_PATTERN* P = m_patternize_string(match, &num_pats);
	mercury_variable* out = mercury_assign_var(M);

	mercury_array* arr=mercury_newarray();
	out->type = M_TYPE_ARRAY;
	out->data.p = arr;

	mercury_int count = 0;
	mercury_int start = 0;
	mercury_int end = 0;
	while (m_evaluate_patterns(str, P, num_pats, end, &start, &end)) {
		mercury_stringliteral* ss=mercury_mstring_substring(str,start,end);
		if (!ss) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_unassign_var(M,strvar);
			mercury_unassign_var(M,out);
			return;
		}
		mercury_variable* inter = mercury_assign_var(M);
		if (!inter) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_unassign_var(M, strvar);
			mercury_unassign_var(M, out);
			return;
		}
		inter->type = M_TYPE_STRING;
		inter->data.p = ss;
		mercury_setarray(arr, inter, count, M);
		count++;
		end++;
	}

	mercury_unassign_var(M, strvar);
	mercury_unassign_var(M, matchvar);

	mercury_pushstack(M,out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_p_replace(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 3) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)3);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 4; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* maxcount = nullptr;
	if (args_in >= 4) {
		maxcount = mercury_popstack(M);
		if (maxcount->type != M_TYPE_INT) {
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)maxcount->type, (void*)M_TYPE_INT, (void*)4);
			return;
		}
	}

	mercury_variable* repvar = mercury_popstack(M);
	if (repvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)repvar->type, (void*)M_TYPE_STRING,(void*)3);
		return;
	}

	mercury_variable* matchvar = mercury_popstack(M);
	if (matchvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)matchvar->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* str = (mercury_stringliteral*)strvar->data.p;
	mercury_stringliteral* match = (mercury_stringliteral*)matchvar->data.p;
	mercury_stringliteral* rep = (mercury_stringliteral*)repvar->data.p;

	mercury_int num_pats = 0;
	M_PATTERN* P = m_patternize_string(match, &num_pats);
	mercury_free_var(matchvar, true);
	mercury_variable* out = matchvar;

	mercury_stringliteral* outstr = mercury_cstring_to_mstring((char*)"", 0);
	out->type = M_TYPE_STRING;
	out->data.p = outstr;

	mercury_int count = 0;

	mercury_int start = 0;
	mercury_int end = 0;
	mercury_int lastendmatch = 0;
	while (m_evaluate_patterns(str, P, num_pats, end, &start, &end)) {
		if (maxcount && count >= maxcount->data.i)break;

		//printf("%i=%i\n", start, end);

		mercury_stringliteral* app = mercury_mstring_substring(str, lastendmatch, start - 1);
		if (!app) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		mercury_mstrings_append(outstr, app);
		mercury_mstrings_append(outstr, rep);
	
		
		end++;
		lastendmatch = end;
		count++;
	}
	mercury_mstrings_append(outstr, mercury_mstring_substring(str,lastendmatch,str->size-1));

	mercury_unassign_var(M,repvar);
	mercury_unassign_var(M,strvar);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_p_count(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* matchvar = mercury_popstack(M);
	if (matchvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)matchvar->type, (void*)M_TYPE_STRING, (void*)2);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_stringliteral* str = (mercury_stringliteral*)strvar->data.p;
	mercury_stringliteral* match = (mercury_stringliteral*)matchvar->data.p;

	mercury_int num_pats = 0;
	M_PATTERN* P = m_patternize_string(match, &num_pats);
	mercury_free_var(matchvar, true);
	mercury_variable* out = matchvar;

	mercury_stringliteral* outstr = mercury_cstring_to_mstring((char*)"", 0);
	out->type = M_TYPE_STRING;
	out->data.p = outstr;

	mercury_int count = 0;

	mercury_int start = 0;
	mercury_int end = 0;
	while (m_evaluate_patterns(str, P, num_pats, end, &start, &end)) {
		//printf("%i=%i\n", start, end);

		end++;
		count++;
	}

	mercury_unassign_var(M, strvar);

	out->type = M_TYPE_INT;
	out->data.i = count;

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_escape_mercury(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* instr = mercury_popstack(M);
	if (instr->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)instr->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_variable* outvar=mercury_assign_var(M);
	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outvar || !os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	outvar->type = M_TYPE_STRING;
	outvar->data.p = os;

	os->size = 0;
	os->ptr = nullptr;
	os->constant = false;

	mercury_stringliteral* is = (mercury_stringliteral*)instr->data.p;
	mercury_int pos = 0;
	while (pos < is->size) {
		char c = is->ptr[pos];
		switch (c) {
		case '\"':
			mercury_mstring_addchars(os, (char*)"\\\"", 2);
			break;
		case '\\':
			mercury_mstring_addchars(os, (char*)"\\\\", 2);
			break;
		default:
			mercury_mstring_addchars(os, is->ptr + pos, 1);
		}
		
		pos++;
	}

	mercury_unassign_var(M, instr);
	mercury_pushstack(M, outvar);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_escape_url(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* instr = mercury_popstack(M);
	if (instr->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)instr->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_variable* outvar = mercury_assign_var(M);
	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outvar || !os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	outvar->type = M_TYPE_STRING;
	outvar->data.p = os;

	os->size = 0;
	os->ptr = nullptr;
	os->constant = false;

	mercury_stringliteral* is = (mercury_stringliteral*)instr->data.p;
	mercury_int pos = 0;
	while (pos < is->size) {
		char c = is->ptr[pos];


		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c=='-' || c == '_' || c == '~' || c == '.') {
			mercury_mstring_addchars(os, is->ptr + pos, 1);
		}
		else { //need to percent encode these.
			mercury_mstring_addchars(os, (char*)"%" , 1);
			char sc = (c & 0xF0)>>4;
			if (sc<0xA) {
				char n = sc+0x30;
				mercury_mstring_addchars(os, &n, 1);
			}
			else {
				char n = sc + 0x41-0x0A;
				mercury_mstring_addchars(os, &n, 1);
			}
			c &= 0xF;
			if (c < 0xA) {
				char n = c + 0x30;
				mercury_mstring_addchars(os, &n, 1);
			}
			else {
				char n = c + 0x41 - 0x0A;
				mercury_mstring_addchars(os, &n, 1);
			}

		}

		pos++;
	}

	mercury_unassign_var(M, instr);
	mercury_pushstack(M, outvar);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_string_escape_c(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* instr = mercury_popstack(M);
	if (instr->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)instr->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_variable* outvar = mercury_assign_var(M);
	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outvar || !os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	outvar->type = M_TYPE_STRING;
	outvar->data.p = os;

	os->size = 0;
	os->ptr = nullptr;
	os->constant = false;

	mercury_stringliteral* is = (mercury_stringliteral*)instr->data.p;
	mercury_int pos = 0;
	while (pos < is->size) {
		char c = is->ptr[pos];
		switch (c) {
		case '\a':
			mercury_mstring_addchars(os, (char*)"\\a", 2);
			break;
		case '\b':
			mercury_mstring_addchars(os, (char*)"\\b", 2);
			break;
		case '\e':
			mercury_mstring_addchars(os, (char*)"\\e", 2);
			break;
		case '\f':
			mercury_mstring_addchars(os, (char*)"\\f", 2);
			break;
		case '\n':
			mercury_mstring_addchars(os, (char*)"\\n", 2);
			break;
		case '\r':
			mercury_mstring_addchars(os, (char*)"\\r", 2);
			break;
		case '\t':
			mercury_mstring_addchars(os, (char*)"\\t", 2);
			break;
		case '\v':
			mercury_mstring_addchars(os, (char*)"\\v", 2);
			break;
		case '\\':
			mercury_mstring_addchars(os, (char*)"\\\\", 2);
			break;
		case '\'':
			mercury_mstring_addchars(os, (char*)"\\'", 2);
			break;
		case '\"':
			mercury_mstring_addchars(os, (char*)"\\\"", 2);
			break;
		case '\?':
			mercury_mstring_addchars(os, (char*)"\\?", 2);
			break;
		case '\0':
			mercury_mstring_addchars(os, (char*)"\\0", 2);
			break;
		default:
			mercury_mstring_addchars(os, is->ptr + pos, 1);
		}

		pos++;
	}

	mercury_unassign_var(M, instr);
	mercury_pushstack(M, outvar);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_string_escape_html(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out) {
		return;
	}
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* instr = mercury_popstack(M);
	if (instr->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)instr->type, (void*)M_TYPE_STRING, (void*)1);
		return;
	}

	mercury_variable* outvar = mercury_assign_var(M);
	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outvar || !os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	outvar->type = M_TYPE_STRING;
	outvar->data.p = os;

	os->size = 0;
	os->ptr = nullptr;
	os->constant = false;

	mercury_stringliteral* is = (mercury_stringliteral*)instr->data.p;
	mercury_int pos = 0;
	while (pos < is->size) {
		char c = is->ptr[pos];
		switch (c) {
		case '<':
			mercury_mstring_addchars(os, (char*)"&lt;", 4);
			break;
		case '>':
			mercury_mstring_addchars(os, (char*)"&gt;", 4);
			break;
		case '&':
			mercury_mstring_addchars(os, (char*)"&amp;", 5);
			break;
		case '\"':
			mercury_mstring_addchars(os, (char*)"&quot;", 6);
			break;
		case '\'':
			mercury_mstring_addchars(os, (char*)"&#39;", 6);
			break;
		default:
			mercury_mstring_addchars(os, is->ptr + pos, 1);
		}

		pos++;
	}

	mercury_unassign_var(M, instr);
	mercury_pushstack(M, outvar);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}
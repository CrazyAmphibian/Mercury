#include "mercury_lib_string.h"
#include "../mercury.h"
#include "../mercury_error.h"

#include "malloc.h"
#ifndef _WIN32
#include <cstring.h>
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_end->type, (void*)M_TYPE_INT);
		return;
	}
	mercury_variable* var_start = mercury_popstack(M);
	if (var_start->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_start->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING);
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
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)startat->type, (void*)M_TYPE_INT);
			return;
		}
	}

	mercury_variable* searchforvar = mercury_popstack(M);
	if (searchforvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)searchforvar->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING);
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
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)max_replacments->type, (void*)M_TYPE_INT);
			return;
		}
	}
	mercury_variable* replace_var = mercury_popstack(M);
	if (replace_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)replace_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* search_var = mercury_popstack(M);
	if (search_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)search_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_ARRAY);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)sep_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)var_string->type, (void*)M_TYPE_STRING);
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


// TODO: add a, x, g, e, p, u, o
int m_readformat(mercury_stringliteral* str, mercury_int offset, mercury_stringliteral* str_out, mercury_variable** v_arr, mercury_int* num_vars) {
	mercury_int add_off = 0;

	unsigned char nargs = 0;
	bool args_def[256] = { false }; //if the arg is defined.
	mercury_int args[256] = {0}; // the value of that arg.
	char mode = 0;

	char printfbuffer[1024] = {'0'};

	const char* type=nullptr;
	mercury_int value = 0;

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
					value = mercury_checkint(v_arr[*num_vars]);
				}		
			}
		case 'x': // hex int
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llx";
#else
				type = "x";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value = mercury_checkint(v_arr[*num_vars]);
				}
			}
		case 'X': // hex int (capital)
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llX";
#else
				type = "X";
#endif
				if (*num_vars) {
					(*num_vars)--;
					value = mercury_checkint(v_arr[*num_vars]);
				}
			}
		case 'f':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llf";
#else
				type = "f";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'F':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llF";
#else
				type = "F";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'e':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "lle";
#else
				type = "e";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'E':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llE";
#else
				type = "E";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'g':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llg";
#else
				type = "g";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'G':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llG";
#else
				type = "G";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'a':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "lla";
#else
				type = "a";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'A':
			if (!type) {
#ifdef MERCURY_64BIT
				type = "llA";
#else
				type = "A";
#endif
				if (*num_vars) {
					(*num_vars)--;
					mercury_float f = mercury_checkfloat(v_arr[*num_vars]);
					value = *(mercury_int*)&f;
				}
			}
		case 'p':
		case 'P':
			if (!type) {
				type = "p";
				if (*num_vars) {
					(*num_vars)--;
					void* p = mercury_checkpointer(v_arr[*num_vars]);
					value = (mercury_int)p;
				}
			}

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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)strvar->type, (void*)M_TYPE_STRING);
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



#include "mercury_lib_string.h"
#include "../mercury.h"
#include "../mercury_error.h"

#include "malloc.h"

void mercury_lib_string_sub(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 3) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_end->type, (void*)M_TYPE_INT);
		return;
	}
	mercury_variable* var_start = mercury_popstack(M);
	if (var_start->type != M_TYPE_INT) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_start->type, (void*)M_TYPE_INT);
		return;
	}

	mercury_variable* var_string = mercury_popstack(M);
	if (var_string->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_string->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_string->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char)*str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)2);
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
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)startat->type, (void*)M_TYPE_INT);
			return;
		}
	}

	mercury_variable* searchforvar = mercury_popstack(M);
	if (searchforvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)searchforvar->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* strvar = mercury_popstack(M);
	if (strvar->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)strvar->type, (void*)M_TYPE_STRING);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
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
			mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)max_replacments->type, (void*)M_TYPE_INT);
			return;
		}
	}
	mercury_variable* replace_var = mercury_popstack(M);
	if (replace_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)replace_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* search_var = mercury_popstack(M);
	if (search_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)search_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)str_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_stringliteral* intermediate=nullptr;

	mercury_stringliteral* outstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!outstr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		intermediate=mercury_mstrings_concat(outstr,replace);
		mercury_mstring_delete(outstr);
		outstr = intermediate;
		c += search->size;
		continue;
		next:
		//string not found.
		intermediate=mercury_mstrings_concat(outstr, mercury_cstring_to_mstring(str->ptr+c, 1));
		mercury_mstring_delete(outstr);
		outstr = intermediate;
		c++;
	}

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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)str_var->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)str_var->data.p;

	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)str_var->type, (void*)M_TYPE_ARRAY);
		return;
	}
	mercury_array* arr = (mercury_array*)str_var->data.p;

	mercury_int len=mercury_array_len(arr);

	mercury_stringliteral* st = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral*));
	if (!st) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}
	st->ptr=(char*)malloc(len * sizeof(char));
	if (!st->ptr) {
		free(st);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)3);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)sep_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)str_var->type, (void*)M_TYPE_STRING);
		return;
	}


	mercury_array* arr = mercury_newarray();
	if (!arr) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_string->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char) * str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)M->programcounter, (void*)args_in, (void*)1);
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
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M->programcounter, (void*)var_string->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)var_string->data.p;

	mercury_stringliteral* os = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!os) {
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
		return;
	}
	os->ptr = (char*)malloc(sizeof(char) * str->size);
	if (!os->ptr) {
		free(os);
		mercury_raise_error(M, M_ERROR_ALLOCATION, (void*)M->programcounter);
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

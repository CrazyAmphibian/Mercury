#include "../mercury.hpp"
#include "../mercury_error.hpp"

#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void mercury_lib_os_time(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //gets the current unix/epoch time.
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

void mercury_lib_os_execute(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //dangerous!
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


void mercury_lib_os_call(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //dangerous!
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


void mercury_lib_os_clock(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //gets the time since program startup.
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


void mercury_lib_os_getdate(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //returns a date in table form, from epoch time.
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)){
		return;
	}
	if (!args_out)return;

	mercury_variable* tvar=mercury_pullstack(M);
	time_t t;
	switch (tvar->type) {
	case M_TYPE_INT:
		t = tvar->data.i;
		break;
	case M_TYPE_FLOAT:
		t = tvar->data.f;
		break;
	default:
		mercury_raise_error(M,M_ERROR_WRONG_TYPE,(void*)tvar->type,(void*)M_TYPE_INT);
		return;
	}

	tm timedata;
#ifdef WIN32
	gmtime_s(&timedata,&t);
#else
	gmtime_r(&t, &timedata);
#endif
	mercury_table* outt=mercury_newtable();
	if (!outt) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	mercury_variable* outv=mercury_assign_var(M);
	if (!outv) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		mercury_destroytable(outt);
		return;
	}

	mercury_variable* kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_sec;
	mercury_table_set_cstring_keyvalue(outt, "seconds", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_min;
	mercury_table_set_cstring_keyvalue(outt, "minutes", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_hour+1;
	mercury_table_set_cstring_keyvalue(outt, "hours", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = (timedata.tm_hour%12)+1;
	mercury_table_set_cstring_keyvalue(outt, "hours12", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_BOOL;
	kvar->data.i = timedata.tm_hour >= 11;
	mercury_table_set_cstring_keyvalue(outt, "ispm", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_year+1900;
	mercury_table_set_cstring_keyvalue(outt, "year", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_mon + 1;
	mercury_table_set_cstring_keyvalue(outt, "month", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_wday;
	mercury_table_set_cstring_keyvalue(outt, "dayofweek", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_mday;
	mercury_table_set_cstring_keyvalue(outt, "dayofmonth", kvar, M);

	kvar = mercury_assign_var(M);
	kvar->type = M_TYPE_INT;
	kvar->data.i = timedata.tm_yday+1;
	mercury_table_set_cstring_keyvalue(outt, "dayofyear", kvar, M);

	outv->type = M_TYPE_TABLE;
	outv->data.p = outt;
	mercury_pushstack(M, outv);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_os_gettime(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //like above, but returns epoch time from date information.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out)return;

	mercury_variable* tvar = mercury_pullstack(M);
	if(tvar->type!=M_TYPE_TABLE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)tvar->type, (void*)M_TYPE_TABLE);
		return;
	}
	tm timedata;
	memset(&timedata, 0, sizeof(tm));

	mercury_table* tab = (mercury_table*)tvar->data.p;
	mercury_variable* var;

	timedata.tm_sec= mercury_checkint(mercury_table_get_cstring_keyvalue(tab, "seconds", M));
	timedata.tm_min= mercury_checkint(mercury_table_get_cstring_keyvalue(tab, "minutes", M));
	
	var = mercury_table_get_cstring_keyvalue(tab, "hours", M);
	if (var->type == M_TYPE_INT || var->type == M_TYPE_FLOAT) {
		timedata.tm_hour = mercury_checkint(var) - 1;
	}
	else {
		timedata.tm_hour = 0;
	}
	mercury_unassign_var(M, var);

	var=mercury_table_get_cstring_keyvalue(tab, "year", M);
	if (var->type == M_TYPE_INT || var->type == M_TYPE_FLOAT) {
		timedata.tm_year = mercury_checkint(var)- 1900;
	}
	else {
		timedata.tm_year = 0;
	}
	mercury_unassign_var(M, var);

	var = mercury_table_get_cstring_keyvalue(tab, "month", M);
	if (var->type == M_TYPE_INT || var->type == M_TYPE_FLOAT) {
		timedata.tm_mon = mercury_checkint(var) - 1;
	}
	else {
		timedata.tm_mon = 0;
	}
	mercury_unassign_var(M, var);

	timedata.tm_mday = mercury_checkint(mercury_table_get_cstring_keyvalue(tab, "dayofmonth", M));
	timedata.tm_isdst = mercury_checkint(mercury_table_get_cstring_keyvalue(tab, "daylightsavings", M));

	mercury_variable* out= mercury_assign_var(M);
	if (!out) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	out->type = M_TYPE_INT;
#ifdef WIN32
	out->data.i= _mkgmtime(&timedata); //why in the world does mktime use your local timezone? bad. stupid. uuuuuugh.
#else
	out->data.i = timegm(&timedata);
#endif
	mercury_pushstack(M, out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

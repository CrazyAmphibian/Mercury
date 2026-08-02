#include "../mercury.hpp"
#include "../mercury_error.hpp"

#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void mercury_lib_os_time(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //gets the current unix/epoch time.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0))return;
	if (!args_out) {
		return;
	}

	mercury_variable out;

	mercury_int t=time(NULL);
	out.type = M_TYPE_INT;
	out.data.i = t;
	

	mercury_pushstack(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_os_execute(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //dangerous!
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable cvar;
	mercury_popstack(M,&cvar);
	if (cvar.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, cvar.type, M_TYPE_STRING, 1);
		return;
	}
	mercury_string* code = (mercury_string*)cvar.data.p;

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

	mercury_free_var(&cvar);
	cvar.type = M_TYPE_STRING;
	cvar.data.p= mercury_cstring_const_to_mstring(out_c, size_c);
	mercury_pushstack(M, &cvar);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_os_call(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //dangerous!
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable cvar;
	mercury_popstack(M, &cvar);
	if (cvar.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, cvar.type, M_TYPE_STRING, 1);
		return;
	}
	mercury_string* code = (mercury_string*)cvar.data.p;

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


	mercury_free_var(&cvar);
	cvar.type = M_TYPE_INT;
	cvar.data.i = r;
	mercury_pushstack(M, &cvar);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_os_clock(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //gets the time since program startup.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0))return;
	if (!args_out) {
		return;
	}

	mercury_variable out;

	mercury_float t = ((mercury_float)clock()) / CLOCKS_PER_SEC;
	out.type = M_TYPE_FLOAT;
	out.data.f = t;
	

	mercury_pushstack(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_os_getdate(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //returns a date in table form, from epoch time.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable tvar;
	mercury_popstack(M,&tvar);
	time_t t;
	switch (tvar.type) {
	case M_TYPE_INT:
		t = (time_t)tvar.data.i;
		break;
	case M_TYPE_FLOAT:
		t = (time_t)tvar.data.f;
		break;
	default:
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, tvar.type, M_TYPE_INT, 1);
		return;
	}
	mercury_free_var(&tvar);

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
	


	mercury_variable kvar;
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_sec;
	mercury_table_set_cstring_keyvalue(outt, "seconds", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_min;
	mercury_table_set_cstring_keyvalue(outt, "minutes", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_hour+1;
	mercury_table_set_cstring_keyvalue(outt, "hours", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = (timedata.tm_hour%12)+1;
	mercury_table_set_cstring_keyvalue(outt, "hours12", &kvar);

	
	kvar.type = M_TYPE_BOOL;
	kvar.data.i = timedata.tm_hour >= 11;
	mercury_table_set_cstring_keyvalue(outt, "ispm", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_year+1900;
	mercury_table_set_cstring_keyvalue(outt, "year", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_mon + 1;
	mercury_table_set_cstring_keyvalue(outt, "month", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_wday;
	mercury_table_set_cstring_keyvalue(outt, "dayofweek", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_mday;
	mercury_table_set_cstring_keyvalue(outt, "dayofmonth", &kvar);

	
	kvar.type = M_TYPE_INT;
	kvar.data.i = timedata.tm_yday+1;
	mercury_table_set_cstring_keyvalue(outt, "dayofyear", &kvar);

	mercury_variable outv;
	outv.type = M_TYPE_TABLE;
	outv.data.p = outt;
	mercury_pushstack(M, &outv);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_os_gettime(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //like above, but returns epoch time from date information.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable tvar;
	mercury_popstack(M,&tvar);

	if(tvar.type!=M_TYPE_TABLE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, tvar.type, M_TYPE_TABLE, 1);
		return;
	}
	tm timedata;
	memset(&timedata, 0, sizeof(tm));

	mercury_table* tab = (mercury_table*)tvar.data.p;

	mercury_variable var;

	mercury_table_get_cstring_keyvalue(tab, "seconds", &var);
	timedata.tm_sec= (int)mercury_checkint(&var);
	mercury_free_var(&var);

	mercury_table_get_cstring_keyvalue(tab, "minutes", &var);
	timedata.tm_min= (int)mercury_checkint(&var);
	mercury_free_var(&var);
	
	mercury_table_get_cstring_keyvalue(tab, "hours", &var);
	if (var.type == M_TYPE_INT || var.type == M_TYPE_FLOAT) {
		timedata.tm_hour = (int)mercury_checkint(&var) - 1;
	}
	else {
		timedata.tm_hour = 0;
	}
	mercury_free_var(&var);

	mercury_table_get_cstring_keyvalue(tab, "year", &var);
	if (var.type == M_TYPE_INT || var.type == M_TYPE_FLOAT) {
		timedata.tm_year = (int)mercury_checkint(&var)- 1900;
	}
	else {
		timedata.tm_year = 0;
	}
	mercury_free_var(&var);

	mercury_table_get_cstring_keyvalue(tab, "month", &var);
	if (var.type == M_TYPE_INT || var.type == M_TYPE_FLOAT) {
		timedata.tm_mon = (int)mercury_checkint(&var) - 1;
	}
	else {
		timedata.tm_mon = 0;
	}
	mercury_free_var(&var);

	mercury_table_get_cstring_keyvalue(tab, "dayofmonth", &var);
	timedata.tm_mday = (int)mercury_checkint(&var);
	mercury_free_var(&var);
	
	mercury_table_get_cstring_keyvalue(tab, "daylightsavings", &var);
	timedata.tm_isdst = (int)mercury_checkint(&var);
	mercury_free_var(&var);

	mercury_variable out;
	
	out.type = M_TYPE_INT;
#ifdef WIN32
	out.data.i= _mkgmtime(&timedata); //why in the world does mktime use your local timezone? bad. stupid. uuuuuugh.
#else
	out.data.i = timegm(&timedata);
#endif
	mercury_pushstack(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



void mercury_lib_os_exit(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0,1)) {
		return;
	}

	int exitcode = 0;
	if (args_in) {
		mercury_variable v;
		mercury_popstack(M,&v);
		if (v.type == M_TYPE_INT) {
			exitcode = (int)v.data.i;
		}

		mercury_free_var(&v);
	}

	exit(exitcode);
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
}
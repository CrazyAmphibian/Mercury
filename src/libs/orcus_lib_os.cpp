#include "../orcus.hpp"
#include "../orcus_error.hpp"

#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void orcus_lib_os_time(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //gets the current unix/epoch time.
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0))return;
	if (!args_out) {
		return;
	}

	orcus_variable out;

	orcus_int t=time(NULL);
	out.type = ORCUS_TYPE_INT;
	out.data.i = t;
	

	orcus_pushstack(M, &out);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void orcus_lib_os_execute(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //dangerous!
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	orcus_variable cvar;
	orcus_popstack(M,&cvar);
	if (cvar.type != ORCUS_TYPE_STRING) {
		orcus_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &cvar, ORCUS_TYPE_STRING, 1);
		orcus_release_var(&cvar);
		return;
	}
	orcus_string* code = (orcus_string*)cvar.data.p;

	char* c_code = (char*)malloc(sizeof(char) * (code->size + 1));
	if (!c_code) {
		orcus_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (orcus_int i = 0; i < code->size; i++) {
		c_code[i] = code->ptr[i];
	}
	c_code[code->size] = '\0'; //end will null for c string compatability.

	//system(c_code);
#ifdef _WIN32
	FILE* a = _popen(c_code, "r");
#else
	FILE* a = popen(c_code, "r");
#endif 
	free(c_code);
	char* out_c = nullptr;
	orcus_int size_c = 0;
	orcus_int allocated_c = 0;

	if (a) {
		int c = 0;
		while (1) {
			c = fgetc(a);
			if (c == EOF)break;


			if (size_c == allocated_c) {
				void* o = realloc(out_c, allocated_c + 128);
				if (!o) {
					orcus_raise_error(M, M_ERROR_ALLOCATION);
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
		for (orcus_int i = 0; i < size_c; i++) {
			putchar(out_c[i]);
		}
		free(out_c);
		return;
	}

	orcus_release_var(&cvar);
	cvar.type = ORCUS_TYPE_STRING;
	cvar.data.p= orcus_cstring_const_to_mstring(out_c, size_c);
	orcus_pushstack(M, &cvar);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void orcus_lib_os_call(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //dangerous!
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	orcus_variable cvar;
	orcus_popstack(M, &cvar);
	if (cvar.type != ORCUS_TYPE_STRING) {
		orcus_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &cvar, ORCUS_TYPE_STRING, 1);
		orcus_release_var(&cvar);
		return;
	}
	orcus_string* code = (orcus_string*)cvar.data.p;

	char* c_code = (char*)malloc(sizeof(char) * (code->size + 1));
	if (!c_code) {
		orcus_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (orcus_int i = 0; i < code->size; i++) {
		c_code[i] = code->ptr[i];
	}
	c_code[code->size] = '\0'; //end will null for c string compatability.

	//system(c_code);

	int r=system(c_code);
	free(c_code);
	if (!args_out) {
		return;
	}


	orcus_release_var(&cvar);
	cvar.type = ORCUS_TYPE_INT;
	cvar.data.i = r;
	orcus_pushstack(M, &cvar);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void orcus_lib_os_clock(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //gets the time since program startup.
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0))return;
	if (!args_out) {
		return;
	}

	orcus_variable out;

	orcus_float t = ((orcus_float)clock()) / CLOCKS_PER_SEC;
	out.type = ORCUS_TYPE_FLOAT;
	out.data.f = t;
	

	orcus_pushstack(M, &out);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void orcus_lib_os_getdate(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //returns a date in table form, from epoch time.
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		orcus_discard_top_of_stack(M);
		return;
	}

	orcus_variable tvar;
	orcus_popstack(M,&tvar);
	time_t t;
	switch (tvar.type) {
	case ORCUS_TYPE_INT:
		t = (time_t)tvar.data.i;
		break;
	case ORCUS_TYPE_FLOAT:
		t = (time_t)tvar.data.f;
		break;
	default:
		orcus_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tvar, ORCUS_TYPE_INT, 1);
		orcus_release_var(&tvar);
		return;
	}
	

	tm timedata;
#ifdef WIN32
	gmtime_s(&timedata,&t);
#else
	gmtime_r(&t, &timedata);
#endif
	orcus_table* outt=orcus_newtable();
	if (!outt) {
		orcus_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	


	orcus_variable kvar;
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = timedata.tm_sec;
	orcus_table_set_cstring_keyvalue(outt, "seconds", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = timedata.tm_min;
	orcus_table_set_cstring_keyvalue(outt, "minutes", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = timedata.tm_hour;
	orcus_table_set_cstring_keyvalue(outt, "hours", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = ((timedata.tm_hour%12)+1);
	orcus_table_set_cstring_keyvalue(outt, "hours12", &kvar);

	
	kvar.type = ORCUS_TYPE_BOOL;
	kvar.data.i = timedata.tm_hour >= 11;
	orcus_table_set_cstring_keyvalue(outt, "ispm", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = (timedata.tm_year+1900);
	orcus_table_set_cstring_keyvalue(outt, "year", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = (timedata.tm_mon + 1);
	orcus_table_set_cstring_keyvalue(outt, "month", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = timedata.tm_wday;
	orcus_table_set_cstring_keyvalue(outt, "dayofweek", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = timedata.tm_mday;
	orcus_table_set_cstring_keyvalue(outt, "dayofmonth", &kvar);

	
	kvar.type = ORCUS_TYPE_INT;
	kvar.data.i = (timedata.tm_yday+1);
	orcus_table_set_cstring_keyvalue(outt, "dayofyear", &kvar);

	orcus_variable outv;
	outv.type = ORCUS_TYPE_TABLE;
	outv.data.p = outt;
	orcus_pushstack(M, &outv);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void orcus_lib_os_gettime(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) { //like above, but returns epoch time from date information.
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		orcus_discard_top_of_stack(M);
		return;
	}

	orcus_variable tvar;
	orcus_popstack(M,&tvar);

	if(tvar.type!=ORCUS_TYPE_TABLE) {
		orcus_raise_error_firstargpointeronly(M, M_ERROR_WRONG_TYPE_VARIABLEPROVIDED, &tvar, ORCUS_TYPE_TABLE, 1);
		return;
	}
	tm timedata;
	memset(&timedata, 0, sizeof(tm));

	orcus_table* tab = (orcus_table*)tvar.data.p;

	orcus_variable var;

	orcus_table_get_cstring_keyvalue(tab, "seconds", &var);
	timedata.tm_sec= (int)orcus_checkint(&var);

	orcus_table_get_cstring_keyvalue(tab, "minutes", &var);
	timedata.tm_min= (int)orcus_checkint(&var);
	
	orcus_table_get_cstring_keyvalue(tab, "hours", &var);
	if (var.type == ORCUS_TYPE_INT || var.type == ORCUS_TYPE_FLOAT) {
		timedata.tm_hour = (int)orcus_checkint(&var);
	}
	else {
		timedata.tm_hour = 0;
	}

	orcus_table_get_cstring_keyvalue(tab, "year", &var);
	if (var.type == ORCUS_TYPE_INT || var.type == ORCUS_TYPE_FLOAT) {
		timedata.tm_year = (int)orcus_checkint(&var)- 1900;
	}
	else {
		timedata.tm_year = 0;
	}

	orcus_table_get_cstring_keyvalue(tab, "month", &var);
	if (var.type == ORCUS_TYPE_INT || var.type == ORCUS_TYPE_FLOAT) {
		timedata.tm_mon = (int)orcus_checkint(&var) - 1;
	}
	else {
		timedata.tm_mon = 0;
	}

	orcus_table_get_cstring_keyvalue(tab, "dayofmonth", &var);
	timedata.tm_mday = (int)orcus_checkint(&var);
	
	orcus_table_get_cstring_keyvalue(tab, "daylightsavings", &var);
	timedata.tm_isdst = (int)orcus_checkint(&var);

	orcus_variable out;
	
	out.type = ORCUS_TYPE_INT;
#ifdef WIN32
	out.data.i= _mkgmtime(&timedata); //why in the world does mktime use your local timezone? bad. stupid. uuuuuugh.
#else
	out.data.i = timegm(&timedata);
#endif
	orcus_pushstack(M, &out);

	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



void orcus_lib_os_exit(orcus_state* const ORCUS_CPP_restrict M, const orcus_int args_in, const orcus_int args_out) {
	if (ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0,1)) {
		return;
	}

	int exitcode = 0;
	if (args_in) {
		orcus_variable v;
		orcus_popstack(M,&v);
		if (v.type == ORCUS_TYPE_INT) {
			exitcode = (int)v.data.i;
		}

		orcus_release_var(&v);
	}

	exit(exitcode);
	ORCUS_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
}
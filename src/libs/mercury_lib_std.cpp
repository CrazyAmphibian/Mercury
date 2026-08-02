#include"../mercury.hpp"
#include"../mercury_error.hpp"
#include"../mercury_bytecode.hpp"

#include<malloc.h>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

//throws stuff into stdout. adds a newline at the end, seperates with a tab. designed to be variadic.
void mercury_lib_std_print(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {

	mercury_variable* vartable = (mercury_variable*)malloc(sizeof(mercury_variable) * args_in);
	if (vartable == nullptr && args_in) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int a = args_in-1; a >= 0; a--) {
		mercury_popstack(M, vartable+a);
	}

	for (mercury_int a = 0; a < args_in; a++) {

		mercury_string* mstrv = mercury_tostring(vartable+a);
		mercury_free_var(vartable+a);
		if (mstrv) {
			for (mercury_int c = 0; c < mstrv->size; c++) {
				putchar(mstrv->ptr[c]);
			}
			putchar('\t');
			fflush(stdout);

			mercury_mstring_delete(mstrv);
		}
		
		
	}
	free(vartable);
	putchar('\n');
	fflush(stdout);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, 0);
}


//traverses a list-like variable entry-by-entry. useful for serializing tables, most likley. arg 1 is the thing, and arg 2 is the function.
void mercury_lib_std_iterate(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2))return;

	mercury_variable function;
	mercury_popstack(M,&function);
	mercury_variable listlike;
	mercury_popstack(M,&listlike);


	if (function.type != M_TYPE_CFUNC && function.type != M_TYPE_FUNCTION) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION, function.type, M_TYPE_TABLE, 2);
		return;
	}
	if (listlike.type != M_TYPE_TABLE && listlike.type != M_TYPE_ARRAY) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_STORAGETYPE, listlike.type, M_TYPE_TABLE, 1);
		return;
	}

	mercury_state* SubM = mercury_get_child_state(M);
	if (!SubM) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	mercury_function previous = SubM->bytecode;

	
	if (function.type == M_TYPE_FUNCTION) {
		SubM->bytecode = *((mercury_function*)function.data.p);
	}

	if (listlike.type == M_TYPE_ARRAY) {
		mercury_array* arr = (mercury_array*)listlike.data.p;
		mercury_int srefs = arr->refrences;


		if (arr->values) {
#ifdef MERCURY_64BIT
			for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
				mercury_variable***** const st1 = arr->values[i1];
				if (!st1)continue;
				for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
					mercury_variable**** const st2 = st1[i2];
					if (!st2)continue;
					for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
						mercury_variable*** const st3 = st2[i3];
						if (!st3)continue;
						for (int i4 = 0; i4 < MERCURY_SIZE_SUBARRAY_4; i4++) {
							mercury_variable** const st4 = st3[i4];
							if (!st4)continue;
							for (int i5 = 0; i5 < MERCURY_SIZE_SUBARRAY_5; i5++) {
								mercury_variable* const st5 = st4[i5];
								if (!st5)continue;
								for (int i6 = 0; i6 < MERCURY_SIZE_SUBARRAY_6; i6++) {
									mercury_variable var = st5[i6];
									const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3, i4, i5, i6);
#else
			for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
				mercury_variable** const st1 = arr->values[i1];
				if (!st1)continue;
				for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
					mercury_variable* const st2 = st1[i2];
					if (!st2)continue;
					for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
						mercury_variable var = st2[i3];
						const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3);
#endif
						if (var.type) {
							mercury_variable v;
							mercury_clonevariable(&var,&v);

							mercury_variable idxvar;
							idxvar.data.i = index;
							idxvar.type = M_TYPE_INT;

							if (function.type == M_TYPE_CFUNC) {
								mercury_pushstack(SubM, &idxvar);
								mercury_pushstack_unrefed(SubM, &v);
								mercury_pushstack(SubM, &listlike);
								((mercury_cfunc)function.data.p)(SubM, 3, 1);

								mercury_variable o;
								mercury_pullstack(SubM,&o);
								if (mercury_checkbool(&o)) { //soft break from all loops
#ifdef MERCURY_64BIT
									i1 = i2 = i3 = i4 = i5 = i6 = INT_MAX-1;
#else
									i1 = i2 = i3 = INT_MAX-1;
#endif
								}
								mercury_free_var(&o);
								mercury_clearstate(SubM);
							}
							else { //M functions get args in the reverse order. confusing, but it works.
								mercury_pushstack(SubM, &listlike);
								mercury_pushstack_unrefed(SubM, &v);
								mercury_pushstack(SubM, &idxvar);
								while (mercury_stepstate(SubM));

								mercury_variable o;
								mercury_pullstack(SubM, &o);
								if (mercury_checkbool(&o)) { //soft break from all loops
#ifdef MERCURY_64BIT
									i1 = i2 = i3 = i4 = i5 = i6 = INT_MAX-1;
#else
									i1 = i2 = i3 = INT_MAX-1;
#endif
								}
								mercury_free_var(&o);
								mercury_clearstate(SubM);
							}
						}


#ifdef MERCURY_64BIT
								}
							}
						}
					}
				}
			}
#else
					}
				}
			}
#endif
		}

		//arr->refrences = srefs;


	}
	else if (listlike.type == M_TYPE_TABLE) {
		mercury_table* tab = (mercury_table*)listlike.data.p;
		mercury_int srefs = tab->refrences;

		for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
			mercury_subtable* subt = tab->data[t];
			for (mercury_int i = 0; i < subt->size; i++) {
				mercury_variable k;
				mercury_clonevariable(subt->keys+i,&k); //because strings are not refed, we need to copy them to avoid using a freed pointer.
				mercury_variable v;
				mercury_clonevariable(subt->values + i, &v);
				
				if (function.type == M_TYPE_CFUNC) {
					mercury_pushstack_unrefed(SubM, &k);
					mercury_pushstack_unrefed(SubM, &v);
					mercury_pushstack(SubM, &listlike);
					((mercury_cfunc)function.data.p)(SubM, 3, 1);
					mercury_variable o;
					mercury_pullstack(SubM, &o);
					if (mercury_checkbool(&o)) {
						t = M_NUMBER_OF_TYPES; //soft break from both loops.
						i = subt->size;
					}
					mercury_free_var(&o);
					
				}
				else {
					mercury_pushstack(SubM, &listlike);
					mercury_pushstack_unrefed(SubM, &v);
					mercury_pushstack_unrefed(SubM, &k);
					
					while (mercury_stepstate(SubM));
					SubM->programcounter = 0; //reset position to start so we can run it again if it's a M func.

					mercury_variable o;
					mercury_pullstack(SubM, &o);
					if (mercury_checkbool(&o)) {
						t = M_NUMBER_OF_TYPES;
						i = subt->size;
					}
					mercury_free_var(&o);
				}
				mercury_clearstate(SubM);
			}
		}
		//tab->refrences = srefs;


	}

	SubM->bytecode= previous;
	mercury_clearstate(SubM);

	mercury_free_var(&function);
	mercury_free_var(&listlike);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, 0);
}


//executes a function whose enviroment is a supplied table. variadic, as it will supply additional args to the function
/*
TODO:
get return args
*/
void mercury_lib_std_restricted_call(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error_nonpointer(M,M_ERROR_NOT_ENOUGH_ARGS, args_in,2);
		return;
	}
	mercury_variable* argt = (mercury_variable*)malloc(sizeof(mercury_variable) * (args_in - 2));
	if (!argt && args_in>2) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (mercury_int a = args_in; a > 2; a--) {
		mercury_popstack(M, argt+ args_in - a);
	}

	mercury_variable tab;
	mercury_popstack(M,&tab);
	mercury_variable func;
	mercury_popstack(M,&func);

	if (tab.type != M_TYPE_TABLE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE,tab.type, M_TYPE_TABLE,2);
		free(argt);
		return;
	}
	if (func.type != M_TYPE_CFUNC && func.type != M_TYPE_FUNCTION) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE_EXPECTS_ANY_FUNCTION, func.type, 1);
		free(argt);
		return;
	}


	mercury_state* iso_M=mercury_newstate();
	if (!iso_M) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		free(argt);
		return;
	}
	mercury_destroytable(iso_M->enviroment);
	iso_M->enviroment = (mercury_table*)tab.data.p;

	for (mercury_int i = 0; i < args_in - 2; i++) {
		mercury_pushstack(iso_M, argt+i);
	}
	if (func.type == M_TYPE_FUNCTION) {
		mercury_function* func2 = (mercury_function*)func.data.p;
		void* nbl = realloc(iso_M->bytecode.instructions, func2->numberofinstructions * sizeof(mercury_opcode));
		if (!nbl) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_destroystate(iso_M);
			free(argt);
			return;
		}
		iso_M->bytecode.instructions = (mercury_opcode*)nbl;
		iso_M->bytecode.numberofinstructions = func2->numberofinstructions;
		iso_M->bytecode.instruction_dbg_lookup = nullptr;
		iso_M->bytecode.num_dbg_tokens = 0;
		iso_M->bytecode.dbg_tokens = nullptr;
		memcpy(iso_M->bytecode.instructions, func2->instructions, func2->numberofinstructions * sizeof(mercury_opcode));
	}

	mercury_variable out;
	out.type = M_TYPE_BOOL;
	out.data.i = 1;
	if (func.type == M_TYPE_CFUNC) {
		((mercury_cfunc)func.data.p)(iso_M, args_in-2, args_out ? args_out-1 : 0);
	}
	else {
		while (mercury_stepstate(iso_M));
	}
	if (iso_M->errorcode)out.data.i = 0;

	if (args_out) {
		mercury_pushstack(M, &out);
	}

	if (out.data.i) {
		for (mercury_int i = 1; i < args_out; i++) {
			mercury_pullstack(iso_M, &out);
			mercury_pushstack_unrefed(M, &out);
		}
	}
	else {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
	}


	free(argt);
	iso_M->enviroment=nullptr; //clear it before we free so that we don't discard the enviroment table.
	mercury_destroystate(iso_M);

	mercury_free_var(&func);
	mercury_free_var(&tab);

	//MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M,args_out, 1);
}


inline bool check_pointer_dumped(void* ptr, mercury_uint* num_pointers_covered, void*** pointers_covered) {
	for (mercury_uint i = 0; i < (*num_pointers_covered); i++) {
		if ((*pointers_covered)[i] == ptr) {
			return true;
		}
	}
	return false;
}

inline bool add_dump_pointer(void* ptr, mercury_uint* num_pointers_covered, void*** pointers_covered) {
	void* nptr = realloc(*pointers_covered, sizeof(void*) * ((*num_pointers_covered) + 1));
	if (!nptr)return false;
	*pointers_covered = (void**)nptr;

	(*pointers_covered)[*num_pointers_covered] = ptr;

	(*num_pointers_covered)++;
	return true;
}


mercury_string* m_stringify(mercury_rawdata data, uint8_t type,mercury_uint* num_pointers_covered,void*** pointers_covered) {
	mercury_string* str = nullptr;// mercury_cstring_to_mstring((char*)"", 0);


	mercury_string* temp = nullptr;
	mercury_string* temp2 = nullptr;
	char tout[256];
	for (int i = 0; i < 256; i++) {
		tout[i] = '\0';
	}
	int tint;

	switch (type)
	{
	case M_TYPE_NIL:
		mercury_cstring_to_mstring((char*)"nil", 3);
		break;
	case M_TYPE_INT:
		tint = snprintf(tout, sizeof(tout), "%zi", data.i);
		str = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_FLOAT:
#ifdef MERCURY_64BIT
		tint = snprintf(tout, sizeof(tout), "%.60g", data.f);
#else
		tint = snprintf(tout, sizeof(tout), "%.30g", data.f);
#endif
		str = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_BOOL:
		if (data.i) {
			str = mercury_cstring_to_mstring((char*)"true", 4);
		}
		else {
			str = mercury_cstring_to_mstring((char*)"false", 5);
		}
		break;
	case M_TYPE_TABLE:
		if (check_pointer_dumped(data.p, num_pointers_covered, pointers_covered)) {
			str = (mercury_string*)malloc(sizeof(mercury_string));
			if (str)memset(str, 0, sizeof(mercury_string));
		}
		else {
			add_dump_pointer(data.p, num_pointers_covered, pointers_covered);
			str = mercury_cstring_to_mstring((char*)"{", 1);
			{
				mercury_table* t = (mercury_table*)data.p;
				for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
					mercury_subtable* st = t->data[i];
					for (mercury_int n = 0; n < st->size; n++) {
						mercury_string* key = m_stringify(st->keys[n].data, i, num_pointers_covered, pointers_covered);
						mercury_variable v = st->values[n];
						mercury_string* value = m_stringify(v.data, v.type, num_pointers_covered, pointers_covered);

						if (!key || !value) {
							if(key)mercury_mstring_delete(key);
							if(value)mercury_mstring_delete(value);
							continue;
						}

						mercury_mstrings_append(str, key);
						mercury_mstring_delete(key);

						mercury_mstring_addchars(str, (char*)"=", 1);

						mercury_mstrings_append(str, value);
						mercury_mstring_delete(value);

						mercury_mstring_addchars(str, (char*)",");

					}
				}
			}
			mercury_mstring_addchars(str, (char*)"}");
		}
		break;
	case M_TYPE_STRING:
		str = (mercury_string*)malloc(sizeof(mercury_string));
		if (str) {
			mercury_string* cstr = (mercury_string*)data.p;
			mercury_int size_total_str = cstr->size;
			for (mercury_int i = 0; i < cstr->size; i++) {
				switch (cstr->ptr[i])
				{
				case '\\':
				case '\"':
					size_total_str++;
				}
			}
			str->ptr=(char*)malloc(sizeof(char)* (size_total_str+2));
			if (!str->ptr) {
				return nullptr;
			}
			str->size = size_total_str+2;

			str->ptr[0] = '\"';
			str->ptr[size_total_str+1] = '\"';

			if (str->ptr) {
				mercury_int traversed = 0;
				for (mercury_int i = 0; i < cstr->size; i++) { //can't use memcpy, we need to escape \ and ".
					switch (cstr->ptr[i])
					{
					case '\"':
					case '\\':
						str->ptr[traversed+1] = '\\';
						str->ptr[traversed+2] = cstr->ptr[i];
						traversed += 2;
						break;
					default:
						str->ptr[traversed+1] = cstr->ptr[i];
						traversed++;
						break;
					}
				}
			}
			else {
				free(str);
				str = nullptr;
			}


			


		}
		break;
	case M_TYPE_ARRAY:
		if (check_pointer_dumped(data.p, num_pointers_covered, pointers_covered)) {
			str = (mercury_string*)malloc(sizeof(mercury_string));
			if (str)memset(str, 0, sizeof(mercury_string));
		}
		else {
			add_dump_pointer(data.p, num_pointers_covered, pointers_covered);
			str = mercury_cstring_to_mstring((char*)"[", 1);
			{
				mercury_array* arr = (mercury_array*)data.p;

				if (arr->values) {
	#ifdef MERCURY_64BIT
					for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
						mercury_variable***** const st1 = arr->values[i1];
						if (!st1)continue;
						for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
							mercury_variable**** const st2 = st1[i2];
							if (!st2)continue;
							for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
								mercury_variable*** const st3 = st2[i3];
								if (!st3)continue;
								for (int i4 = 0; i4 < MERCURY_SIZE_SUBARRAY_4; i4++) {
									mercury_variable** const st4 = st3[i4];
									if (!st4)continue;
									for (int i5 = 0; i5 < MERCURY_SIZE_SUBARRAY_5; i5++) {
										mercury_variable* const st5 = st4[i5];
										if (!st5)continue;
										for (int i6 = 0; i6 < MERCURY_SIZE_SUBARRAY_6; i6++) {
											mercury_variable const var = st5[i6];
											const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3, i4, i5, i6);
	#else
					for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
						mercury_variable** const st1 = arr->values[i1];
						if (!st1)continue;
						for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
							mercury_variable* const st2 = st1[i2];
							if (!st2)continue;
							for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
								mercury_variable const var = st2[i3];
								const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3);
	#endif
								if (var.type) {
									temp = m_stringify(var.data, var.type, num_pointers_covered, pointers_covered);
									if (!temp)continue;

									mercury_string* temp2 = m_stringify({ index }, M_TYPE_INT, num_pointers_covered, pointers_covered);
									mercury_mstrings_append(str, temp2);
									mercury_mstring_delete(temp2);

									mercury_mstring_addchars(str, (char*)"=", 1);

									mercury_mstrings_append(str, temp);
									mercury_mstring_delete(temp);

									mercury_mstring_addchars(str, (char*)",");
								}

	#ifdef MERCURY_64BIT
										}
									}
								}
							}
						}
					}
	#else
							}
						}
					}
	#endif				
				}
			}
			mercury_mstring_addchars(str, (char*)"]");
		}
		break;
	default:
		return nullptr;
		break;
	}
	return str;
}








//takes a variable and generates a string which represents that variable.
void mercury_lib_std_dump(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable vartodump;
	mercury_popstack(M,&vartodump);

	mercury_uint nptrs=0;
	void** ptrs = nullptr;
	mercury_string* dmp_str = m_stringify(vartodump.data, vartodump.type,&nptrs,&ptrs);
	if (!dmp_str) {
		dmp_str = (mercury_string*)malloc(sizeof(mercury_string));
		if (!dmp_str) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		dmp_str->ptr = nullptr;
		dmp_str->size = 0;
	}
	free(ptrs);

	mercury_free_var(&vartodump);
	vartodump.type = M_TYPE_STRING;
	vartodump.data.p = dmp_str;
	mercury_pushstack(M, &vartodump);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_std_compile(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable codestr;
	mercury_popstack(M,&codestr);
	if (codestr.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, codestr.type,M_TYPE_STRING,1);
		return;
	}


	mercury_variable out;
	mercury_compile_mstring((mercury_string*)codestr.data.p, &out);
	mercury_free_var(&codestr);
	mercury_pushstack_unrefed(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, 1, 1);
}


void mercury_lib_std_type(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable var;
	mercury_popstack(M,&var);
	mercury_free_var(&var);
	var.data.i = var.type;
	var.type = M_TYPE_INT;
	mercury_pushstack(M, &var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}




void mercury_lib_std_tostring(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //pretty easy, actually. we already have a function.
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable i;
	mercury_popstack(M,&i);
	mercury_string* l = mercury_tostring(&i);
	mercury_free_var(&i); //we can just re-use the variable struct. saves time, probly
	i.type = M_TYPE_STRING;
	i.data.p = l;
	mercury_pushstack(M, &i);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_std_tonumber(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //bit more complicated.
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable i;
	mercury_popstack(M,&i);
	mercury_variable o;

	switch (i.type) {
	case M_TYPE_INT:
	case M_TYPE_FLOAT: //already numbers. easy.
		o.type = i.type;
		o.data = i.data;
		break;
	case M_TYPE_BOOL:
		o.type = M_TYPE_INT;
		o.data.i = i.data.i ? 1 : 0;
		break;
	case M_TYPE_STRING:
		{
		mercury_string* s = (mercury_string*)i.data.p;
		if (!s->size) {
			o.type = M_TYPE_NIL;
			o.data.i = 0;
			break;
		}
		char* c = mercury_mstring_to_cstring(s);
		char* e;
		mercury_int n = strtoll(c, &e, 0);
		if (*e == '\0') {
			o.type = M_TYPE_INT;
			o.data.i = n;
			break;
		}
		mercury_float f = strtod(c, &e);
		if (*e == '\0') {
			o.type = M_TYPE_FLOAT;
			o.data.f = f;
			break;
		}
		o.type = M_TYPE_NIL;
		o.data.i = 0;
		}
		break;
	default:
		o.type = M_TYPE_NIL;
		o.data.i = 0;
	}

	mercury_free_var(&i);
	mercury_pushstack(M, &o);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



void mercury_lib_std_dynamic_library_load(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //dangerous, hell yeah!
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable i;
	mercury_popstack(M,&i);

	if (i.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, i.type, M_TYPE_STRING,1);
		return;
	}

	char* c=mercury_mstring_to_cstring((mercury_string*)i.data.p);

	mercury_free_var(&i);

	mercury_variable o;
	o.type = M_TYPE_BOOL;
	

#ifdef _WIN32
	HMODULE lib = LoadLibraryA(c);
	if (!lib) {
		o.data.i = 0;
	}
	else {
		FreeLibrary(lib);
		o.data.i = 1;
	}
#else
	void* lib = dlopen(c, RTLD_NOW);
	if (!lib) {
		o.data.i = 0;
	}
	else {
		dlclose(lib);
		o.data.i = 1;
	}
#endif

	free(c);
	

	mercury_pushstack(M, &o);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_std_toint(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_lib_std_tonumber(M, 1, 1); //we already have to number code. might as well use it.

	mercury_variable i;
	mercury_popstack(M,&i);

	switch (i.type) { //because we know it'll only be a float, int, or nil, we can only check for float for extra easy code.
	case M_TYPE_FLOAT:
		i.type = M_TYPE_INT;
		i.data.i = (mercury_int)i.data.f;
		mercury_pushstack(M, &i);
		break;
	default:
		mercury_pushstack(M, &i);
		break;
	}
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_std_tofloat(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //basically the same thing as the above.
	if(MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_lib_std_tonumber(M, 1, 1);

	mercury_variable i;
	mercury_popstack(M, &i);

	switch (i.type) {
	case M_TYPE_INT:
		i.type = M_TYPE_FLOAT;
		i.data.f = (mercury_float)i.data.i;
		mercury_pushstack(M, &i);
		break;
	default:
		mercury_pushstack(M, &i);
		break;
	}
	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


enum deepcopy_returntypes :int {
	DEEPCOPY_SUCCES = 0,
	DEEPCOPY_MEMORY_ALLOCATION_ERROR = 1,
	DEEPCOPY_UNCOPYABLE_TYPE = 2,
};

inline bool var_can_be_deepcopied(const mercury_variable* var) {
	switch (var->type) {
	case M_TYPE_NIL:
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
	case M_TYPE_BOOL:
	case M_TYPE_CFUNC:
	case M_TYPE_STRING:
	case M_TYPE_FUNCTION:
	case M_TYPE_ARRAY:
	case M_TYPE_TABLE:
		return true;
	default:
		return false;
	}
}

inline bool type_is_pointer(const mercury_variable* var) {
	switch (var->type) {
	case M_TYPE_NIL:
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
	case M_TYPE_BOOL:
	case M_TYPE_CFUNC:
		return false;
	default:
		return true;
	};

}

inline void* get_converted_pointer(const void* in,mercury_uint* num_pointers_converted, void*** pointer_conversions_in, void*** pointer_conversions_out) {
	//printf("checking for self from %zu pointers\n",*num_pointers_converted);
	for (mercury_uint i = 0; i < (*num_pointers_converted); i++) {
		//printf("comparing %p to %p\n", (*pointer_conversions_in)[i],in);
		if ( (*pointer_conversions_in)[i] == in) {
			//printf("output convert pointer\n");
			return (*pointer_conversions_out)[i];
		}
	}
	return nullptr;
}

inline bool add_converted_pointer(void* originalp,void* newp, mercury_uint* num_pointers_converted, void*** pointer_conversions_in, void*** pointer_conversions_out) {
	void* nptr=realloc(*pointer_conversions_in, sizeof(void*) * ((*num_pointers_converted) + 1));
	if (!nptr)return false;
	*pointer_conversions_in = (void**)nptr;
	nptr = realloc(*pointer_conversions_out, sizeof(void*) * ((*num_pointers_converted) + 1) );
	if (!nptr)return false;
	*pointer_conversions_out = (void**)nptr;

	//printf("adding pointer %p replacded by %p\n",originalp,newp);

	(*pointer_conversions_in)[*num_pointers_converted] = originalp;
	(*pointer_conversions_out)[*num_pointers_converted] = newp;

	(*num_pointers_converted)++;
	return true;
}

//pointer to an array of void*s
int m_variable_deepcopy(mercury_variable* var_in, mercury_variable* var_out,mercury_uint* num_pointers_converted,void*** pointer_conversions_in,void*** pointer_conversions_out) {
	var_out->type = var_in->type;
	//printf("called!\n");
	if (type_is_pointer(var_in)) {
		void* cp = get_converted_pointer(var_in->data.p, num_pointers_converted, pointer_conversions_in, pointer_conversions_out);
		if (cp) {
			var_out->data.p = cp;
			//mercury_increment_variable_refrence_count(var_out);
			return DEEPCOPY_SUCCES;
		}
	}

	switch (var_in->type) {
		case M_TYPE_NIL:
		case M_TYPE_INT:
		case M_TYPE_FLOAT:
		case M_TYPE_BOOL:
		case M_TYPE_CFUNC:
			var_out->data = var_in->data;
			return DEEPCOPY_SUCCES;
		case M_TYPE_TABLE:
			{
			mercury_table* newtab=mercury_newtable();
			mercury_table* intab = (mercury_table*)var_in->data.p;
			if (!newtab) {
				return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
			}
			add_converted_pointer(intab, newtab, num_pointers_converted, pointer_conversions_in, pointer_conversions_out);

			newtab->enviromental = false;
			newtab->refrences = 0;
			for (uint8_t t = 0; t < M_NUMBER_OF_TYPES;t++) {
				mercury_subtable* st = intab->data[t];
				mercury_variable key;
				mercury_variable value;
				for (mercury_int i = 0; i < st->size; i++) {
					key=st->keys[i];
					value=st->values[i];
					if (var_can_be_deepcopied(&key) && var_can_be_deepcopied(&value) ) {
						mercury_variable newkey;
						mercury_variable newvalue;
						if (m_variable_deepcopy(&key, &newkey, num_pointers_converted, pointer_conversions_in, pointer_conversions_out) != DEEPCOPY_SUCCES) {
							continue;
						}
						if(m_variable_deepcopy(&value,&newvalue,num_pointers_converted,pointer_conversions_in,pointer_conversions_out) != DEEPCOPY_SUCCES) {
							mercury_free_var(&newkey);
							continue;
						}
						mercury_increment_variable_refrence_count(&newkey);
						mercury_increment_variable_refrence_count(&newvalue);
						mercury_setkey(newtab, &newkey, &newvalue);
					}
				}
			}
			var_out->data.p = newtab;

			}
			return DEEPCOPY_SUCCES;
		case M_TYPE_STRING:
			{
			mercury_string* nptr = (mercury_string*)malloc(sizeof(mercury_string));
			mercury_string* instr = (mercury_string*)var_in->data.p;
			if (!nptr) {
				return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
			}
			memset(nptr, 0, sizeof(mercury_string));
			nptr->ptr=(char*)malloc(instr->size);
			if (!nptr->ptr) {
				free(nptr);
				return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
			}
			add_converted_pointer(instr, nptr, num_pointers_converted, pointer_conversions_in, pointer_conversions_out);
			memcpy(nptr->ptr, instr->ptr, instr->size);
			nptr->size = instr->size;
			nptr->refrences = 0;
			var_out->data.p = nptr;
			}
			return DEEPCOPY_SUCCES;
		case M_TYPE_ARRAY:
			{
				mercury_array* newarr = mercury_newarray();
				newarr->refrences = 0;
				mercury_array* inarr = (mercury_array*)var_in->data.p;
				if (!newarr) {
					return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
				}
				
				add_converted_pointer(inarr, newarr, num_pointers_converted, pointer_conversions_in, pointer_conversions_out);

				if (inarr->values) {
#ifdef MERCURY_64BIT
					for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
						mercury_variable***** const st1 = inarr->values[i1];
						if (!st1)continue;
						for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
							mercury_variable**** const st2 = st1[i2];
							if (!st2)continue;
							for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
								mercury_variable*** const st3 = st2[i3];
								if (!st3)continue;
								for (int i4 = 0; i4 < MERCURY_SIZE_SUBARRAY_4; i4++) {
									mercury_variable** const st4 = st3[i4];
									if (!st4)continue;
									for (int i5 = 0; i5 < MERCURY_SIZE_SUBARRAY_5; i5++) {
										mercury_variable* const st5 = st4[i5];
										if (!st5)continue;
										for (int i6 = 0; i6 < MERCURY_SIZE_SUBARRAY_6; i6++) {
											mercury_variable const var = st5[i6];
											const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3, i4, i5, i6);
#else
					for (int i1 = 0; i1 < MERCURY_SIZE_SUBARRAY_1; i1++) {
						mercury_variable** const st1 = inarr->values[i1];
						if (!st1)continue;
						for (int i2 = 0; i2 < MERCURY_SIZE_SUBARRAY_2; i2++) {
							mercury_variable* const st2 = st1[i2];
							if (!st2)continue;
							for (int i3 = 0; i3 < MERCURY_SIZE_SUBARRAY_3; i3++) {
								mercury_variable const var = st2[i3];
								const mercury_int index = mercury_reconstruct_array_index(i1, i2, i3);
#endif
								if (var.type && var_can_be_deepcopied(&var)) {
									mercury_variable newvar;
									if (m_variable_deepcopy((mercury_variable*)&var, &newvar, num_pointers_converted, pointer_conversions_in, pointer_conversions_out) != DEEPCOPY_SUCCES) {
										continue;
									}
									mercury_increment_variable_refrence_count(&newvar);
									mercury_setarray(newarr, &newvar, index);
								}

#ifdef MERCURY_64BIT
							}
						}
					}
										}
									}
								}
#else
							}
						}
					}
#endif				
				}

				var_out->data.p = newarr;
			}
			
			return DEEPCOPY_SUCCES;
		case M_TYPE_FUNCTION:
			{
			mercury_function* nptr = (mercury_function*)malloc(sizeof(mercury_function));
			mercury_function* infun = (mercury_function*)var_in->data.p;
			if (!nptr) {
				return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
			}
			memset(nptr, 0, sizeof(mercury_function));
			mercury_clone_function(infun, nptr);
			nptr->refrences = 0;
			if (nptr->numberofinstructions != infun->numberofinstructions) {
				free(nptr);
				return DEEPCOPY_MEMORY_ALLOCATION_ERROR;
			}

			add_converted_pointer(infun, nptr, num_pointers_converted, pointer_conversions_in, pointer_conversions_out);
			}
			return DEEPCOPY_SUCCES;
		case M_TYPE_FILE:
		case M_TYPE_THREAD:
		default:
			return DEEPCOPY_UNCOPYABLE_TYPE;
	}

}



void mercury_lib_std_deepcopy(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out) {
		return;
	}

	mercury_variable in;
	mercury_variable out;
	mercury_popstack(M, &in);
	
	mercury_uint nc = 0;
	void** cpi = nullptr;
	void** cpo = nullptr;
	int code=m_variable_deepcopy(&in, &out, &nc, &cpi, &cpo);
	free(cpi);
	free(cpo);
	mercury_free_var(&in);

	if (code != DEEPCOPY_SUCCES) {
		out.data.i = 0;
		out.type = M_TYPE_NIL;
	}
	mercury_increment_variable_refrence_count(&out);
	mercury_pushstack(M, &out);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_std_error(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0,1))return;


	mercury_variable in;
	in.type = M_TYPE_NIL;
	if(args_in)mercury_popstack(M, &in);
	mercury_string* str=mercury_tostring(&in);
	if (str) {
		char* cstr = mercury_mstring_to_cstring(str);
		if (cstr) {
			mercury_raise_error(M, M_ERROR_CUSTOM_STRING, (const mercury_int*)cstr);
			free(cstr);
		}
		mercury_mstring_delete(str);
	}
	mercury_free_var(&in);
	

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,0);
}



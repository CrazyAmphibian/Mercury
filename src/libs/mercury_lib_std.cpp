#include"../mercury.h"
#include"../mercury_error.h"
#include"../mercury_bytecode.h"

#include"malloc.h"
#include"stdio.h"
#include"string.h"
#include <stdlib.h>


#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

//throws stuff into stdout. adds a newline at the end, seperates with a tab. designed to be variadic.
void mercury_lib_std_print(mercury_state* M, mercury_int args_in, mercury_int args_out) {

	mercury_variable** vartable = (mercury_variable**)malloc(sizeof(mercury_variable*) * args_in);
	if (vartable == nullptr && args_in) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	for (mercury_int a = args_in-1; a >= 0; a--) {
		vartable[a] = mercury_popstack(M);
	}

	for (mercury_int a = 0; a < args_in; a++) {

		mercury_variable* mstrv = mercury_tostring(vartable[a]);
		mercury_unassign_var(M, vartable[a]);
		if (mstrv->type == M_TYPE_STRING) {
			mercury_stringliteral* str = (mercury_stringliteral*)mstrv->data.p;

			

			for (mercury_int c = 0; c < str->size; c++) {
				putchar(str->ptr[c]);
			}
			putchar('\t');
			fflush(stdout);

		}
		mercury_free_var(mstrv);
	}
	putchar('\n');
	fflush(stdout);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


//traverses a list-like variable entry-by-entry. useful for serializing tables, most likley. arg 1 is the thing, and arg 2 is the function.
void mercury_lib_std_iterate(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}

	for (mercury_int a = 2; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* function = mercury_popstack(M);
	mercury_variable* listlike = mercury_popstack(M);


	if (function->type != M_TYPE_CFUNC && function->type != M_TYPE_FUNCTION) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M_TYPE_FUNCTION, (void*)function->type);
		return;
	}


	mercury_state* SubM = mercury_newstate(M);
	if (function->type == M_TYPE_FUNCTION) {
		mercury_function* func = (mercury_function*)function->data.p;
		void* nbl = realloc(SubM->bytecode.instructions, func->numberofinstructions * sizeof(uint32_t));
		if (!nbl) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_destroystate(SubM);
			return;
		}
		SubM->bytecode.instructions = (uint32_t*)nbl;
		SubM->bytecode.numberofinstructions = func->numberofinstructions;
		memcpy(SubM->bytecode.instructions, func->instructions, func->numberofinstructions * sizeof(uint32_t));
	}

	if (listlike->type == M_TYPE_ARRAY) {
		listlike->constant = true; //prevents the array being garbage collected when we mess with the substate.
		mercury_array* arr = (mercury_array*)listlike->data.p;
		mercury_int srefs = arr->refrences;

		for (mercury_int b = 0; b < arr->size; b++) {
			if (!arr->values[b])continue;
			for (mercury_int i = 0; i < (1 << MERCURY_ARRAY_BLOCKSIZE) - 1; i++) {
				mercury_variable* var = arr->values[b][i];
				if (var) {
					mercury_variable* idxvar = mercury_assign_var(M);
					idxvar->data.i = (b << MERCURY_ARRAY_BLOCKSIZE) | i;
					idxvar->type = M_TYPE_INT;

					if (function->type == M_TYPE_CFUNC) {
						mercury_pushstack(SubM, idxvar);
						mercury_pushstack(SubM, var);
						mercury_pushstack(SubM, listlike);
						((mercury_cfunc)function->data.p)(SubM,3,1);

						mercury_variable* o = mercury_pullstack(SubM);
						if (mercury_checkbool(o)) {
							b = arr->size; //soft break from both loops.
							i = (1 << MERCURY_ARRAY_BLOCKSIZE) - 1;
						}
						mercury_free_var(o);
					}
					else { //M functions get args in the reverse order. confusing, but it works.
						mercury_pushstack(SubM, listlike);
						mercury_pushstack(SubM, var);
						mercury_pushstack(SubM, idxvar);
						while (mercury_stepstate(SubM));
						SubM->programcounter = 0; //reset position to start so we can run it again if it's a M func.

						mercury_variable* o = mercury_pullstack(SubM);
						if (mercury_checkbool(o)) {
							b = arr->size; //soft break from both loops.
							i = (1 << MERCURY_ARRAY_BLOCKSIZE) - 1;
						}
						mercury_free_var(o);

						M_BYTECODE_CLS(SubM, 0); //clear the stack to clean stuff up.
					}
				}
			}
		}
		arr->refrences = srefs;


	}
	else if (listlike->type == M_TYPE_TABLE) {
		listlike->constant = true;
		mercury_table* tab = (mercury_table*)listlike->data.p;
		mercury_int srefs = tab->refrences;

		for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
			mercury_subtable* subt = tab->data[t];
			for (mercury_int i = 0; i < subt->size; i++) {
				mercury_variable ik;
				ik.constant = false;
				ik.type = t;
				ik.data = subt->keys[i];
				mercury_variable* k = mercury_clonevariable(&ik);
				mercury_variable* v = mercury_clonevariable(subt->values[i]);

				
				if (function->type == M_TYPE_CFUNC) {
					mercury_pushstack(SubM, k);
					mercury_pushstack(SubM, v);
					mercury_pushstack(SubM, listlike);
					((mercury_cfunc)function->data.p)(SubM, 3, 1);
					mercury_variable* o=mercury_pullstack(SubM);
					if (mercury_checkbool(o)) {
						t = M_NUMBER_OF_TYPES; //soft break from both loops.
						i = subt->size;
					}
					mercury_free_var(o);
					
				}
				else {
					mercury_pushstack(SubM, listlike);
					mercury_pushstack(SubM, v);
					mercury_pushstack(SubM, k);
					
					while (mercury_stepstate(SubM));
					SubM->programcounter = 0; //reset position to start so we can run it again if it's a M func.

					mercury_variable* o = mercury_pullstack(SubM);
					if (mercury_checkbool(o)) {
						t = M_NUMBER_OF_TYPES;
						i = subt->size;
					}
					mercury_free_var(o);
				}
				M_BYTECODE_CLS(SubM, 0); //clear the stack to clean stuff up.
				
			}
		}
		tab->refrences = srefs;


	}
	else {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M_TYPE_TABLE, (void*)listlike->type);
		mercury_destroystate(SubM);
		return;
	}

	mercury_destroystate(SubM);

	listlike->constant = false;
	mercury_unassign_var(M, function);
	mercury_unassign_var(M, listlike);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


//executes a function whose enviroment is a supplied table. variadic, as it will supply additional args to the function
/*
TODO:
get return args
*/
void mercury_lib_std_restricted_call(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M,M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}
	mercury_variable** argt = (mercury_variable**)malloc(sizeof(mercury_variable*) * (args_in - 2));
	if (!argt && args_in>2) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	for (mercury_int a = args_in; a > 2; a--) {
		argt[args_in-a]=mercury_popstack(M);
	}

	mercury_variable* tab = mercury_popstack(M);
	mercury_variable* func = mercury_popstack(M);

	if (tab->type != M_TYPE_TABLE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M_TYPE_TABLE , (void*)tab->type );
		free(argt);
		return;
	}
	if (func->type != M_TYPE_CFUNC && func->type != M_TYPE_FUNCTION) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)M_TYPE_FUNCTION, (void*)func->type);
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
	iso_M->enviroment = (mercury_table*)tab->data.p;

	for (mercury_int i = 0; i < args_in - 2; i++) {
		mercury_pushstack(iso_M, argt[i]);
	}
	if (func->type == M_TYPE_FUNCTION) {
		mercury_function* func2 = (mercury_function*)func->data.p;
		void* nbl = realloc(iso_M->bytecode.instructions, func2->numberofinstructions * sizeof(uint32_t));
		if (!nbl) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			mercury_destroystate(iso_M);
			free(argt);
			return;
		}
		iso_M->bytecode.instructions = (uint32_t*)nbl;
		iso_M->bytecode.numberofinstructions = func2->numberofinstructions;
		memcpy(iso_M->bytecode.instructions, func2->instructions, func2->numberofinstructions * sizeof(uint32_t));
	}


	if (func->type == M_TYPE_CFUNC) {
		((mercury_cfunc)func->data.p)(iso_M, 2, 0);
	}
	else {
		while (mercury_stepstate(iso_M));
	}



	iso_M->enviroment=nullptr; //clear it before we free so that we don't discard the enviroment table.
	mercury_destroystate(iso_M);


	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



mercury_stringliteral* m_stringify(mercury_rawdata data, uint8_t type) {
	mercury_stringliteral* str = nullptr;// mercury_cstring_to_mstring((char*)"", 0);


	mercury_stringliteral* temp = nullptr;
	mercury_stringliteral* temp2 = nullptr;
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
		tint = snprintf(tout, sizeof(tout), "%.60lg", data.f);
#else
		tint = snprintf(tout, sizeof(tout), "%.30lg", data.f);
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
		str = mercury_cstring_to_mstring((char*)"{", 1);
		{
			mercury_table* t = (mercury_table*)data.p;
			for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
				mercury_subtable* st = t->data[i];
				for (mercury_int n = 0; n < st->size; n++) {
					mercury_stringliteral* key=m_stringify(st->keys[n],i);
					mercury_variable* v = st->values[n];
					mercury_stringliteral* value=m_stringify(v->data, v->type);

					if (!key || !value) {
						mercury_mstring_delete(key);
						mercury_mstring_delete(value);
						continue;
					}

					mercury_mstring_addchars(str, (char*)"[");

					mercury_mstrings_append(str, key);
					mercury_mstring_delete(key);

					mercury_mstring_addchars(str, (char*)"]=",2);

					mercury_mstrings_append(str, value);
					mercury_mstring_delete(value);

					mercury_mstring_addchars(str, (char*)",");
					
				}
			}
		}
		mercury_mstring_addchars(str, (char*)"}");
		break;
	case M_TYPE_STRING:
		str = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
		if (str) {
			mercury_stringliteral* cstr = (mercury_stringliteral*)data.p;
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
		str = mercury_cstring_to_mstring((char*)"[", 1);
		{
			mercury_array* arr = (mercury_array*)data.p;
			for (mercury_int block = 0; block < arr->size; block++) {
				//mercury_int block = i >> MERCURY_ARRAY_BLOCKSIZE;

				if (arr->values[block]) {
					for (mercury_int pos = 0; pos < (1 << MERCURY_ARRAY_BLOCKSIZE); pos++) {
						//mercury_int pos = i & ((1 << MERCURY_ARRAY_BLOCKSIZE) - 1);
						if (!arr->values[block][pos])continue;
						mercury_int i = (block << MERCURY_ARRAY_BLOCKSIZE) | pos;

						mercury_variable* var = arr->values[block][pos];
						temp = m_stringify(var->data, var->type);
						if (!temp)continue;

						mercury_mstring_addchars(str, (char*)"[");

						mercury_stringliteral* temp2 = m_stringify({ i = i }, M_TYPE_INT);
						mercury_mstrings_append(str, temp2);
						mercury_mstring_delete(temp2);

						mercury_mstring_addchars(str, (char*)"]=", 2);

						mercury_mstrings_append(str, temp);
						mercury_mstring_delete(temp);

						mercury_mstring_addchars(str, (char*)",");
					}
				}
			}
		}

		mercury_mstring_addchars(str, (char*)"]");
		break;
	default:
		return nullptr;
		break;
	}
	return str;
}








//takes a variable and generates a string which represents that variable.
void mercury_lib_std_dump(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* vartodump = mercury_popstack(M);

	mercury_variable* dump_var=mercury_assign_var(M);

	mercury_stringliteral* dmp_str = m_stringify(vartodump->data, vartodump->type);
	if (!dmp_str) {
		dmp_str = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
		if (!dmp_str) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		dmp_str->ptr = nullptr;
		dmp_str->size = 0;
	}

	dump_var->type = M_TYPE_STRING;
	dump_var->data.p = dmp_str;
	mercury_pushstack(M, dump_var);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_std_compile(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)1, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* codestr = mercury_popstack(M);
	if (codestr->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)codestr->type, (void*)M_TYPE_STRING);
		return;
	}


	mercury_variable* out=mercury_compile_mstring((mercury_stringliteral*)codestr->data.p);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_std_type(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}
	mercury_variable* var = mercury_popstack(M);

	mercury_variable* out = mercury_assign_var(M);
	out->type = M_TYPE_INT;
	out->data.i = (mercury_int)var->type;
	mercury_pushstack(M, out);

	mercury_free_var(var);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




void mercury_lib_std_tostring(mercury_state* M, mercury_int args_in, mercury_int args_out) { //pretty easy, actually. we already have a function.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* i = mercury_popstack(M);
	mercury_variable* o=mercury_tostring(i);
	mercury_pushstack(M, o);
	mercury_unassign_var(M,i);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_std_tonumber(mercury_state* M, mercury_int args_in, mercury_int args_out) { //bit more complicated.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)1, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* i = mercury_popstack(M);
	mercury_variable* o = mercury_assign_var(M);


	switch (i->type) {
	case M_TYPE_INT:
	case M_TYPE_FLOAT: //already numbers. easy.
		o->type = i->type;
		o->data = i->data;
		break;
	case M_TYPE_BOOL:
		o->type = M_TYPE_INT;
		o->data.i = i->data.i ? 1 : 0;
		break;
	case M_TYPE_STRING:
		{
		mercury_stringliteral* s = (mercury_stringliteral*)i->data.p;
		if (!s->size) {
			o->type = M_TYPE_NIL;
			o->data.i = 0;
			break;
		}
		char* c = mercury_mstring_to_cstring(s);
		char* e;
		mercury_int n = strtoll(c, &e, 0);
		if (*e == '\0') {
			o->type = M_TYPE_INT;
			o->data.i = n;
			break;
		}
		mercury_float f = strtod(c, &e);
		if (*e == '\0') {
			o->type = M_TYPE_FLOAT;
			o->data.f = f;
			break;
		}
		o->type = M_TYPE_NIL;
		o->data.i = 0;
		}
		break;
	default:
		o->type = M_TYPE_NIL;
		o->data.i = 0;
	}


	mercury_pushstack(M, o);
	mercury_unassign_var(M, i);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_std_dynamic_library_load(mercury_state* M, mercury_int args_in, mercury_int args_out) { //dangerous, hell yeah!
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)2, (void*)args_in);
		return;
	}

	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* i = mercury_popstack(M);

	if (i->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)i->type, (void*)M_TYPE_STRING);
		return;
	}

	char* c=mercury_mstring_to_cstring((mercury_stringliteral*)i->data.p);


	mercury_variable* o = mercury_assign_var(M);
	o->type = M_TYPE_BOOL;
	

#ifdef _WIN32
	HMODULE lib = LoadLibraryA(c);
	if (!lib) {
		o->data.i = 0;
	}
	else {
		FreeLibrary(lib);
		o->data.i = 1;
	}
#else
	void* lib = dlopen(c, RTLD_NOW);
	if (!lib) {
		o->data.i = 0;
	}
	else {
		dlclose(lib);
		o->data.i = 1;
	}
#endif

	free(c);
	mercury_unassign_var(M, i);
	

	mercury_pushstack(M, o);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_std_toint(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)1, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) { //remove extra input args
		mercury_unassign_var(M, mercury_popstack(M));
	}


	mercury_lib_std_tonumber(M, 1, 1); //we already have to number code. might as well use it.

	mercury_variable* i = mercury_popstack(M);
	mercury_variable* o; //= mercury_assign_var(M);

	switch (i->type) { //because we know it'll only be a float, int, or nil, we can only check for float for extra easy code.
	case M_TYPE_FLOAT:
		o = mercury_assign_var(M);
		o->type = M_TYPE_INT;
		o->data.i = (mercury_int)i->data.f;
		mercury_pushstack(M, o);
		break;
	default:
		o = i;
		mercury_pushstack(M, o);
		break;
	}
	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_std_tofloat(mercury_state* M, mercury_int args_in, mercury_int args_out) { //basically the same thing as the above.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)1, (void*)args_in);
		return;
	}
	if (!args_out) {
		return;
	}
	for (mercury_int a = 1; a < args_in; a++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}


	mercury_lib_std_tonumber(M, 1, 1);

	mercury_variable* i = mercury_popstack(M);
	mercury_variable* o;

	switch (i->type) {
	case M_TYPE_INT:
		o = mercury_assign_var(M);
		o->type = M_TYPE_FLOAT;
		o->data.f = (mercury_float)i->data.i;
		mercury_pushstack(M, o);
		break;
	default:
		o = i;
		mercury_pushstack(M, o);
		break;
	}
	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


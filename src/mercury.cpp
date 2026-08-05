#include "mercury.hpp"
#include "mercury_bytecode.hpp"
#include "mercury_compiler.hpp"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <pthread.h>
#endif

//mercury libs
#include "libs/mercury_lib_std.hpp"
#include "libs/mercury_lib_math.hpp"
#include "libs/mercury_lib_array.hpp"
#include "libs/mercury_lib_string.hpp"
#include "libs/mercury_lib_os.hpp"
#include "libs/mercury_lib_io.hpp"
#include "libs/mercury_lib_thread.hpp"
#include "libs/mercury_lib_table.hpp"
#include "libs/mercury_lib_debug.hpp"

uint8_t M_NUMBER_OF_TYPES = 11; //VERY IMPORTANT that this is kept at the proper number.
uint16_t register_max = 0xf;






mercury_string* mercury_cstring_to_mstring(const char* const M_CPP_restrict str , const mercury_int size) {
	mercury_string* nstr=(mercury_string*)malloc(sizeof(mercury_string));
	if (!nstr) return nullptr;
	char* nad = (char*)malloc(sizeof(char) * size);
	if (!nad) return nullptr;
	nstr->size = size;
	
	memcpy(nad,str,size*sizeof(char));
	nstr->ptr = nad;
	nstr->constant = false;
	nstr->refrences = 1;
	return nstr;
}

mercury_string* mercury_cstring_const_to_mstring(const char* const M_CPP_restrict str, const mercury_int size) {
	mercury_string* nstr = (mercury_string*)malloc(sizeof(mercury_string));
	if (!nstr) return nullptr;
	nstr->size = size;
	nstr->ptr = (char*)str;
	nstr->constant = true;
	nstr->refrences = 1;
	return nstr;
}

char* mercury_mstring_to_cstring(const mercury_string* const M_CPP_restrict str) {
	//mercury_int sz = strlen(str->ptr); //use this mecause null terminator
	//printf("%i/%i %s",str->size,sz,str->ptr);
	//if(sz > str->size)sz = str->size;
	char* out = (char*)malloc(sizeof(char) * (str->size+1)); 
	if (!out)return nullptr;

	memcpy(out, str->ptr, str->size*sizeof(char));
	out[str->size] = '\0';
	return out;
}


mercury_string* mercury_copystring(const mercury_string* const M_CPP_restrict str) {
	if (str->constant) {
		mercury_string* nstr = (mercury_string*)malloc(sizeof(mercury_string));
		if (nstr == nullptr) return nullptr;
		nstr->size = str->size;
		nstr->constant = true;
		nstr->ptr = str->ptr;
		nstr->refrences = 1;
		return nstr;
	}
	else {
		mercury_string* nstr = (mercury_string*)malloc(sizeof(mercury_string));
		if (nstr == nullptr) return nullptr;
		nstr->ptr = (char*)malloc(sizeof(char) * str->size);
		if (nstr->ptr == nullptr) return nullptr;
		nstr->size = str->size;
		nstr->constant = false;
		nstr->refrences = 1;
		memcpy(nstr->ptr, str->ptr, str->size * sizeof(char));
		return nstr;
	}
}

mercury_string* mercury_mstrings_concat(const mercury_string* const str1, const mercury_string* const str2) {
	mercury_string* nstr=(mercury_string*)malloc(sizeof(mercury_string));
	if (nstr == nullptr) return nullptr;

	nstr->ptr=(char*)malloc(sizeof(char) * (str1->size + str2->size));
	if (nstr->ptr == nullptr) {
		free(nstr);
		return nullptr;
	}

	memcpy(nstr->ptr,str1->ptr,str1->size*sizeof(char));
	memcpy(nstr->ptr+str1->size, str2->ptr, str2->size * sizeof(char));
	nstr->constant = false;
	nstr->size = str1->size + str2->size;
	nstr->refrences = 1;
	return nstr;
}

//like concat, but does not return a new string. adds appstr to the end of basestr.
bool mercury_mstrings_append(mercury_string* const basestr, const mercury_string* const appstr) {
	if (basestr->constant) {
		char* nptr = (char*)malloc( sizeof(char) * (basestr->size + appstr->size));
		if (!nptr)return false;
		if(basestr->size)memcpy(nptr,basestr->ptr, basestr->size);
		basestr->ptr = nptr;
		if(appstr->size)memcpy(basestr->ptr + basestr->size, appstr->ptr, appstr->size);
		basestr->size += appstr->size;
		basestr->constant = false;
	}
	else {
		char* nptr = (char*)realloc(basestr->ptr, sizeof(char) * (basestr->size + appstr->size));
		if (!nptr)return false;
		basestr->ptr = nptr;
		if(appstr->size)memcpy(basestr->ptr + basestr->size, appstr->ptr, appstr->size);
		basestr->size += appstr->size;
	}
	return true;
}

bool mercury_mstring_addchars(mercury_string* const M_CPP_restrict str, const char* const chars, mercury_int len) {
	if (str->constant) {
		char* nptr = (char*)malloc(sizeof(char) * (str->size + len));
		if (!nptr)return false;
		memcpy(nptr,str->ptr, str->size);
		str->ptr = nptr;
		memcpy(str->ptr + str->size, chars, len);
		str->size += len;
		str->constant = false;
	}
	else {
		char* nptr = (char*)realloc(str->ptr, sizeof(char) * (str->size + len));
		if (!nptr)return false;
		str->ptr = nptr;
		for (mercury_int i=0;i<len;i++){
			str->ptr[str->size + i]=chars[i];
		}
		//memcpy(str->ptr + str->size, chars, len);
		str->size += len;
	}
	return true;
}

void mercury_mstring_delete(mercury_string* const M_CPP_restrict str) {
	if (!str->constant) {
		free(str->ptr);
	}
	free(str);
}

mercury_string* mercury_mstring_substring(mercury_string* str, mercury_int start, mercury_int end) {
	mercury_string* nstr = (mercury_string*)malloc(sizeof(mercury_string));
	if (nstr == nullptr) return nullptr;

	if (start > str->size || end < 0) { //no characters? just return an empty string.
		nstr->ptr = (char*)"";
		nstr->constant = true;
		nstr->size = 0;

		return nstr;
	}

	start = start < 0 ? 0 : start;
	end = end > str->size-1 ? str->size-1 : end; //clip to the bounds of the string.

	nstr->size = 1l+end - start;
	nstr->ptr=(char*)malloc(sizeof(char)*nstr->size);
	nstr->refrences = 1;
	nstr->constant = false;
	if (!nstr->ptr) {
		nstr->size = 0;
		return nstr;
	}
	memcpy(nstr->ptr, str->ptr + start,nstr->size*sizeof(char));

	return nstr;
}


mercury_table* mercury_newtable() {

	mercury_table* newt = (mercury_table*)malloc(sizeof(mercury_table));
	if (newt == nullptr) return nullptr;
	newt->data=(mercury_subtable**)malloc(sizeof(mercury_subtable*) * M_NUMBER_OF_TYPES);
	if (!newt->data)return nullptr;

	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* st=(mercury_subtable*)malloc(sizeof(mercury_subtable));
		if (st == nullptr) { //we have to deallocate all of the subtables. this is annoying.
			for (uint8_t n = 0; n < i; n++) {
				free(newt->data[n]);
			}
			free(newt->data);
			free(newt);
			return nullptr;
		}
		st->size = 0;
		st->keys = nullptr;
		st->values = nullptr;
		newt->data[i] = st;
	}
	newt->enviromental = false;
	newt->refrences = 1;

	return newt;
}

void mercury_destroytable(mercury_table* const table) { //not ideal, but it works. kind of.
	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* st = table->data[i];
		for (mercury_int i2 = 0; i2 < st->size; i2++) {
			mercury_free_var(st->keys+i2);
			mercury_free_var(st->values+i2);
		}
		free(st->keys);
		free(st->values);
		free(st);
	}
	free(table->data);
	free(table);
}

void mercury_cleartable(const mercury_table* const table) {
	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* st = table->data[i];
		for (mercury_int i2 = 0; i2 < st->size; i2++) {
			mercury_free_var(st->keys+i2);
			mercury_free_var(st->values+i2);
		}
		st->size = 0;
	}
}


mercury_int mercury_tablehaskey(const mercury_table* const table, const mercury_variable* const key) {
	const mercury_subtable* const subt = table->data[key->type];
	for (mercury_int i = 0; i < subt->size; i++) {
		if (mercury_vars_equal(subt->keys+i,key))return i;
	}
	return -1;
}

bool mercury_getkey(const mercury_table* const table, mercury_variable* const key, mercury_variable* out) {
	const mercury_subtable* const subt=table->data[key->type];
	for (mercury_int i = 0; i < subt->size; i++) {
		if (mercury_vars_equal(subt->keys+i, key)) {
			mercury_free_var(key);
			mercury_clonevariable(subt->values+i, out);
			return true;
		}
	}
	mercury_free_var(key);
	out->type = M_TYPE_NIL;
	out->data.i = 0;
	return false;
}

mercury_int mercury_setkey(mercury_table* const table, mercury_variable* const key, const mercury_variable* const value) {
	mercury_subtable* subt = table->data[key->type];
	for (mercury_int i = 0; i < subt->size; i++) {
		if (mercury_vars_equal(subt->keys+i,key)) {
			mercury_free_var(subt->values+i);
			mercury_free_var(key);
			subt->values[i] = *value;
			return i;
		}
	}

	void* nptr=realloc(subt->keys,sizeof(mercury_variable)*(subt->size+1) );
	if (nptr == nullptr) return -1;
	subt->keys = (mercury_variable*)nptr;
	nptr = realloc(subt->values, sizeof(mercury_variable) * (subt->size + 1));
	if (nptr == nullptr) return -1;
	subt->values = (mercury_variable*)nptr;

	subt->keys[subt->size] = *key;
	subt->values[subt->size] = *value;

	subt->size++;

	return subt->size-1;
}

bool mercury_tables_equal(const mercury_table* const table1, const mercury_table* const table2) { //returns true if every single value of the tables match. no, it's not recursive, because fuck that (also because refs).
	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* subt1=table1->data[i];
		mercury_subtable* subt2=table2->data[i];

		if (subt1->size != subt2->size) {
			return false;
		}

		for (mercury_int i1 = 0; i1 < subt1->size; i1++) { //not ideal. O(n^2)... this is what i get for not ordering anything.
			for (mercury_int i2 = 0; i2 < subt2->size; i2++) {
				if(mercury_vars_equal(subt1->keys+i1,subt2->keys+i2)) {
					goto found;
				}
			}
			return false;
			found:;
		}

	}
	return true;
}


bool mercury_table_get_cstring_keyvalue(const mercury_table* const table, const char* const key, mercury_variable* out) {
	const mercury_subtable* const subt = table->data[M_TYPE_STRING];
	for (mercury_int i = 0; i < subt->size; i++) {
		if(mercury_mstring_equal_cstring((mercury_string*)subt->keys[i].data.p,key)){
			mercury_clonevariable(subt->values+i, out);
			return true;
		}
	}
	out->type = M_TYPE_NIL;
	out->data.i = 0;
	return false;
}


mercury_int mercury_table_set_cstring_keyvalue(mercury_table* const table, const char* const key, const mercury_variable* const value) {
	mercury_subtable* subt = table->data[M_TYPE_STRING];
	for (mercury_int i = 0; i < subt->size; i++) {
		if (mercury_mstring_equal_cstring((mercury_string*)subt->keys[i].data.p,key)) {		
			mercury_free_var(subt->values+i);
			subt->values[i] = *value;
		}
	}

	void* nptr = realloc(subt->keys, sizeof(mercury_variable) * (subt->size + 1));
	if (nptr == nullptr) return -1;
	subt->keys = (mercury_variable*)nptr;
	nptr = realloc(subt->values, sizeof(mercury_variable) * (subt->size + 1));
	if (nptr == nullptr) return -1;
	subt->values = (mercury_variable*)nptr;

	mercury_variable kv;
	kv.type = M_TYPE_STRING;
	kv.data.p = mercury_cstring_const_to_mstring(key,strlen(key));
	subt->keys[subt->size] = kv;
	subt->values[subt->size] = *value;

	subt->size++;

	return subt->size - 1;
}

mercury_int mercury_table_has_cstring_key(const mercury_table* const table, const char* const key) {
	const mercury_subtable* const subt = table->data[M_TYPE_STRING];
	for (mercury_int i = 0; i < subt->size; i++) {
		if (mercury_mstring_equal_cstring((mercury_string*)subt->keys[i].data.p, key))return i;
	}
	return -1;
}


void mercury_prepare_table_for_state(mercury_table* table,mercury_state* M) {
	mercury_variable v;

	v.type = M_TYPE_TABLE;
	v.data.p = table;
	mercury_table_set_cstring_keyvalue(table, "_ENV", &v);

	v.data.p = M->masterstate ? M->masterstate->enviroment : table;
	mercury_table_set_cstring_keyvalue(table, "_G", &v);
	table->enviromental = true;
}

mercury_state* mercury_newstate(const mercury_state* const parent) {
	mercury_state* newstate=(mercury_state*)malloc(sizeof(mercury_state));
	if (newstate == nullptr) return nullptr;

	newstate->enviroment = mercury_newtable();
	if (newstate->enviroment == nullptr) {
		free(newstate);
		return nullptr;
	}
	newstate->enviroment->enviromental = true;
	newstate->enviroment->refrences = 0xFFFF;

	/*
	mercury_variable* envvarkey = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (!envvarkey)return nullptr;
	envvarkey->type = M_TYPE_STRING;
	envvarkey->data.p = mercury_cstring_const_to_mstring((char*)"_ENV",4);
	mercury_variable* envvarval = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (!envvarval)return nullptr;
	envvarval->type = M_TYPE_TABLE;
	envvarval->data.p = newstate->enviroment;
	mercury_setkey(newstate->enviroment, envvarkey, envvarval);
	*/


	if (!parent) {
		newstate->registers = (mercury_variable*)malloc(sizeof(mercury_variable) * (1u+register_max));
		if (newstate->registers == nullptr) {
			free(newstate);
			return nullptr;
		}
		for (mercury_int i = 0; i <= register_max; i++) {
			newstate->registers = nullptr;
		}
		newstate->masterstate = newstate;
		newstate->parentstate =  nullptr;
	}
	else {
		newstate->registers = parent->registers;
		newstate->masterstate = parent->masterstate;
		newstate->parentstate = (mercury_state*)parent;

		parent->enviroment->refrences+=1;
		parent->masterstate->enviroment->refrences += 1;
	}
	newstate->childstate = nullptr;

	newstate->constants = nullptr;
	newstate->num_constants = 0;

	/*
	mercury_variable* globvarkey = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (!globvarkey)return nullptr;
	globvarkey->type = M_TYPE_STRING;
	globvarkey->data.p = mercury_cstring_const_to_mstring((char*)"_G", 2);
	mercury_variable* globvarval = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (!globvarval)return nullptr;
	globvarval->type = M_TYPE_TABLE;
	globvarval->data.p = newstate->masterstate->enviroment;
	mercury_setkey(newstate->enviroment, globvarkey, globvarval);
	*/

	

	newstate->sizeofstack = 0;
	newstate->allocatedstacksize = 0;
	newstate->stack = nullptr;

	newstate->programcounter = 0;
	newstate->errorcode = 0;

	newstate->bytecode.enviromental = true;
	newstate->bytecode.instructions = nullptr;
	newstate->bytecode.numberofinstructions = 0;
	newstate->bytecode.refrences = 0xFFFF;
	newstate->bytecode.dbg_tokens = nullptr;
	newstate->bytecode.num_dbg_tokens = 0;
	newstate->bytecode.instruction_dbg_lookup = nullptr;

	newstate->enviroment = mercury_newtable();
	if (!newstate->enviroment)return nullptr;
	mercury_prepare_table_for_state(newstate->enviroment,newstate);
	

	//newstate->numberofinstructions = 0;
	//newstate->instructions = nullptr;


	return newstate;
}


bool mercury_stepstate(mercury_state* const M_CPP_restrict M) {
	if (M->programcounter >= M->bytecode.numberofinstructions) return false;

	mercury_opcode opcode = M->bytecode.instructions[M->programcounter];

	//printf("%i - %i %i\n", M->programcounter,iflags,opcode);
	M->programcounter++;
	mercury_bytecode_list[opcode](M);

	/*
	printf("post stack: %i\n", M->sizeofstack);
	for (mercury_int i = 0; i < M->sizeofstack; i++) {
		mercury_variable* v = M->stack[i];
		printf("\t%i [%i]: %i %f %p\n",i,v->type,v->data.i, v->data.f, v->data.p);
	}
	//*/

	return true;
}

void mercury_clearstate(mercury_state* const M_CPP_restrict M, bool for_deletion) {
	if (M->childstate) {
		if (for_deletion) {
			mercury_destroystate(M->childstate);
			M->childstate = nullptr;
		}
		else {
			mercury_clearstate(M->childstate, for_deletion);
		}
	}

	for (mercury_uint i = 0; i < M->sizeofstack; i++) {
		mercury_free_var(M->stack+i);
	}
	M->sizeofstack = 0;
	

	
	
	if (M->masterstate == M && M->registers) {
		for (mercury_uint i = 0; i < register_max; i++) {
			if (M->registers[i].type) {
				mercury_free_var(M->registers+i);
				M->registers[i].type = M_TYPE_NIL;
			}
		}
	}

	if (M->enviroment) {
		if (!for_deletion) {
			mercury_cleartable(M->enviroment);
			mercury_prepare_table_for_state(M->enviroment, M);
		}
		else {
			mercury_destroytable(M->enviroment);
			M->enviroment = nullptr;
		}
	}


	for (mercury_uint i = 0; i < M->num_constants; i++) {
		mercury_variable v = M->constants[i];
		mercury_free_var(&v);
	}
	M->num_constants = 0;

	M->programcounter = 0;
}

void mercury_destroystate(mercury_state* const M_CPP_restrict M) {
	mercury_clearstate(M,true);

	if (M->parentstate) {
		M->parentstate->enviroment->refrences -= 1;
		M->parentstate->masterstate->enviroment->refrences -= 1;
	}


	free(M->stack);

	//if (M->enviroment)mercury_destroytable(M->enviroment);

	if (M->bytecode.instructions) {
		free(M->bytecode.instructions);
	}

	if (M->bytecode.instruction_dbg_lookup) {
		free(M->bytecode.instruction_dbg_lookup);
	}
	if (M->bytecode.dbg_tokens) {
		for (mercury_uint i = 0; i < M->bytecode.num_dbg_tokens; i++) {
			free(M->bytecode.dbg_tokens[i].chars);
		}
		free(M->bytecode.dbg_tokens);
	}

	if (M->masterstate == M && M->registers) {
		free(M->registers);
	}

	free(M);
}


void mercury_free_var(mercury_variable* const M_CPP_restrict var) {
	switch (var->type)
	{
	case M_TYPE_TABLE:
	{
		mercury_table* ftab = (mercury_table*)var->data.p;
		ftab->refrences--;
		if (!ftab->refrences && !ftab->enviromental) {

#ifdef MERCURY_DEBUG
			if (ftab->enviromental) {
				printf("enviromental table %p marked for freeing. something has gone terribly worng. probably.\n",ftab);
			}
#endif
			mercury_destroytable(ftab);
		}
	}
		break;
	case M_TYPE_STRING:
	{
		mercury_string* str = (mercury_string*)var->data.p;
		str->refrences--;
		if (!str->refrences) {
			mercury_mstring_delete(str);
		}
	}
		break;
	case M_TYPE_ARRAY:
		{
		mercury_array* farray = (mercury_array*)var->data.p; //get the array
		farray->refrences--;
		if (!farray->refrences) { //if this is the last refrence, destroy all
			mercury_destroyarray(farray);
		}
		}
		break;
	case M_TYPE_FUNCTION:
		{
		mercury_function* ffunction = (mercury_function*)var->data.p;
		ffunction->refrences--;
		if (!ffunction->refrences) {
			free(ffunction->instructions); //this causes a heap issue. dunno why.
			if (ffunction->instruction_dbg_lookup) {
				free(ffunction->instruction_dbg_lookup);
			}
			if (ffunction->dbg_tokens) {
				for (mercury_uint i = 0; i < ffunction->num_dbg_tokens; i++) {
					free(ffunction->dbg_tokens[i].chars);
				}
				free(ffunction->dbg_tokens);
			}
			free(ffunction);
		}
		}
		break;
	case M_TYPE_FILE:
	{
		mercury_filewrapper* fw = (mercury_filewrapper*)var->data.p;
		fw->refrences--;
		if (!fw->refrences) {
			if (fw->open)fclose(fw->file);
			free(fw);
		}
	}
		break;
	case M_TYPE_THREAD:
	{
		mercury_threadholder* t = (mercury_threadholder*)var->data.p;
		t->refrences--;
		if (!t->refrences) {
			if (!t->finished) { //you stupid son of a bitch why are you like this?
#if defined(_WIN32) || defined(_WIN64)
				WaitForSingleObject(t->threadobject, INFINITE);
				CloseHandle(t->threadobject);
				t->threadobject = NULL;
#else
				pthread_join(t->threadobject, NULL);
				t->threadobject = NULL;
#endif
			}
			if (t->customenv) {
				t->state->enviroment = nullptr;
			}
			//t->state->bytecode.instructions = nullptr;
			mercury_destroystate(t->state);
			free(t);
		}

		
	}
		break;
	default:
		break;
	}
}

void mercury_popstack(mercury_state* const M_CPP_restrict M, mercury_variable* out) {
	if (M->sizeofstack==0) {
		out->type = M_TYPE_NIL;
		out->data.i = 0;
		return;
	}
	M->sizeofstack--;
	*out=  M->stack[M->sizeofstack];
}

//takes from the bottom instead of the top of stack
void mercury_pullstack(mercury_state* const M_CPP_restrict M, mercury_variable* out) {
	if (M->sizeofstack == 0) {
		out->type = M_TYPE_NIL;
		out->data.i = 0;
		return;
	}

	*out = M->stack[0];
	M->sizeofstack--;
	memmove(M->stack, M->stack + 1, sizeof(mercury_variable) * M->sizeofstack);
}


bool mercury_pushstack(mercury_state* const M_CPP_restrict M, mercury_variable* const var) {
	if (!(M->allocatedstacksize >M->sizeofstack)) {
		void* nstackptr = realloc(M->stack, (M->sizeofstack + 1) * sizeof(mercury_variable));
		if (nstackptr == nullptr) return false;
		M->stack = (mercury_variable*)nstackptr;

		M->allocatedstacksize = M->sizeofstack + 1;
	}

	M->stack[M->sizeofstack] = *var;
	M->sizeofstack++;

	switch (var->type) {
	case M_TYPE_STRING:
		((mercury_string*)var->data.p)->refrences++;
		break;
	case M_TYPE_ARRAY:
	{
		mercury_array* a = (mercury_array*)var->data.p;
		a->refrences++;
	}
		break;
	case M_TYPE_TABLE:
	{
		mercury_table* t = (mercury_table*)var->data.p;
		t->refrences++;
	}
		break;
	case M_TYPE_FUNCTION:
	{
		mercury_function* f = (mercury_function*)var->data.p;
		f->refrences++;
	}
		break;
	case M_TYPE_FILE:
	{
		mercury_filewrapper* w = (mercury_filewrapper*)var->data.p;
		w->refrences++;
	}
		break;
	case M_TYPE_THREAD:
	{
		mercury_threadholder* t = (mercury_threadholder*)var->data.p;
		t->refrences++;
	}
		break;
	}

	return true;
}

//does not increment the refcounter of the variable.
bool mercury_pushstack_unrefed(mercury_state* const M_CPP_restrict M, mercury_variable* const var) {
	if (!(M->allocatedstacksize > M->sizeofstack)) {
		void* nstackptr = realloc(M->stack, (M->sizeofstack + 1) * sizeof(mercury_variable));
		if (nstackptr == nullptr) return false;
		M->stack = (mercury_variable*)nstackptr;

		M->allocatedstacksize = M->sizeofstack + 1;
	}

	M->stack[M->sizeofstack] = *var;
	M->sizeofstack++;
	return true;
}

void mercury_clonevariable(const mercury_variable* const var, mercury_variable* out) {
	out->type = var->type;
	switch (out->type) {
		case M_TYPE_STRING:
			//out->data.p = mercury_copystring((mercury_string*)var->data.p);
			((mercury_string*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		case M_TYPE_TABLE:
			((mercury_table*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		case M_TYPE_ARRAY:
			((mercury_array*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		case M_TYPE_FILE:
			((mercury_filewrapper*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		case M_TYPE_THREAD:
			((mercury_threadholder*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		case M_TYPE_FUNCTION:
			((mercury_function*)var->data.p)->refrences++;
			out->data = var->data;
			break;
		default:
			out->data = var->data;
	}
}


mercury_array* mercury_newarray() {
	mercury_array* nar = (mercury_array*)malloc(sizeof(mercury_array));
	if (nar == nullptr)return nullptr;

	nar->refrences = 1;
	nar->values = nullptr;

	return nar;
}

void mercury_destroyarray(mercury_array* const M_CPP_restrict arr) {
	if (arr->values) {
#ifdef MERCURY_64BIT
		//this can't be the best way to do it. i mean... just look at this piece of shit.
		for (int i1 = (MERCURY_SIZE_SUBARRAY_1 - 1); i1 >= 0; i1--) {
			mercury_variable***** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = (MERCURY_SIZE_SUBARRAY_2 - 1); i2 >= 0; i2--) {
				mercury_variable**** const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = (MERCURY_SIZE_SUBARRAY_3 - 1); i3 >= 0; i3--) {
					mercury_variable*** const st3 = st2[i3];
					if (!st3)continue;
					for (int i4 = (MERCURY_SIZE_SUBARRAY_4 - 1); i4 >= 0; i4--) {
						mercury_variable** const st4 = st3[i4];
						if (!st4)continue;
						for (int i5 = (MERCURY_SIZE_SUBARRAY_5 - 1); i5 >= 0; i5--) {
							mercury_variable* const st5 = st4[i5];
							if (!st5)continue;
							for (int i6 = (MERCURY_SIZE_SUBARRAY_6 - 1); i6 >= 0; i6--) {
								mercury_variable* var = st5+i6;
								if (var->type)mercury_free_var(var);
							}
							free(st5);
						}
						free(st4);
					}
					free(st3);
				}
				free(st2);
			}
			free(st1);
		}
#else
		//it's less shit here but still not great.
		for (int i1 = (MERCURY_SIZE_SUBARRAY_1 - 1); i1 >= 0; i1--) {
			mercury_variable** const st1 = arr->values[i1];
			if (!st1)continue;
			for (int i2 = (MERCURY_SIZE_SUBARRAY_2 - 1); i2 >= 0; i2--) {
				mercury_variable* const st2 = st1[i2];
				if (!st2)continue;
				for (int i3 = (MERCURY_SIZE_SUBARRAY_3 - 1); i3 >= 0; i3--) {
					mercury_variable* const var = st2+i3;
					if (var->type)mercury_free_var(var);
				}
				free(st2);
			}
			free(st1);
		}
#endif
	}
	free(arr->values);
	free(arr);
}

bool mercury_setarray(mercury_array* const array, const mercury_variable* const var, const mercury_int pos) {
#ifdef MERCURY_64BIT
	if (!array->values) {
		array->values = (mercury_variable******)calloc(MERCURY_SIZE_SUBARRAY_1, sizeof(void*));
		if (!array->values) {
			return false;
		}
	}

	int current_subindex = get_array_index_from_mint_1(pos);
	mercury_variable *****sa1 = array->values[current_subindex];
	if (!sa1) {
		sa1=(mercury_variable*****)calloc(MERCURY_SIZE_SUBARRAY_2, sizeof(void*));
		if (!sa1) {
			return false;
		}
		array->values[current_subindex]=sa1;
	}
	
	current_subindex = get_array_index_from_mint_2(pos);
	mercury_variable**** sa2 = sa1[current_subindex];
	if (!sa2) {
		sa2 = (mercury_variable****)calloc(MERCURY_SIZE_SUBARRAY_3, sizeof(void*));
		if (!sa2) {
			return false;
		}
		sa1[current_subindex] = sa2;
	}

	current_subindex = get_array_index_from_mint_3(pos);
	mercury_variable*** sa3 = sa2[current_subindex];
	if (!sa3) {
		sa3 = (mercury_variable***)calloc(MERCURY_SIZE_SUBARRAY_4, sizeof(void*));
		if (!sa3) {
			return false;
		}
		sa2[current_subindex] = sa3;
	}

	current_subindex = get_array_index_from_mint_4(pos);
	mercury_variable** sa4 = sa3[current_subindex];
	if (!sa4) {
		sa4 = (mercury_variable**)calloc(MERCURY_SIZE_SUBARRAY_5, sizeof(void*));
		if (!sa4) {
			return false;
		}
		sa3[current_subindex] = sa4;
	}

	current_subindex = get_array_index_from_mint_5(pos);
	mercury_variable* sa5 = sa4[current_subindex];
	if (!sa5) {
		sa5 = (mercury_variable*)calloc(MERCURY_SIZE_SUBARRAY_6, sizeof(mercury_variable));
		if (!sa5) {
			return false;
		}
		sa4[current_subindex] = sa5;
	}

	current_subindex = get_array_index_from_mint_6(pos);
	mercury_variable* arrvar = sa5+current_subindex;
	if (arrvar->type) {
		mercury_free_var(arrvar);
	}
	sa5[current_subindex] = *var;
#else
	if (!array->values) {
		array->values = (mercury_variable***)calloc(MERCURY_SIZE_SUBARRAY_1, sizeof(void*));
		if (!array->values) {
			return false;
		}
	}

	int current_subindex = get_array_index_from_mint_1(pos);
	mercury_variable** sa1 = array->values[current_subindex];
	if (!sa1) {
		sa1 = (mercury_variable**)calloc(MERCURY_SIZE_SUBARRAY_2, sizeof(void*));
		if (!sa1) {
			return false;
		}
		array->values[current_subindex] = sa1;
	}

	current_subindex = get_array_index_from_mint_2(pos);
	mercury_variable* sa2 = sa1[current_subindex];
	if (!sa2) {
		sa2 = (mercury_variable*)calloc(MERCURY_SIZE_SUBARRAY_3, sizeof(mercury_variable));
		if (!sa2) {
			return false;
		}
		sa1[current_subindex] = sa2;
	}

	current_subindex = get_array_index_from_mint_3(pos);
	mercury_variable* arrvar = sa2+current_subindex;
	if (arrvar->type) {
		mercury_free_var(arrvar);
	}
	sa2[current_subindex] = *var;
#endif
	return true;
}

void mercury_getarray(mercury_array* const array, const mercury_int pos, mercury_variable* out) {
	if (!array->values)goto no_index;
#ifdef MERCURY_64BIT
	{
		int current_subindex = get_array_index_from_mint_1(pos);
		mercury_variable***** sa1 = array->values[current_subindex];
		if (!sa1)goto no_index;
		current_subindex = get_array_index_from_mint_2(pos);
		mercury_variable**** sa2 = sa1[current_subindex];
		if (!sa2)goto no_index;
		current_subindex = get_array_index_from_mint_3(pos);
		mercury_variable*** sa3 = sa2[current_subindex];
		if (!sa3)goto no_index;
		current_subindex = get_array_index_from_mint_4(pos);
		mercury_variable** sa4 = sa3[current_subindex];
		if (!sa4)goto no_index;
		current_subindex = get_array_index_from_mint_5(pos);
		mercury_variable* sa5 = sa4[current_subindex];
		if (!sa5)goto no_index;
		current_subindex = get_array_index_from_mint_6(pos);
		mercury_variable* var = sa5+current_subindex;
		if (var->type) {
			mercury_clonevariable(var, out);
			return;
		}
	}
#else
	{
		int current_subindex = get_array_index_from_mint_1(pos);
		mercury_variable** sa1 = array->values[current_subindex];
		if (!sa1)goto no_index;
		current_subindex = get_array_index_from_mint_2(pos);
		mercury_variable* sa2 = sa1[current_subindex];
		if (!sa2)goto no_index;
		current_subindex = get_array_index_from_mint_3(pos);
		mercury_variable* var = sa2+current_subindex;
		if (var) { 
			mercury_clonevariable(var, out); 
			return; 
		}
	}
#endif
	no_index:
	out->data.i = 0;
	out->type = M_TYPE_NIL;
}

mercury_int mercury_array_len(const mercury_array* const M_CPP_restrict arr) {
	if (!arr->values)return -1;
	mercury_int out = 0;
#ifdef MERCURY_64BIT
	//this can't be the best way to do it. i mean... just look at this piece of shit.
	for (int i1 = (MERCURY_SIZE_SUBARRAY_1 - 1) >> 1; i1>=0; i1--) { //bitshift right once because we are ignoring negative values, and those start with 1
		mercury_variable***** const st1 = arr->values[i1];
		if (!st1)continue;
		for (int i2 = (MERCURY_SIZE_SUBARRAY_2 - 1); i2 >= 0; i2--) {
			mercury_variable**** const st2 = st1[i2];
			if (!st2)continue;
			for (int i3 = (MERCURY_SIZE_SUBARRAY_3 - 1); i3 >= 0; i3--) {
				mercury_variable*** const st3 = st2[i3];
				if (!st3)continue;
				for (int i4 = (MERCURY_SIZE_SUBARRAY_4 - 1); i4 >= 0; i4--) {
					mercury_variable** const st4 = st3[i4];
					if (!st4)continue;
					for (int i5 = (MERCURY_SIZE_SUBARRAY_5 - 1); i5 >= 0; i5--) {
						mercury_variable* const st5 = st4[i5];
						if (!st5)continue;
						for (int i6 = (MERCURY_SIZE_SUBARRAY_6 - 1); i6 >= 0; i6--) {
							const mercury_variable* const var = st5+i6;
							if (var->type)return mercury_reconstruct_array_index(i1,i2,i3,i4,i5,i6);
						}
					}
				}
			}
		}
	}
#else
	//it's less shit here but still not great.
	for (int i1 = (MERCURY_SIZE_SUBARRAY_1 - 1) >> 1; i1 > 0; i1--) { //bitshift right once because we are ignoring negative values, and those start with 1
		mercury_variable** const st1 = arr->values[i1];
		if (!st1)continue;
		for (int i2 = (MERCURY_SIZE_SUBARRAY_2 - 1); i2 > 0; i2--) {
			mercury_variable* const st2 = st1[i2];
			if (!st2)continue;
			for (int i3 = (MERCURY_SIZE_SUBARRAY_3 - 1); i3 > 0; i3--) {
				const mercury_variable* const var = st2+i3;
				if (var->type)return mercury_reconstruct_array_index(i1, i2, i3);
			}
		}
	}
#endif
	return -1;
}




mercury_string* mercury_tostring(const mercury_variable* const M_CPP_restrict var) {
	mercury_string* tstr=nullptr;

	char tout[256];
	for (int i = 0; i < 256; i++) {
		tout[i] = '\0';
	}
	int tint;

	switch (var->type) {
	case M_TYPE_NIL:
		tstr = mercury_cstring_const_to_mstring((char*)"nil", 3);
		break;
	case M_TYPE_INT:
		tint=snprintf( tout,sizeof(tout),"%zi",var->data.i);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout) );
		break;
	case M_TYPE_FLOAT:
		tint = snprintf(tout, sizeof(tout), "%#.10g", var->data.f);	
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_BOOL:
		if (var->data.i) {
			tstr = mercury_cstring_const_to_mstring((char*)"true", 4);
		}
		else {
			tstr = mercury_cstring_const_to_mstring((char*)"false", 5);
		}
		break;
	case M_TYPE_TABLE:
		tint = snprintf(tout, sizeof(tout), "table %p", var->data.p);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_STRING:
		tstr = mercury_copystring((mercury_string*)var->data.p);
		break;
	case M_TYPE_CFUNC:
		tint = snprintf(tout, sizeof(tout), "c function %p", var->data.p);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_FUNCTION:
		tint = snprintf(tout, sizeof(tout), "function %p", var->data.p);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_ARRAY:
		tint = snprintf(tout, sizeof(tout), "array %p", var->data.p);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	case M_TYPE_FILE:
		tint = snprintf(tout, sizeof(tout), "file %p", var->data.p);
		if (tint==-1) {
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		break;
	default:
		tstr = mercury_cstring_const_to_mstring((char*)"unknown", 7);
	}
	
	return tstr;
}

bool mercury_checkbool(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_NIL:
		return false;
	case M_TYPE_BOOL:
		if (var->data.i) {
			return true;
		}
		else {
			return false;
		}
	case M_TYPE_INT:
		return var->data.i != 0;
	default:
		return true;
	}
}

//type coersion when you really need it
mercury_int mercury_checkint(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_NIL:
		return 0;
	case M_TYPE_BOOL:
		if (var->data.i) {
			return 1;
		}
		else {
			return 0;
		}
	case M_TYPE_INT:
		return var->data.i;
	case M_TYPE_FLOAT:
		return (mercury_int)var->data.f;
	default:
		return 0;
	}
}
//see above
mercury_float mercury_checkfloat(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_NIL:
		return 0.0;
	case M_TYPE_BOOL:
		if (var->data.i) {
			return 1.0;
		}
		else {
			return 0.0;
		}
	case M_TYPE_INT:
		return (mercury_float)var->data.i;
	case M_TYPE_FLOAT:
		return var->data.f;
	default:
		return 0.0;
	}
}
//ditto
void* mercury_checkpointer(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_NIL:
	case M_TYPE_BOOL:
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
		return nullptr;
	default:
		return var->data.p;
	}
}


bool mercury_vars_equal(const mercury_variable* const var1, const mercury_variable* const var2) {
	if (var1->type != var2->type) {
		return false;
	}

	if (var1->type == M_TYPE_STRING) {
		return var1->data.i==var2->data.i || mercury_mstrings_equal( (mercury_string*)var1->data.p, (mercury_string*)var2->data.p);
	}
	else {
		return var1->data.i == var2->data.i;
	}
}


void mercury_debugdumptable(mercury_table* tab,int level) {
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable* subt = tab->data[t];
		for (mercury_int i = 0; i < subt->size; i++) {
			for (int n = 0; n < level; n++) {
				putchar('\t');
			}
			mercury_rawdata keydat = subt->keys[i].data;
			switch (t) {
				case M_TYPE_NIL:
					printf("nil_%zi", keydat.i);
					break;
				case M_TYPE_INT:
					printf("%zi", keydat.i);
					break;
				case M_TYPE_FLOAT:
					printf("%f", keydat.f);
					break;
				case M_TYPE_STRING:
					{
					mercury_string* str = (mercury_string*)keydat.p;
					putchar('\"');
					for (mercury_int i2 = 0; i2 < str->size; i2++) {
						putchar(str->ptr[i2]);
						}
					}
					putchar('\"');
					break;
				default:
					printf("0x%p", keydat.p);
			}

			printf(" - ");

			mercury_variable* var = subt->values+i;

			switch (var->type) {
				case M_TYPE_INT:
					printf("%zi", var->data.i);
					break;
				case M_TYPE_FLOAT:
					printf("%f", var->data.f);
					break;
				case M_TYPE_TABLE:
					if (var->data.p != tab) {
						printf("TABLE 0x%p:\n", var->data.p);
						mercury_debugdumptable((mercury_table*)var->data.p, level + 1);
					}
					else {
						printf("TABLE <self> 0x%p", var->data.p);
					}
					break;
				case M_TYPE_ARRAY:
					printf("ARRAY 0x%p", var->data.p);
					{
						mercury_array* arr = (mercury_array*)var->data.p;
						mercury_int l=mercury_array_len(arr)+1;
						for (mercury_int q = 0; q < l; q++) {
							putchar('\n');
							mercury_variable v;
							mercury_getarray(arr, q,&v);
							for (int n = 0; n < level + 1; n++) {
								putchar('\t');
							}
							switch (v.type)
							{
							case M_TYPE_NIL:
								printf("%zi - nil", q);
								break;
							case M_TYPE_INT:
								printf("%zi - %zi", q, v.data.i);
								break;
							case M_TYPE_FLOAT:
								printf("%zi - %f", q, v.data.f);
								break;
							case M_TYPE_STRING:
							{
								mercury_string* sp = (mercury_string*)v.data.p;
							
								printf("%zi - ", q);
								putchar('\"');
								for (mercury_int sc = 0; sc < sp->size; sc++) {
									putchar(sp->ptr[sc]);
								}
								putchar('\"');
								
							}
								break;
							default:
								printf("%zi - [%hhu] 0x%p",q,v.type ,v.data.p);
								break;
							}
						}
					}
					break;
				case M_TYPE_STRING:
					{
						mercury_string* str = (mercury_string*)var->data.p;
						putchar('\"');
						for (mercury_int i2 = 0; i2 < str->size; i2++) {
							putchar(str->ptr[i2]);
						}
						putchar('\"');
					}
					break;
				default:
					printf("[%hhu] 0x%p",var->type, var->data.p);
			}

			
			

			putchar('\n');
		}
	}


}

void mercury_clone_function(mercury_function* in, mercury_function* out) {
	out->numberofinstructions = in->numberofinstructions - 1;// guard value. this will be overwritten later as long as the function executes correctly and these should be the same number.
	out->refrences = 1;
	if (in->instruction_dbg_lookup) {
		out->instruction_dbg_lookup = (mercury_uint*)malloc(sizeof(mercury_uint) * in->numberofinstructions);
		if (!out->instruction_dbg_lookup)return;
		out->dbg_tokens = (mercury_debug_token*)malloc(sizeof(mercury_debug_token) * in->num_dbg_tokens);
		if (!out->dbg_tokens)return;
		out->num_dbg_tokens = in->num_dbg_tokens;
		for (mercury_uint i = 0; i < in->num_dbg_tokens; i++) {
			out->dbg_tokens[i] = in->dbg_tokens[i];
			out->dbg_tokens[i].chars = (char*)malloc(in->dbg_tokens[i].num_chars);
			if (out->dbg_tokens[i].chars) {
				memcpy(out->dbg_tokens[i].chars, in->dbg_tokens[i].chars, in->dbg_tokens[i].num_chars);
			}
		}
		memcpy(out->instruction_dbg_lookup, in->instruction_dbg_lookup, sizeof(mercury_uint) * in->numberofinstructions);

	}
	else {
		out->instruction_dbg_lookup = nullptr;
		out->num_dbg_tokens = 0;
		out->dbg_tokens = nullptr;
	}
	out->instructions = (mercury_opcode*)malloc(sizeof(mercury_opcode) * in->numberofinstructions);
	if (!out->instructions)return;
	memcpy(out->instructions, in->instructions, sizeof(mercury_opcode) * in->numberofinstructions);
	out->numberofinstructions = in->numberofinstructions;
	out->enviromental = false;
	out->refrences = 1;
}

inline const char* m_get_opcode_str(mercury_opcode instruction) {

	if (instruction > M_OPCODE_SWXY) {
		return "????";
	}
	static const char* lookup[0xFFFF] = {
		" NOP", //0

		" ADD", //1
		" SUB", //2
		" MUL", //3
		" DIV", //4
		" POW", //5
		"IDIV", //6
		" MOD", //7

		"BAND", //8
		"BOR ", //9
		"BXOR", //10
		"BNOT", //11
		"BSHL", //12
		"BSHR", //13

		"LAND", //14
		"LOR ", //15
		"LXOR", //16
		"LNOT", //17

		"EQL ", //18
		"NEQ ", //19
		"GRT ", //20
		"LET ", //21
		"GTE ", //22
		"LTE ", //23

		"SENV", //24
		"GENV", //25
		"SET ", //26
		"GET ", //27
		"SREG", //28
		"GREG", //29

		"NINT", //30
		"NFLO", //31
		"NTRU", //32
		"NFAL", //33
		"NNIL", //34
		"NSTR", //35
		"NFUN", //36
		"NTAB", //37
		"NARR", //38

		"JMP ", //39
		"JMPR", //40
		"JIF ", //41
		"JNIF", //42
		"JRIF", //43
		"JRNI", //44

		"CALL", //45
		"EXIT", //46
		"LEN ", //47
		"CNCT", //48
		"CLS ", //49
		"GETL", //50
		"SETL", //51
		"GETG", //52
		"SETG", //53

		"CPYT", //54
		"SWPT", //55
		"CPYX", //56

		"UNM ", //57
		"INC ", //58
		"DEC ", //59

		"SCON", //60
		"GCON", //61

		"SWXY", //62
	};
	return lookup[instruction];
}

mercury_string* mercury_get_bytecode_debug(mercury_function* F) {
	mercury_string* out= (mercury_string*)malloc(sizeof(mercury_string));
	if (!out)return nullptr;
	out->constant = false;
	out->ptr = nullptr;
	out->size = 0;

	mercury_uint offset = 0;
	while (offset < F->numberofinstructions) {
		mercury_opcode instruction =F->instructions[offset];
		offset++;
		char buffer[0x2FFF];

#ifdef MERCURY_64BIT
	snprintf(buffer, 0x2FFF, "[%016zX - %04hX] %s", offset - 1, instruction, m_get_opcode_str(instruction));
#else
	snprintf(buffer, 0x2FFF, "[%08zX - %04hX] %s", offset - 1, instruction, m_get_opcode_str(instruction));
#endif
		
		mercury_mstring_addchars(out, buffer, strlen(buffer) );

		switch (instruction) {
			case M_OPCODE_NINT: //static var inputs
				snprintf(buffer, 0x2FFF, " %zi", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_NFLO:
				snprintf(buffer, 0x2FFF, " %f", *(mercury_float*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_NSTR:
				{
				mercury_int size = *(mercury_int*)(F->instructions + offset);
				snprintf(buffer, 0x2FFF, " size:%zi ", size );
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				size = size >= 0x2FFF ? 0x2FFF : size;
				for (mercury_int i = 0; i < size; i++) {
					char c = *(((char*)(F->instructions + offset)) + i);
					buffer[i] = c;
				}
				offset += (size + sizeof(mercury_opcode)-1) / sizeof(mercury_opcode);
				mercury_mstring_addchars(out, (char*)"\"", 1);
				mercury_mstring_addchars(out, buffer, size);
				mercury_mstring_addchars(out, (char*)"\"", 1);
				}
				break;
			case M_OPCODE_NFUN:
				snprintf(buffer, 0x2FFF, " instructions:%zi ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_CALL:
				snprintf(buffer, 0x2FFF, " in:%zi ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;	
				snprintf(buffer, 0x2FFF, " out:%zi ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_JMP: //gaurentee takes 1, int
			case M_OPCODE_JMPR:
			case M_OPCODE_JIF:
			case M_OPCODE_JNIF:
			case M_OPCODE_JRIF:
			case M_OPCODE_JRNI:
				snprintf(buffer, 0x2FFF, " offset:%zi ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_CPYX:
			case M_OPCODE_SCON:
			case M_OPCODE_GCON:
				snprintf(buffer, 0x2FFF, " %zi ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
			case M_OPCODE_SWXY:
				snprintf(buffer, 0x2FFF, " %zu ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				snprintf(buffer, 0x2FFF, " %zu ", *(mercury_int*)(F->instructions + offset));
				mercury_mstring_addchars(out, buffer, strlen(buffer));
				offset += MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE;
				break;
		}

		mercury_mstring_addchars(out,(char*)"\n",1);
	}
	return out;
}

//same as above, but does not auto-advance or detect variables for alternate debugging
mercury_string* mercury_get_bytecode_rawbinary_debug(mercury_function* F) {
	mercury_string* out = (mercury_string*)malloc(sizeof(mercury_string));
	if (!out)return nullptr;
	out->constant = false;
	out->ptr = nullptr;
	out->size = 0;

	mercury_uint offset = 0;
	while (offset < F->numberofinstructions) {
		mercury_opcode instruction = F->instructions[offset];
		offset++;
		char buffer[0x2FFF];

		char charbuffer[3]="\0\0";

		charbuffer[0]= instruction & 0xFF;
		charbuffer[0] = (charbuffer[0] < '\x20') ? charbuffer[0] = '\x20' : charbuffer[0]; //prevent control characters messing everything up, since this normally gets printed to stdout.
		charbuffer[1]= (instruction >> 8) & 0xFF;
		charbuffer[1] = (charbuffer[1] < '\x20') ? charbuffer[1] = '\x20' : charbuffer[1];

#ifdef MERCURY_64BIT
		snprintf(buffer, 0x2FFF, "[%016zX - %04hX\t%s\t%hi\t%hu\t%s]", offset - 1, instruction, m_get_opcode_str(instruction), instruction, instruction,charbuffer);
#else
		snprintf(buffer, 0x2FFF, "[%08zX - %04hX\t%s\t%hi\t%hu\t%s]", offset - 1, instruction, m_get_opcode_str(instruction), instruction, instruction, charbuffer);
#endif

		mercury_mstring_addchars(out, buffer, strlen(buffer));
		mercury_mstring_addchars(out, (char*)"\n", 1);
	}
	return out;
}


bool mercury_register_library(void* data, const char* key, const char* table,uint8_t type=M_TYPE_CFUNC) {
	void* nreg=realloc(M_LIBS,sizeof(mercury_libdef*)*(M_NUM_LIBS+1) );
	if (!nreg)return false;
	M_LIBS = (mercury_libdef**)nreg;

	mercury_libdef* ndef = (mercury_libdef*)malloc(sizeof(mercury_libdef));
	if (!ndef)return false;
	ndef->dataptr = data;
	ndef->key = key;
	ndef->table = table;
	ndef->type = type;

	M_LIBS[M_NUM_LIBS] = ndef;
	M_NUM_LIBS++;
	return true;
}

mercury_libdef** M_LIBS = nullptr;
mercury_int M_NUM_LIBS = 0;




void mercury_populate_enviroment_with_libs(mercury_state* M) {

	for (mercury_int i = 0; i < M_NUM_LIBS; i++) {
		mercury_libdef* lib = M_LIBS[i];
		
		mercury_variable v;
		v.type = lib->type;
		switch (lib->type)
		{
		case M_TYPE_FLOAT:
			v.data.f = *(mercury_float*)lib->dataptr;
			break;
		case M_TYPE_INT:
		case M_TYPE_BOOL:
			v.data.i = *(mercury_int*)lib->dataptr;
			break;
		case M_TYPE_CFUNC:
			v.data.p = (mercury_cfunc)lib->dataptr;
			break;
		case M_TYPE_NIL:
			v.data.p = lib->dataptr;
			break;
		default:
			continue;
		}

		if (lib->table) {
			mercury_variable k;
			k.type = M_TYPE_STRING;
			k.data.p = mercury_cstring_to_mstring(lib->key, strlen(lib->key));
			if (!k.data.p)continue;

			mercury_variable tidx;
			tidx.type = M_TYPE_STRING;
			mercury_string* tidxstr= mercury_cstring_to_mstring(lib->table, strlen(lib->table));
			tidxstr->refrences++;
			tidx.data.p = tidxstr;
			mercury_variable t;
			mercury_getkey(M->enviroment, &tidx,&t);

			if (t.type == M_TYPE_TABLE) {
				mercury_free_var(&tidx);
				mercury_setkey((mercury_table*)t.data.p,&k,&v);
			}
			else {
				mercury_table* nt=mercury_newtable();

				mercury_setkey(nt, &k, &v);

				mercury_variable vv;
				vv.type = M_TYPE_TABLE;
				vv.data.p = nt;

				mercury_setkey(M->enviroment, &tidx, &vv);
			}


		}
		else {
			mercury_variable k;
			k.type = M_TYPE_STRING;
			k.data.p = mercury_cstring_to_mstring((char*)lib->key,strlen(lib->key) );

			mercury_setkey(M->enviroment, &k, &v);
		}


	}


}


//use dll loading stuff to register libraries. simple and easy.

#ifdef _WIN32
BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
#else
static void __attribute__((constructor)) dynamic_lib_load() {
#endif

	static const mercury_int v = MERCURY_VERSION;
	mercury_register_library((void*)&v, "_VERSION", nullptr,M_TYPE_INT);
	static const mercury_int v_p = MERCURY_VERSION_PATCH;
	mercury_register_library((void*)&v_p, "_VERSION_PATCH", nullptr, M_TYPE_INT);

#ifdef MERCURY_LIB_STD
	mercury_register_library(mercury_lib_std_print, "print", nullptr);
	mercury_register_library(mercury_lib_std_iterate, "iterate", nullptr);
	mercury_register_library(mercury_lib_std_restricted_call, "rcall", nullptr);
	mercury_register_library(mercury_lib_std_dump, "dump", nullptr);
	mercury_register_library(mercury_lib_std_compile, "compile", nullptr);
	mercury_register_library(mercury_lib_std_type, "type", nullptr);
	mercury_register_library(mercury_lib_std_tostring, "tostring", nullptr);
	mercury_register_library(mercury_lib_std_tonumber, "tonumber", nullptr);
	mercury_register_library(mercury_lib_std_toint, "toint", nullptr);
	mercury_register_library(mercury_lib_std_tofloat, "tofloat", nullptr);
	mercury_register_library(mercury_lib_std_dynamic_library_load, "loadlibrary", nullptr);
	mercury_register_library(mercury_lib_std_deepcopy, "deepcopy", nullptr);
	mercury_register_library(mercury_lib_std_error, "error", nullptr);


	mercury_register_library((void*)&m_const_type_nil, "TYPE_NIL", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_int, "TYPE_INT", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_float, "TYPE_FLOAT", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_bool, "TYPE_BOOL", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_table, "TYPE_TABLE", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_string, "TYPE_STRING", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_cfunc, "TYPE_CFUNC", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_array, "TYPE_ARRAY", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_function, "TYPE_FUNCTION", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_file, "TYPE_FILE", nullptr, M_TYPE_INT);
	mercury_register_library((void*)&m_const_type_thread, "TYPE_THREAD", nullptr, M_TYPE_INT);
#endif

#ifdef MERCURY_LIB_MATH
	mercury_register_library(mercury_lib_math_max, "max", "math");
	mercury_register_library(mercury_lib_math_min, "min", "math");
	mercury_register_library(mercury_lib_math_floor, "floor", "math");
	mercury_register_library(mercury_lib_math_ceil, "ceil", "math");
	mercury_register_library(mercury_lib_math_to_radians, "radians", "math");
	mercury_register_library(mercury_lib_math_to_degrees, "degrees", "math");
	mercury_register_library(mercury_lib_math_log, "log", "math");
	mercury_register_library(mercury_lib_math_to_absolute, "abs", "math");
	mercury_register_library(mercury_lib_math_to_sin, "sin", "math");
	mercury_register_library(mercury_lib_math_to_cos, "cos", "math");
	mercury_register_library(mercury_lib_math_to_tan, "tan", "math");
	mercury_register_library(mercury_lib_math_to_asin, "asin", "math");
	mercury_register_library(mercury_lib_math_to_acos, "acos", "math");
	mercury_register_library(mercury_lib_math_to_atan, "atan", "math");
	mercury_register_library(mercury_lib_math_to_atan2, "atan2", "math");
	mercury_register_library(mercury_lib_math_random, "random", "math");
	mercury_register_library(mercury_lib_math_randomint, "randomint", "math");
	mercury_register_library(mercury_lib_math_randomseed, "randomseed", "math");
	mercury_register_library(mercury_lib_math_isnan, "isnan", "math");
	mercury_register_library(mercury_lib_math_max_array, "amax", "math");
	mercury_register_library(mercury_lib_math_min_array, "amin", "math");
	mercury_register_library(mercury_lib_math_mean, "mean", "math");
	mercury_register_library(mercury_lib_math_mean_array, "amean", "math");

	mercury_register_library((void*)&m_math_pi, "pi", "math", M_TYPE_FLOAT);
	mercury_register_library((void*)&m_math_root2, "root2", "math", M_TYPE_FLOAT);
	mercury_register_library((void*)&m_math_e, "e", "math", M_TYPE_FLOAT);
	mercury_register_library((void*)&m_math_root3, "root3", "math", M_TYPE_FLOAT);
	mercury_register_library((void*)&m_math_golden, "golden", "math", M_TYPE_FLOAT);

	mercury_register_library((void*)&m_math_intmax, "int_max", "math", M_TYPE_INT);
	mercury_register_library((void*)&m_math_uintmax, "uint_max", "math", M_TYPE_INT);
#endif
#ifdef MERCURY_LIB_ARRAY
	mercury_register_library(mercury_lib_array_flush, "flush", "array");
	mercury_register_library(mercury_lib_array_copy, "copy", "array");
	mercury_register_library(mercury_lib_array_insert, "insert", "array");
	mercury_register_library(mercury_lib_array_remove, "remove", "array");
	mercury_register_library(mercury_lib_array_swap, "swap", "array");
	mercury_register_library(mercury_lib_array_sort, "sort", "array");
	mercury_register_library(mercury_lib_array_concat, "concat", "array");

	mercury_register_library(mercury_sort_greater_to_lesser, "SORTING_GREATER_TO_LESSER", "array");
	mercury_register_library(mercury_sort_lesser_to_greater, "SORTING_LESSER_TO_GREATER", "array");
	mercury_register_library(mercury_sort_greater_to_lesser_absolute, "SORTING_GREATER_TO_LESSER_MAGNITUDE", "array");
	mercury_register_library(mercury_sort_lesser_to_greater_absolute, "SORTING_LESSER_TO_GREATER_MAGNITUDE", "array");
	mercury_register_library(mercury_sort_alphabet_az, "SORTING_ALPHABETICAL_A_TO_Z", "array");
	mercury_register_library(mercury_sort_alphabet_za, "SORTING_ALPHABETICAL_Z_TO_A", "array");
#endif
#ifdef MERCURY_LIB_STRING
	mercury_register_library(mercury_lib_string_sub, "sub", "string");
	mercury_register_library(mercury_lib_string_reverse, "reverse", "string");
	mercury_register_library(mercury_lib_string_find, "find", "string");
	mercury_register_library(mercury_lib_string_replace, "replace", "string");
	mercury_register_library(mercury_lib_string_count, "count", "string");
	mercury_register_library(mercury_lib_string_toarray, "toarray", "string");
	mercury_register_library(mercury_lib_string_fromarray, "fromarray", "string");
	mercury_register_library(mercury_lib_string_separate, "separate", "string");
	mercury_register_library(mercury_lib_string_upper, "upper", "string");
	mercury_register_library(mercury_lib_string_lower, "lower", "string");
	mercury_register_library(mercury_lib_string_format, "format", "string");
	mercury_register_library(mercury_lib_string_p_find, "pfind", "string");
	mercury_register_library(mercury_lib_string_p_extract, "pextract", "string");
	mercury_register_library(mercury_lib_string_p_replace, "preplace", "string");
	mercury_register_library(mercury_lib_string_p_count, "pcount", "string");
	mercury_register_library(mercury_lib_string_escape_mercury, "escape", "string");
	mercury_register_library(mercury_lib_string_escape_url, "escape_url", "string");
	mercury_register_library(mercury_lib_string_escape_c, "escape_c", "string");
	mercury_register_library(mercury_lib_string_escape_html, "escape_html", "string");
	mercury_register_library(mercury_lib_string_copy_string, "copy", "string");

#endif
#ifdef MERCURY_LIB_OS
	mercury_register_library(mercury_lib_os_time, "time", "os");
	mercury_register_library(mercury_lib_os_execute, "execute", "os");
	mercury_register_library(mercury_lib_os_call, "call", "os");
	mercury_register_library(mercury_lib_os_clock, "clock", "os");
	mercury_register_library(mercury_lib_os_getdate, "get_date", "os");
	mercury_register_library(mercury_lib_os_gettime, "get_time", "os");
	mercury_register_library((void*)&m_os_isposix, "IS_POSIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_isposix, "IS_UNIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_is64bit, "IS_64BIT", "os", M_TYPE_BOOL);
	mercury_register_library(mercury_lib_os_exit, "exit", "os");
#endif
#ifdef MERCURY_LIB_IO
	mercury_register_library(mercury_lib_io_open, "open", "io");
	mercury_register_library(mercury_lib_io_read, "read", "io");
	mercury_register_library(mercury_lib_io_close, "close", "io");
	mercury_register_library(mercury_lib_io_write, "write", "io");
	mercury_register_library(mercury_lib_io_getfiles, "getfiles", "io");
	mercury_register_library(mercury_lib_io_getdirs, "getdirs", "io");
	mercury_register_library(mercury_lib_io_lines, "lines", "io");
	mercury_register_library(mercury_lib_io_post, "post", "io");
	mercury_register_library(mercury_lib_io_prompt, "prompt", "io");
	mercury_register_library(mercury_lib_io_input, "input", "io");
	mercury_register_library(mercury_lib_io_remove, "remove", "io");
	mercury_register_library(mercury_lib_io_removedir, "removedir", "io");
	mercury_register_library(mercury_lib_io_createdir, "createdir", "io");
	mercury_register_library(mercury_lib_io_serialize, "serialize", "io");
	mercury_register_library(mercury_lib_io_deserialize, "deserialize", "io");


#endif
#ifdef MERCURY_LIB_THREAD
	mercury_register_library(mercury_lib_thread_new, "new", "thread");
	mercury_register_library(mercury_lib_thread_checkfinish, "isfinished", "thread");
	mercury_register_library(mercury_lib_thread_checkrunning, "isrunning", "thread");
	mercury_register_library(mercury_lib_thread_getvalue, "fetch", "thread");
	mercury_register_library(mercury_lib_thread_abort, "abort", "thread");
	mercury_register_library(mercury_lib_thread_getnumvalues, "getcount", "thread");
	mercury_register_library(mercury_lib_thread_waitfor, "await", "thread");
	mercury_register_library(mercury_lib_thread_break, "break", "thread");
	mercury_register_library(mercury_lib_thread_check_error, "checkerror", "thread");
#endif
#ifdef MERCURY_LIB_TABLE
	mercury_register_library(mercury_lib_table_copy, "copy", "table");
#endif
#ifdef MERCURY_LIB_DEBUG
	mercury_register_library(mercury_lib_debug_stack_dbg, "dumpstack", "debug");
	mercury_register_library(mercury_lib_debug_state_dbg, "dumpstate", "debug");
	mercury_register_library(mercury_lib_debug_enviroment_dbg, "dumpenv", "debug");
	mercury_register_library(mercury_lib_debug_constants_dbg, "dumpconstants", "debug");
	mercury_register_library(mercury_lib_debug_bytecode_dbg, "dumpbytecode", "debug");
	mercury_register_library(mercury_lib_debug_bytecode_rawbinary_dbg, "dumprawbytecode", "debug");
	mercury_register_library(mercury_lib_debug_refcount_dbg, "showrefs", "debug");
	mercury_register_library(mercury_lib_debug_dump_debug_info_dbg, "dumpdebuginfo", "debug");
#endif

#ifdef _WIN32
	}
return TRUE;
}
#else
}
#endif

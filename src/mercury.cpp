#include "mercury.h"

#include "mercury_bytecode.h"
#include "mercury_compiler.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <pthread.h>
#endif

//mercury libs
#include "libs/mercury_lib_std.h"
#include "libs/mercury_lib_math.h"
#include "libs/mercury_lib_array.h"
#include "libs/mercury_lib_string.h"
#include "libs/mercury_lib_os.h"
#include "libs/mercury_lib_io.h"
#include "libs/mercury_lib_thread.h"

uint8_t M_NUMBER_OF_TYPES = 11; //VERY IMPORTANT that this is kept at the proper number.
uint16_t register_max = 0xf;






mercury_stringliteral* mercury_cstring_to_mstring(char* str , long size) {
	mercury_stringliteral* nstr=(mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!nstr) return nullptr;
	char* nad = (char*)malloc(sizeof(char) * size);
	if (!nad) return nullptr;
	nstr->size = size;
	
	memcpy(nad,str,size*sizeof(char));
	nstr->ptr = nad;
	nstr->constant = false;
	return nstr;
}

mercury_stringliteral* mercury_cstring_const_to_mstring(char* str, long size) {
	mercury_stringliteral* nstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (!nstr) return nullptr;
	nstr->size = size;
	nstr->ptr = str;
	nstr->constant = true;
	return nstr;
}

char* mercury_mstring_to_cstring(mercury_stringliteral* str) {
	mercury_int sz = strlen(str->ptr); //use this mecause null terminator
	if(sz > str->size)sz = str->size+1;
	char* out = (char*)malloc(sizeof(char) * sz); 
	if (!out)return nullptr;

	memcpy(out, str->ptr, (sz-1)*sizeof(char));
	out[sz-1] = '\0';
	return out;
}


mercury_stringliteral* mercury_copystring(mercury_stringliteral* str) {
	if (str->constant) {
		mercury_stringliteral* nstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
		if (nstr == nullptr) return nullptr;
		nstr->size = str->size;
		nstr->constant = true;
		nstr->ptr = str->ptr;
		return nstr;
	}
	else {
		mercury_stringliteral* nstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
		if (nstr == nullptr) return nullptr;
		nstr->ptr = (char*)malloc(sizeof(char) * str->size);
		if (nstr->ptr == nullptr) return nullptr;
		nstr->size = str->size;
		nstr->constant = false;
		memcpy(nstr->ptr, str->ptr, str->size * sizeof(char));
		return nstr;
	}
}


bool mercury_mstrings_equal(mercury_stringliteral* str1, mercury_stringliteral* str2) {

	if (str1->size != str2->size) {
		return false;
	}
	if ( str1->ptr == str2->ptr)return true;

	for (mercury_int c = 0; c < str1->size; c++) {
		if (str1->ptr[c] != str2->ptr[c]) {
			return false;
		}
	}

	return true;
}

mercury_stringliteral* mercury_mstrings_concat(mercury_stringliteral* str1, mercury_stringliteral* str2) {
	mercury_stringliteral* nstr=(mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
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

	return nstr;
}

//like concat, but does not return a new string. adds appstr to the end of basestr.
bool mercury_mstrings_append(mercury_stringliteral* basestr, mercury_stringliteral* appstr) { 
	if (basestr->constant) {
		char* nptr = (char*)malloc( sizeof(char) * (basestr->size + appstr->size));
		if (!nptr)return false;
		memcpy(nptr,basestr->ptr, basestr->size);
		basestr->ptr = nptr;
		memcpy(basestr->ptr + basestr->size, appstr->ptr, appstr->size);
		basestr->size += appstr->size;
		basestr->constant = false;
	}
	else {
		char* nptr = (char*)realloc(basestr->ptr, sizeof(char) * (basestr->size + appstr->size));
		if (!nptr)return false;
		basestr->ptr = nptr;
		memcpy(basestr->ptr + basestr->size, appstr->ptr, appstr->size);
		basestr->size += appstr->size;
	}
	return true;
}

bool mercury_mstring_addchars(mercury_stringliteral* str, char* chars, mercury_int len) {
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
		memcpy(str->ptr + str->size, chars, len);
		str->size += len;
	}
	return true;
}

void mercury_mstring_delete(mercury_stringliteral* str) {
	if (str) {
		free(str->ptr);
		free(str);
	}
}

mercury_stringliteral* mercury_mstring_substring(mercury_stringliteral* str, mercury_int start, mercury_int end) {
	mercury_stringliteral* nstr = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
	if (nstr == nullptr) return nullptr;

	if (start > str->size || end < 0) { //no characters? just return an empty string.
		nstr->ptr = nullptr;
		nstr->size = 0;

		return nstr;
	}

	start = start < 0 ? 0 : start;
	end = end > str->size-1 ? str->size-1 : end; //clip to the bounds of the string.

	nstr->size = 1l+end - start;
	nstr->ptr=(char*)malloc(sizeof(char)*nstr->size);
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

void mercury_destroytable(mercury_table* table) { //not ideal, but it works. kind of.
	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* st = table->data[i];
		for (mercury_int i2 = 0; i2 < st->size; i2++) {
			mercury_free_var(st->values[i2]);
		}
		free(st);
	}
	free(table);
}


bool mercury_tablehaskey(mercury_table* table, mercury_variable* key) {
	mercury_subtable* subt = table->data[key->type];
	if (key->type != M_TYPE_STRING) {
		for (mercury_int i = 0; i < subt->size; i++) {
			if (subt->keys[i].i == key->data.i) {
				return true;
			}
		}
	}
	else {
		for (mercury_int i = 0; i < subt->size; i++) {
			if (mercury_mstrings_equal((mercury_stringliteral*)subt->keys[i].p, (mercury_stringliteral*)key->data.p) ) {
				return true;
			}
		}
	}
	return false;
}

mercury_variable* mercury_getkey(mercury_table* table, mercury_variable* key, mercury_state* M) {
	mercury_variable* outvar = M?mercury_assign_var(M):(mercury_variable*)malloc(sizeof(mercury_variable));
	if (outvar == nullptr) return nullptr;
	outvar->type = M_TYPE_NIL;
	outvar->data.i = 0;

	mercury_subtable* subt=table->data[key->type];
	for (mercury_int i = 0; i < subt->size; i++) {

		if (key->type == M_TYPE_STRING) {
			if (mercury_mstrings_equal((mercury_stringliteral*)subt->keys[i].p , (mercury_stringliteral*)key->data.p)) {
				if (subt->values[i]->type == M_TYPE_STRING) {
					outvar->type = M_TYPE_STRING;
					outvar->data.p = mercury_copystring((mercury_stringliteral*)subt->values[i]->data.p);
				}
				else {
					outvar->type = subt->values[i]->type;
					outvar->data = subt->values[i]->data;
				}
				if(M)mercury_unassign_var(M, key);
				return outvar;
			}
		}
		else {


			if (subt->keys[i].i == key->data.i) {
				if (subt->values[i]->type == M_TYPE_STRING) {
					outvar->type = M_TYPE_STRING;
					outvar->data.p = mercury_copystring((mercury_stringliteral*)subt->values[i]->data.p);
				}
				else {
					outvar->type = subt->values[i]->type;
					outvar->data = subt->values[i]->data;
				}
				if (M)mercury_unassign_var(M, key);
				return outvar;
			}
		}
	}
	if (M)mercury_unassign_var(M, key);
	return outvar;
}

mercury_int mercury_setkey(mercury_table* table, mercury_variable* key, mercury_variable* value, mercury_state* M) {

	mercury_subtable* subt = table->data[key->type];
	for (mercury_int i = 0; i < subt->size; i++) {

		if (key->type == M_TYPE_STRING) {
			if ( mercury_mstrings_equal( (mercury_stringliteral*)subt->keys[i].p , (mercury_stringliteral*)key->data.p) ) {
				if (M) {
					mercury_unassign_var(M, subt->values[i]);
					mercury_unassign_var(M, key);
				}
				else {
					mercury_free_var(subt->values[i]);
					mercury_free_var(key);
				}
				subt->values[i] = value;
				return i;
			}
		}
		else {
			if (subt->keys[i].i == key->data.i) {
				if (M) {
					mercury_unassign_var(M,subt->values[i]);
					mercury_unassign_var(M,key);
				}
				else {
					mercury_free_var(subt->values[i]);
					mercury_free_var(key);
				}
				subt->values[i] = value;
				return i;
			}
		}


	}

	void* nptr=realloc(subt->keys,sizeof(mercury_rawdata)*(subt->size+1) );
	if (nptr == nullptr) return -1;
	subt->keys = (mercury_rawdata*)nptr;
	nptr = realloc(subt->values, sizeof(mercury_variable*) * (subt->size + 1));
	if (nptr == nullptr) return -1;
	subt->values = (mercury_variable**)nptr;

	subt->keys[subt->size] = key->data;
	subt->values[subt->size] = value;

	subt->size++;

	return subt->size-1;
}

bool mercury_tables_equal(mercury_table* table1, mercury_table* table2) { //returns true if every single value of the tables match. no, it's not recursive, because fuck that (also because refs).
	for (uint8_t i = 0; i < M_NUMBER_OF_TYPES; i++) {
		mercury_subtable* subt1=table1->data[i];
		mercury_subtable* subt2=table2->data[i];

		if (subt1->size != subt2->size) {
			return false;
		}

		for (mercury_int i1 = 0; i1 < subt1->size; i1++) { //not ideal. O(n^2)... this is what i get for not ordering anything.
			bool found = false;
			for (mercury_int i2 = 0; i2 < subt2->size; i2++) {
				if (subt1->keys[i1].i==subt2->keys[i2].i) {
					mercury_variable* v1 = subt1->values[i1];
					mercury_variable* v2 = subt1->values[i1];
					if (v1->type == v2->type && v1->data.i==v2->data.i) {
						found = true;
						break;
					}
				}
			}
			if (!found) return false;
		}

	}
	return true;
}






mercury_state* mercury_newstate(mercury_state* parent) {
	mercury_state* newstate=(mercury_state*)malloc(sizeof(mercury_state));
	if (newstate == nullptr) return nullptr;

	newstate->enviroment = mercury_newtable();
	if (newstate->enviroment == nullptr) {
		free(newstate);
		return nullptr;
	}
	newstate->enviroment->enviromental = true;
	newstate->enviroment->refrences = 0xFFFF;


	mercury_variable* envvarkey = (mercury_variable*)malloc(sizeof(mercury_variable));
	envvarkey->type = M_TYPE_STRING;
	envvarkey->data.p = mercury_cstring_const_to_mstring((char*)"_ENV",4);
	mercury_variable* envvarval = (mercury_variable*)malloc(sizeof(mercury_variable));
	envvarval->type = M_TYPE_TABLE;
	envvarval->data.p = newstate->enviroment;
	mercury_setkey(newstate->enviroment, envvarkey, envvarval);



	if (!parent) {
		newstate->registers = (mercury_variable**)malloc(sizeof(mercury_variable*) * (register_max + 1u));
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
		newstate->parentstate = parent;
	}



	mercury_variable* globvarkey = (mercury_variable*)malloc(sizeof(mercury_variable));
	globvarkey->type = M_TYPE_STRING;
	globvarkey->data.p = mercury_cstring_const_to_mstring((char*)"_G", 2);
	mercury_variable* globvarval = (mercury_variable*)malloc(sizeof(mercury_variable));
	globvarval->type = M_TYPE_TABLE;
	globvarval->data.p = newstate->masterstate->enviroment;
	mercury_setkey(newstate->enviroment, globvarkey, globvarval);


	newstate->sizeofstack = 0;
	newstate->actualstacksize = 0;
	newstate->stack = nullptr;

	newstate->unassignedstack = nullptr;
	newstate->numunassignedstack = 0;
	newstate->sizeunassignedstack = 0;

	newstate->programcounter = 0;

	newstate->bytecode.enviromental = true;
	newstate->bytecode.instructions = nullptr;
	newstate->bytecode.numberofinstructions = 0;
	newstate->bytecode.refrences = 0xFFFF;
	//newstate->numberofinstructions = 0;
	//newstate->instructions = nullptr;


	return newstate;
}



bool mercury_stepstate(mercury_state* M) {
	if (M->programcounter >= M->bytecode.numberofinstructions) return false;

	uint32_t instr = M->bytecode.instructions[M->programcounter];

	uint16_t opcode= instr&0xFFFF;
	uint16_t iflags= instr>>16;

	//printf("%i - %i %i\n", M->programcounter,iflags,opcode);
	M->programcounter++;
	mercury_bytecode_list[opcode](M, iflags);

	/*printf("post stack: %i\n", M->sizeofstack);
	for (mercury_int i = 0; i < M->sizeofstack; i++) {
		mercury_variable* v = M->stack[i];
		printf("\t%i [%i]: %i %f %p\n",i,v->type,v->data.i, v->data.f, v->data.p);
	}
	*/

	return true;
}

void mercury_destroystate(mercury_state* M) {
	for (mercury_int i = 0; i < M->sizeofstack; i++) {
		mercury_unassign_var(M,M->stack[i]);
	}
	free(M->stack);
	for (mercury_int i = 0; i < M->numunassignedstack; i++) {
		free(M->unassignedstack[i]);
	}
	free(M->unassignedstack);

	if (M->masterstate == M && M->registers) {
		for (mercury_int i = 0; i < register_max; i++) {
			if(M->registers[i])free(M->registers[i]);
		}
		free(M->registers);
	}

	if(M->enviroment)mercury_destroytable(M->enviroment);
	if(M->bytecode.instructions)free(M->bytecode.instructions);
	free(M);
}

bool mercury_unassign_var(mercury_state* M,mercury_variable* var) {
	if (!(M->sizeunassignedstack > M->numunassignedstack)) {
		void* nptr=realloc(M->unassignedstack, sizeof(mercury_variable*) * (M->numunassignedstack + 1) );
		if (nptr == nullptr) return false;
		M->sizeunassignedstack = M->numunassignedstack + 1;
		M->unassignedstack = (mercury_variable**)nptr;
	}
	mercury_free_var(var, true);
	M->unassignedstack[M->numunassignedstack] = var;
	M->numunassignedstack++;

	return true;
}

mercury_variable* mercury_assign_var(mercury_state* M) {
	if (M->numunassignedstack) {
		M->numunassignedstack--;
		mercury_variable* nvar = M->unassignedstack[M->numunassignedstack];
		nvar->data.i = 0;
		nvar->type = M_TYPE_NIL;
		return nvar;
	}
	//printf("allocated\n");
	mercury_variable* nvar=(mercury_variable*)malloc(sizeof(mercury_variable));
	if (nvar == nullptr)return nullptr;
	nvar->data.i = 0;
	nvar->type = M_TYPE_NIL;

	return nvar;
}

void mercury_free_var(mercury_variable* var,bool keep_struct) {
	if (var == nullptr)return;

	switch (var->type)
	{
	case M_TYPE_TABLE:
	{
		mercury_table* ftab = (mercury_table*)var->data.p;
		ftab->refrences--;
		if (!ftab->refrences) {
			for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
				mercury_subtable* st = ftab->data[t];
				for (mercury_int i = 0; i < st->size; i++) {
					mercury_variable* v = st->values[i];
					mercury_free_var(v,true);
					v->type = t;	//because keys are stored as raw data, we have to do a bit of hax. still, this is a neat way to save some processing and memory.
					v->data = st->keys[i];
					mercury_free_var(v);
				}
				free(st);
			}
			free(ftab);
		}
	}
		break;
	case M_TYPE_STRING:
	{
		mercury_stringliteral* str = (mercury_stringliteral*)var->data.p;
		if (!str->constant)free(str->ptr);
		free(str);
	}
		break;
	case M_TYPE_ARRAY:
		{
		mercury_array* farray = (mercury_array*)var->data.p; //get the array
		farray->refrences--;
		if (!farray->refrences) { //if this is the last refrence, destroy all
			
			for (mercury_int i1 = 0; i1 < (farray->size); i1++) {
				mercury_variable** vt = farray->values[i1];
				if (!vt)continue;
				for (mercury_int i2 = 0; i2 < (1 << MERCURY_ARRAY_BLOCKSIZE); i2++) {
					mercury_variable* v = vt[i2];
					if (!v)continue;
					mercury_free_var(v);
				}
			}
	
		}
		}
		break;
	case M_TYPE_FUNCTION:
		{
		mercury_function* ffunction = (mercury_function*)var->data.p;
		ffunction->refrences--;
		if (!ffunction->refrences) {
			//free(ffunction->instructions); //this causes a heap issue. dunno why.
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
			t->state->bytecode.instructions = nullptr;
			mercury_destroystate(t->state);
			free(t);
		}

		
	}
		break;
	default:
		break;
	}
	if(!keep_struct)free(var);
}

mercury_variable* mercury_popstack(mercury_state* M) {

	if (M->sizeofstack==0) {
		mercury_variable* newvar = mercury_assign_var(M);
		if (newvar == nullptr) return nullptr;

		newvar->type = M_TYPE_NIL;
		newvar->data.i = 0;

		return newvar;
	}

	M->sizeofstack--;
	return  M->stack[M->sizeofstack];
}

//takes from the bottom instead of the top of stack
mercury_variable* mercury_pullstack(mercury_state* M) {

	if (M->sizeofstack == 0) {
		mercury_variable* newvar = mercury_assign_var(M);
		if (newvar == nullptr) return nullptr;

		newvar->type = M_TYPE_NIL;
		newvar->data.i = 0;

		return newvar;
	}

	mercury_variable* o = M->stack[0];
	M->sizeofstack--;
	memmove(M->stack, M->stack + 1, sizeof(mercury_variable*) * M->sizeofstack);
	
	return o;
}


bool mercury_pushstack(mercury_state* M, mercury_variable* var) {
	if (!(M->actualstacksize>M->sizeofstack)) {
		void* nstackptr = realloc(M->stack, (M->sizeofstack + 1) * sizeof(mercury_variable*));
		if (nstackptr == nullptr) return false;
		M->stack = (mercury_variable**)nstackptr;

		M->actualstacksize = M->sizeofstack + 1;
	}

	M->stack[M->sizeofstack] = var;
	M->sizeofstack++;

	switch (var->type) {
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

mercury_variable* mercury_clonevariable(mercury_variable* var,mercury_state* M) {
	mercury_variable* out=nullptr;
	if (M) {
		out=mercury_assign_var(M);
	}
	else {
		out = (mercury_variable*)malloc(sizeof(mercury_variable));
	}
	if (!out)return nullptr;

	out->type = var->type;
	if (out->type == M_TYPE_STRING) {
		out->data.p = mercury_copystring((mercury_stringliteral*)var->data.p);
	}
	else {
		out->data = var->data;
	}
	
	return out;
}


mercury_array* mercury_newarray() {
	mercury_array* nar = (mercury_array*)malloc(sizeof(mercury_array));
	if (nar == nullptr)return nullptr;

	nar->size = 0;
	nar->refrences = 1;
	nar->values = nullptr;

	return nar;
}

bool mercury_setarray(mercury_array* array, mercury_variable* var, mercury_int pos, mercury_state* M) {
	if (pos < 0) return false;
	mercury_int subnumber = pos >> MERCURY_ARRAY_BLOCKSIZE;
	mercury_int mantisa = pos & ((1 << MERCURY_ARRAY_BLOCKSIZE) - 1);

	if (subnumber >= array->size) {
		void* nptr=realloc(array->values, sizeof(mercury_variable**) * (subnumber+1) );
		if (!nptr)return false;
		array->values = (mercury_variable***)nptr;
		for (mercury_int i = array->size;i<=subnumber;i++) {
			array->values[i] = nullptr;
		}
		array->size = subnumber + 1;
	}
	mercury_variable** subblock = array->values[subnumber];
	if (!subblock) {
		void* nptr=malloc(sizeof(mercury_variable*) * (1 << MERCURY_ARRAY_BLOCKSIZE));
		if (!nptr)return false;
		subblock = (mercury_variable**)nptr;
		array->values[subnumber] = subblock;
		for (mercury_int i = 0; i < (1 << MERCURY_ARRAY_BLOCKSIZE); i++) {
			subblock[i] = nullptr;
		}
	}
	if (subblock[mantisa])
	{
		if (M){
			mercury_unassign_var(M, subblock[mantisa]);
		}
		else {
			mercury_free_var(subblock[mantisa]);
		}
	}
	subblock[mantisa] = var;

	return true;
}

mercury_variable* mercury_getarray(mercury_array* array, mercury_int pos, mercury_state* M) {
	mercury_variable* out;
	if (pos < 0) {
		out = M?mercury_assign_var(M):(mercury_variable*)malloc(sizeof(mercury_variable));
		if (!out)return nullptr;
		out->data.i = 0;
		out->type = M_TYPE_NIL;
		return out;
	}
	mercury_int subnumber = pos >> MERCURY_ARRAY_BLOCKSIZE;
	mercury_int mantisa = pos & ((1 << MERCURY_ARRAY_BLOCKSIZE) - 1);

	if (subnumber >= array->size) {
		out = M ? mercury_assign_var(M) : (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!out)return nullptr;
		out->data.i = 0;
		out->type = M_TYPE_NIL;
		return out;
	}

	mercury_variable** subblock = array->values[subnumber];
	if (!subblock) {
		out = M ? mercury_assign_var(M) : (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!out)return nullptr;
		out->data.i = 0;
		out->type = M_TYPE_NIL;
		return out;
	}

	if (subblock[mantisa]) {
		out = M ? mercury_assign_var(M) : (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!out)return nullptr;

		mercury_variable* v = subblock[mantisa];
		out->type = v->type;
		if (out->type == M_TYPE_STRING) {
			out->data.p = mercury_copystring((mercury_stringliteral*)v->data.p);
		}
		else {
			out->data.i = v->data.i;
		}
		return out;
	}
	else {
		out = M ? mercury_assign_var(M) : (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!out)return nullptr;
		out->data.i = 0;
		out->type = M_TYPE_NIL;
		return out;
	}

}

mercury_int mercury_array_len(mercury_array* arr) {
	if (!arr->size)return 0;
	mercury_variable** suba=arr->values[arr->size-1];
	mercury_int slen = 0;
	for (mercury_int i = 0; i < (1 << MERCURY_ARRAY_BLOCKSIZE); i++) {
		if (suba[i] && suba[i]->type) {
			slen = i;
		}
	}
	return (slen | ((arr->size-1) << MERCURY_ARRAY_BLOCKSIZE)) +1;
}




mercury_variable* mercury_tostring(mercury_variable* var) {
	mercury_variable* newvar = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (newvar == nullptr) return nullptr;
	newvar->type = M_TYPE_STRING;
	newvar->data.p = nullptr;

	mercury_stringliteral* tstr;

	char tout[256];
	for (int i = 0; i < 256; i++) {
		tout[i] = '\0';
	}
	int tint;

	switch (var->type) {
	case M_TYPE_NIL:
		tstr = mercury_cstring_to_mstring((char*)"nil", 3);
			if(tstr == nullptr) {
				free(newvar);
				return nullptr; 
			}
			newvar->data.p = tstr;
		break;
	case M_TYPE_INT:
		tint=snprintf( tout,sizeof(tout),"%zi",var->data.i);
		if (tint==-1) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout) );
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_FLOAT:
		#ifdef MERCURY_64BIT
		tint = snprintf(tout, sizeof(tout), "%#.30lg", var->data.f);
		#else
		tint = snprintf(tout, sizeof(tout), "%#.15g", var->data.f);
		#endif
		
		if (tint==-1) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_BOOL:
		if (var->data.i) {
			tstr = mercury_cstring_to_mstring((char*)"true", 4);
		}
		else {
			tstr = mercury_cstring_to_mstring((char*)"false", 5);
		}
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_TABLE:
		tint = snprintf(tout, sizeof(tout), "table 0x%p", var->data.p);
		if (!tint) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_STRING:
		tstr = mercury_copystring((mercury_stringliteral*)var->data.p);
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_CFUNC:
		tint = snprintf(tout, sizeof(tout), "c function 0x%p", var->data.p);
		if (!tint) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_FUNCTION:
		tint = snprintf(tout, sizeof(tout), "function 0x%p", var->data.p);
		if (!tint) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_ARRAY:
		tint = snprintf(tout, sizeof(tout), "array 0x%p", var->data.p);
		if (!tint) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	case M_TYPE_FILE:
		tint = snprintf(tout, sizeof(tout), "file 0x%p", var->data.p);
		if (!tint) {
			free(newvar);
			return nullptr;
		}
		tstr = mercury_cstring_to_mstring(tout, strlen(tout));
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
		break;
	default:
		tstr = mercury_cstring_to_mstring((char*)"unknown", 7);
		if (tstr == nullptr) {
			free(newvar);
			return nullptr;
		}
		newvar->data.p = tstr;
	}
	
	return newvar;
}

bool mercury_checkbool(mercury_variable* var) {
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
mercury_int mercury_checkint(mercury_variable* var) {
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
mercury_float mercury_checkfloat(mercury_variable* var) {
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
void* mercury_checkpointer(mercury_variable* var) {
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


bool mercury_vars_equal(mercury_variable* var1, mercury_variable* var2) {
	if (var1->type != var2->type) {
		return false;
	}

	if (var1->type == M_TYPE_STRING) {
		return mercury_mstrings_equal( (mercury_stringliteral*)var1->data.p, (mercury_stringliteral*)var2->data.p);
	}
	else {
		return var1->data.i == var2->data.i;
	}
}


void mercury_debugdumpbytecode(uint32_t* instructions, mercury_int number_instructions) {
	mercury_int offset = 0;


	while (offset< number_instructions)
	{
		uint16_t instr = instructions[offset]& 0xFFFF;
		uint16_t flags = instructions[offset] >> 16;
		printf("%2llu] ", offset);
		switch (instr) {
		case M_OPCODE_NOP:
			printf(" NOP\n"); break;
		case M_OPCODE_ADD:
			printf(" ADD\n"); break;
		case M_OPCODE_SUB:
			printf(" SUB\n"); break;
		case M_OPCODE_MUL:
			printf(" MUL\n"); break;
		case M_OPCODE_DIV:
			printf(" DIV\n"); break;
		case M_OPCODE_IDIV:
			printf("IDIV\n"); break;
		case M_OPCODE_POW:
			printf(" POW\n"); break;
		case M_OPCODE_LOR:
			printf("L OR\n"); break;
		case M_OPCODE_LXOR:
			printf("LXOR\n"); break;
		case M_OPCODE_LNOT:
			printf("LNOT\n"); break;
		case M_OPCODE_LAND:
			printf("LAND\n"); break;
		case M_OPCODE_BOR:
			printf("B OR\n"); break;
		case M_OPCODE_BXOR:
			printf("BXOR\n"); break;
		case M_OPCODE_BNOT:
			printf("BNOT\n"); break;
		case M_OPCODE_BAND:
			printf("BAND\n"); break;
		case M_OPCODE_BSHL:
			printf("BSHL\n"); break;
		case M_OPCODE_BSHR:
			printf("BSHR\n"); break;
		case M_OPCODE_EQL:
			printf(" EQL\n"); break;
		case M_OPCODE_NSTR:
			printf("NSTR ");
			{
				mercury_int sz = 0;
#ifdef MERCURY_64BIT
				sz = *((mercury_int*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_int*)(instructions + offset + 1));
				offset++;
#endif
				putchar('\"');
				char* cp = (char*)(instructions + offset + 1);
				for (int c = 0; c < sz; c++) {
					putchar(cp[c]);
				}
				putchar('\"');
				offset += (sz + 3) / 4;
				//offset++;
			}
			putchar('\n');
			
			break;
		case M_OPCODE_NINT:
			printf("NINT ");
			{
				mercury_int sz = 0;
#ifdef MERCURY_64BIT
				sz = *((mercury_int*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_int*)(instructions + offset + 1));
				offset++;
#endif
				printf("%i\n", sz);
			}
			break;
		case M_OPCODE_NFLO:
			printf("NFLO ");
			{
				mercury_float sz = 0.0;
#ifdef MERCURY_64BIT
				sz = *((mercury_float*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_float*)(instructions + offset + 1));
				offset++;
#endif
				printf("%llf\n", sz);
			}
			break;
		case M_OPCODE_NFUN:
			printf("NFUN ");
			{
				mercury_int sz = 0;
#ifdef MERCURY_64BIT
				sz = *((mercury_int*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_int*)(instructions + offset + 1));
				offset++;
#endif
				printf("instructions:%i\n", sz);
			}
			break;
		case M_OPCODE_NFAL:
			printf("NFAL\n");
			break;
		case M_OPCODE_NTRU:
			printf("NTRU\n");
			break;
		case M_OPCODE_NARR:
			printf("NARR\n");
			break;
		case M_OPCODE_NTAB:
			printf("NTAB\n");
			break;
		case M_OPCODE_SENV:
			printf("SENV\n");
			break;
		case M_OPCODE_GENV:
			printf("GENV\n");
			break;
		case M_OPCODE_GET:
			printf("GET \n");
			break;
		case M_OPCODE_SET:
			printf("SET \n");
			break;
		case M_OPCODE_SETL:
			printf("SETL\n");
			break;
		case M_OPCODE_SETG:
			printf("SETG\n");
			break;
		case M_OPCODE_GETL:
			printf("GETL\n");
			break;
		case M_OPCODE_GETG:
			printf("GETG\n");
			break;
		case M_OPCODE_CPYT:
			printf("CPYT\n");
			break;
		case M_OPCODE_CNCT:
			printf("CNCT\n");
			break;
		case M_OPCODE_JRNI:
			printf("JRNI ");
			{
				mercury_int sz = 0;
#ifdef MERCURY_64BIT
				sz = *((mercury_int*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_int*)(instructions + offset + 1));
				offset++;
#endif
				printf("%lli\n", sz);
			}
			break;
		case M_OPCODE_JMPR:
			printf("JMPR ");
			{
				mercury_int sz = 0;
#ifdef MERCURY_64BIT
				sz = *((mercury_int*)(instructions + offset + 1));
				offset += 2;
#else
				sz = *((mercury_int*)(instructions + offset + 1));
				offset++;
#endif
				printf("%lli\n", sz);
			}
			break;
		case M_OPCODE_CALL:
		{
			mercury_int i = 0;
			mercury_int o = 0;
#ifdef MERCURY_64BIT
			i = *((mercury_int*)(instructions + offset + 1));
			offset += 2;
			o = *((mercury_int*)(instructions + offset + 1));
			offset += 2;
#else
			i = *((mercury_int*)(instructions + offset + 1));
			offset++;
			o = *((mercury_int*)(instructions + offset + 1));
			offset++;
#endif
			printf("CALL in:%lli out:%lli\n", i, o);
		}
			break;
		default:
			printf("???? (%i)\n", instructions[offset]);
		}


		offset++;
	}


}





void mercury_debugdumptable(mercury_table* tab,int level=0) {
	for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
		mercury_subtable* subt = tab->data[t];
		for (mercury_int i = 0; i < subt->size; i++) {
			for (int n = 0; n < level; n++) {
				putchar('\t');
			}

			switch (t) {
				case M_TYPE_NIL:
					printf("nil_%i", subt->keys[i].i);
					break;
				case M_TYPE_INT:
					printf("%lli", subt->keys[i].i);
					break;
				case M_TYPE_FLOAT:
					printf("%f", subt->keys[i].f);
					break;
				case M_TYPE_STRING:
					{
					mercury_stringliteral* str = (mercury_stringliteral*)subt->keys[i].p;
					putchar('\"');
					for (mercury_int i2 = 0; i2 < str->size; i2++) {
						putchar(str->ptr[i2]);
						}
					}
					putchar('\"');
					break;
				default:
					printf("0x%p", subt->keys[i].p);
			}

			printf(" - ");

			mercury_variable* var = subt->values[i];

			switch (var->type) {
				case M_TYPE_INT:
					printf("%lli", var->data.i);
					break;
				case M_TYPE_FLOAT:
					printf("%llf", var->data.f);
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
						mercury_int l=mercury_array_len(arr);
						for (mercury_int q = 0; q < l; q++) {
							putchar('\n');
							mercury_variable* v = mercury_getarray(arr,q);
							for (int n = 0; n < level + 1; n++) {
								putchar('\t');
							}
							switch (v->type)
							{
							case M_TYPE_NIL:
								printf("%i - nil", q);
								break;
							case M_TYPE_INT:
								printf("%i - %i", q, var->data.i);
								break;
							case M_TYPE_FLOAT:
								printf("%i - %f", q, var->data.f);
								break;
							case M_TYPE_STRING:
							{
								mercury_stringliteral* sp = (mercury_stringliteral*)v->data.p;
							
								printf("%i - ", q);
								putchar('\"');
								for (mercury_int sc = 0; sc < sp->size; sc++) {
									putchar(sp->ptr[sc]);
								}
								putchar('\"');
								
							}
								break;
							default:
								printf("%i - [%i] 0x%p",q,v->type ,v->data.p);
								break;
							}
						}
					}
					break;
				case M_TYPE_STRING:
					{
						mercury_stringliteral* str = (mercury_stringliteral*)var->data.p;
						putchar('\"');
						for (mercury_int i2 = 0; i2 < str->size; i2++) {
							putchar(str->ptr[i2]);
						}
						putchar('\"');
					}
					break;
				default:
					printf("[%i] 0x%p",var->type, var->data.p);
			}

			
			

			putchar('\n');
		}
	}


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
		
		mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
		if (!v)continue;
		v->type = lib->type;
		switch (lib->type)
		{
		case M_TYPE_FLOAT:
			v->data.f = *(mercury_float*)lib->dataptr;
			break;
		case M_TYPE_INT:
		case M_TYPE_BOOL:
			v->data.i = *(mercury_int*)lib->dataptr;
			break;
		case M_TYPE_CFUNC:
			v->data.p = (mercury_cfunc)lib->dataptr;
			break;
		case M_TYPE_NIL:
			v->data.p = lib->dataptr;
			break;
		default:
			free(v);
			continue;
		}

		if (lib->table) {
			mercury_variable* k = (mercury_variable*)malloc(sizeof(mercury_variable));
			if (!k) { free(v); continue; }
			k->type = M_TYPE_STRING;
			k->data.p = mercury_cstring_to_mstring((char*)lib->key, strlen(lib->key));

			mercury_stringliteral* sp= mercury_cstring_to_mstring((char*)lib->table, strlen(lib->table));

			mercury_variable tidx;
			tidx.type = M_TYPE_STRING;
			tidx.data.p = sp;
			mercury_variable* t=mercury_getkey(M->enviroment, &tidx);

			if (t->type == M_TYPE_TABLE) {
				mercury_setkey((mercury_table*)t->data.p,k,v);
			}
			else {
				mercury_table* nt=mercury_newtable();

				mercury_setkey(nt, k, v);

				mercury_variable* vv = (mercury_variable*)malloc(sizeof(mercury_variable));
				if (!vv) {

					continue;
				}
				vv->type = M_TYPE_TABLE;
				vv->data.p = nt;

				mercury_setkey(M->enviroment, &tidx, vv);
			}


		}
		else {

			mercury_variable* k= (mercury_variable*)malloc(sizeof(mercury_variable));
			if (!k) { free(v); continue; }
			k->type = M_TYPE_STRING;
			k->data.p = mercury_cstring_to_mstring((char*)lib->key,strlen(lib->key) );

			mercury_setkey(M->enviroment, k, v);

		}


	}


}


//use dll loading stuff to register libraries. simple and easy.

#ifdef _WIN32
BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
#else
__attribute__((constructor)) dynamic_lib_load() {
#endif


#ifdef MERCURY_LIB_STD
	mercury_register_library(mercury_lib_std_print, "print", nullptr);
	mercury_register_library(mercury_lib_std_iterate, "iterate", nullptr);
	mercury_register_library(mercury_lib_std_restricted_call, "rcall", nullptr);
	mercury_register_library(mercury_lib_std_dump, "dump", nullptr);
	mercury_register_library(mercury_lib_std_compile, "compile", nullptr);
	mercury_register_library(mercury_lib_std_type, "type", nullptr);
	mercury_register_library(mercury_lib_std_tostring, "tostring", nullptr);
	mercury_register_library(mercury_lib_std_tonumber, "tonumber", nullptr);
	mercury_register_library(mercury_lib_std_dynamic_library_load, "loadlibrary", nullptr);


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
	mercury_register_library(mercury_lib_string_toarray, "toarray", "string");
	mercury_register_library(mercury_lib_string_fromarray, "fromarray", "string");
	mercury_register_library(mercury_lib_string_separate, "separate", "string");
	mercury_register_library(mercury_lib_string_upper, "upper", "string");
	mercury_register_library(mercury_lib_string_lower, "lower", "string");
	mercury_register_library(mercury_lib_string_format, "format", "string");

#endif
#ifdef MERCURY_LIB_OS
	mercury_register_library(mercury_lib_os_time, "time", "os");
	mercury_register_library(mercury_lib_os_execute, "execute", "os");
	mercury_register_library(mercury_lib_os_call, "call", "os");
	mercury_register_library(mercury_lib_os_clock, "clock", "os");
	mercury_register_library((void*)&m_os_isposix, "IS_POSIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_isposix, "IS_UNIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_is64bit, "IS_64BIT", "os", M_TYPE_BOOL);
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

#endif
#ifdef MERCURY_LIB_THREAD
	mercury_register_library(mercury_lib_thread_new, "new", "thread");
	mercury_register_library(mercury_lib_thread_checkfinish, "isfinished", "thread");
	mercury_register_library(mercury_lib_thread_checkrunning, "isrunning", "thread");
	mercury_register_library(mercury_lib_thread_getvalue, "fetch", "thread");
	mercury_register_library(mercury_lib_thread_abort, "abort", "thread");
	mercury_register_library(mercury_lib_thread_getnumvalues, "getcount", "thread");
	mercury_register_library(mercury_lib_thread_waitfor, "await", "thread");
#endif


#ifdef _WIN32
	}
return TRUE;
}
#else
}
#endif



 /*
int main(int argc, char** argv) {
	


#ifdef MERCURY_LIB_STD
	mercury_register_library(mercury_lib_std_print, "print", nullptr);
	mercury_register_library(mercury_lib_std_iterate, "iterate", nullptr);
	mercury_register_library(mercury_lib_std_restricted_call, "rcall", nullptr);
	mercury_register_library(mercury_lib_std_dump, "dump", nullptr);
	mercury_register_library(mercury_lib_std_compile, "compile", nullptr);
	mercury_register_library(mercury_lib_std_type, "type", nullptr);
	mercury_register_library(mercury_lib_std_tostring, "tostring", nullptr);
	mercury_register_library(mercury_lib_std_tonumber, "tonumber", nullptr);
	mercury_register_library(mercury_lib_std_dynamic_library_load, "loadlibrary", nullptr);


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
	mercury_register_library(mercury_lib_string_toarray, "toarray", "string");
	mercury_register_library(mercury_lib_string_fromarray, "fromarray", "string");
	mercury_register_library(mercury_lib_string_separate, "separate", "string");
	mercury_register_library(mercury_lib_string_upper, "upper", "string");
	mercury_register_library(mercury_lib_string_lower, "lower", "string");

#endif
#ifdef MERCURY_LIB_OS
	mercury_register_library(mercury_lib_os_time, "time", "os");
	mercury_register_library(mercury_lib_os_execute, "execute", "os");
	mercury_register_library(mercury_lib_os_clock, "clock", "os");
	mercury_register_library((void*)&m_os_isposix, "IS_POSIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_isposix, "IS_UNIX", "os", M_TYPE_BOOL);
	mercury_register_library((void*)&m_os_is64bit, "IS_64BIT", "os", M_TYPE_BOOL);
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

#endif
#ifdef MERCURY_LIB_THREAD
	mercury_register_library(mercury_lib_thread_new, "new", "thread");
	mercury_register_library(mercury_lib_thread_checkfinish, "isfinished", "thread");
	mercury_register_library(mercury_lib_thread_checkrunning, "isrunning", "thread");
	mercury_register_library(mercury_lib_thread_getvalue, "fetch", "thread");
	mercury_register_library(mercury_lib_thread_abort, "abort", "thread");
	mercury_register_library(mercury_lib_thread_getnumvalues, "getcount", "thread");
	mercury_register_library(mercury_lib_thread_waitfor, "await", "thread");
#endif


	mercury_array* arg_arr=mercury_newarray();

	//printf("arg count: %i\n", argc);
	for (int i = 0; i < argc; i++) {
		mercury_variable* av = (mercury_variable*)malloc(sizeof(mercury_variable));
		if (av) {
			av->type = M_TYPE_STRING;
			av->data.p = mercury_cstring_to_mstring(argv[i], strlen(argv[i]));
			mercury_setarray(arg_arr, av, i);
		}
		//printf("\t%i %s\n",i, argv[i]);
	}
	

	const char* code = "iterate(io, function(k,v) print(k,v) end ) a=202";



	mercury_stringliteral* tstr = mercury_cstring_const_to_mstring((char*)code,strlen(code));

	mercury_variable* funcy = mercury_compile_mstring(tstr);


	if (funcy->type != M_TYPE_FUNCTION) {
		if (funcy->type == M_TYPE_STRING) {
			mercury_stringliteral* s = (mercury_stringliteral*)funcy->data.p;
			for (mercury_int n = 0; n < s->size; n++) {
				putchar(s->ptr[n]);
			}
			putchar('\n');
		}
		return 1;
	}

	mercury_function* compiled = (mercury_function*)funcy->data.p;

	
	int o = 0;
	while (o < compiled->numberofinstructions)
	{
		uint16_t instr = compiled->instructions[o] & 0xFFFF;
		uint16_t flags = compiled->instructions[o] >> 16;

		printf("0x%X (%i) 0x%X (%i)\n", flags, flags, instr, instr);
		o++;
	}


	//mercury_debugdumpbytecode(compiled->instructions, compiled->numberofinstructions);


	mercury_state* M=mercury_newstate();

	mercury_variable* at_v =mercury_assign_var(M);
	mercury_variable* atk_v = mercury_assign_var(M);

	at_v->type = M_TYPE_ARRAY;
	at_v->data.p = arg_arr;
	atk_v->type = M_TYPE_STRING;
	atk_v->data.p = mercury_cstring_const_to_mstring((char*)"_ARGS",5);
	mercury_setkey(M->enviroment, atk_v, at_v);

	mercury_populate_enviroment_with_libs(M);

	M->bytecode.instructions = compiled->instructions;
	M->bytecode.numberofinstructions = compiled->numberofinstructions;
	M->bytecode.debug_info = compiled->debug_info;

	while(mercury_stepstate(M));


	mercury_debugdumptable(M->enviroment, 0);

	return 0;
}
 //*/
#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32

#ifdef MERCURY_DYNAMIC_LIBRARY_COMPILE
#define MERCURY_DYNAMIC_LIBRARY __declspec(dllexport)
#else
#define MERCURY_DYNAMIC_LIBRARY __declspec(dllimport)
#endif

#else
#define MERCURY_DYNAMIC_LIBRARY __attribute__((visibility("default")))
#endif 

#if __x86_64__ || __ppc64__ || __aarch64__ || _WIN64
typedef int64_t mercury_int; //typedefs to ensure that our variables occupy the same space in memory.
typedef uint64_t mercury_uint;
typedef double mercury_float;
//typedef void* mercury_rawvar; //to represent stored binary data of almost any type. using a void* because there's no implicit typecasted behavior when using it.
#define MERCURY_64BIT
#else
typedef __int32 mercury_int;
typedef unsigned __int32 mercury_uint;
typedef float mercury_float;
//typedef void* mercury_rawvar;
#define MERCURY_32BIT
#endif

union mercury_rawdata { //to represent stored binary data of almost any type.
	mercury_int i;
	mercury_uint u;
	mercury_float f;
	void* p;
};

struct mercury_variable {
	uint8_t type = 0;
	mercury_rawdata data;
};

struct mercury_stringliteral {
	mercury_int size=0;
	char* ptr = nullptr;
	bool constant = false; //if true, ptr points to another char*, and so we should not free it. optimization to reduce malloc calls.
};

struct mercury_stringrefrence {
	mercury_int refrencecount = 0;
	mercury_stringliteral* string = nullptr;
};

struct mercury_subtable {
	mercury_int size = 0;
	mercury_rawdata* keys = nullptr;
	mercury_variable** values = nullptr;
};

struct mercury_table {
	mercury_subtable** data;
	mercury_uint refrences = 0;
	bool enviromental = false;
};


//typedef mercury_subtable** mercury_table;

struct mercury_debug_token {
	char* token_prev_prev=	nullptr;
	char* token_prev=	nullptr;
	char* token	  =	nullptr;	//you get 5 tokens.
	char* token_next=	nullptr;
	char* token_next_next=	nullptr;
	mercury_int col=0;
	mercury_int line=0;
};

struct mercury_array { //gee bill, two storage types?
	mercury_int size = 0;
	mercury_uint refrences = 0;
	mercury_variable*** values = nullptr;
};

struct mercury_function {
	mercury_uint refrences = 0;
	mercury_uint numberofinstructions = 0;
	uint32_t* instructions = nullptr;
	bool enviromental = false;
	mercury_debug_token* debug_info=nullptr;
};

struct mercury_filewrapper {
	mercury_uint refrences = 0;
	FILE* file;
	bool open = false;
};




struct mercury_state {
	mercury_state* parentstate = nullptr; //the parent of this state. nullptr if there is no parent
	mercury_state* masterstate = nullptr; // the parent of the parent of the... this can also be itself.

	mercury_int sizeofstack = 0;  // number of elements on the stack
	mercury_variable** stack = nullptr;
	mercury_int actualstacksize = 0; // the size of the stack array

	mercury_int numunassignedstack = 0; // number of unused elements
	mercury_variable** unassignedstack = nullptr;
	mercury_int sizeunassignedstack = 0; // size of the unused element array.

	mercury_variable** registers = nullptr;

	mercury_int programcounter = 0;
	mercury_function bytecode;


	mercury_table* enviroment;

};


struct mercury_threadholder {
	mercury_uint refrences = 0;
	volatile bool finished = false; //it could change, you know.
	bool customenv = false;
#ifdef _WIN32
	HANDLE threadobject = NULL;
#else
	pthread_t threadobject = NULL;
#endif
	mercury_state* state = nullptr; //that's right, it carries an enviroment with it. very hilarious, obviously.
};


typedef void (*mercury_cfunc) (mercury_state* M, mercury_int args_in, mercury_int args_out);

//type defenitions
extern uint8_t M_NUMBER_OF_TYPES;

enum M_TYPE_ENUMS:uint8_t {
	M_TYPE_NIL = 0,
	M_TYPE_INT = 1,
	M_TYPE_FLOAT = 2,
	M_TYPE_BOOL = 3,
	M_TYPE_TABLE = 4,
	M_TYPE_STRING = 5,
	M_TYPE_CFUNC = 6,
	M_TYPE_ARRAY = 7,
	M_TYPE_FUNCTION = 8,
	M_TYPE_FILE = 9,
	M_TYPE_THREAD=10,
};

extern uint16_t register_max;// = 0xf; //registers rage from 0 to this number

#define MERCURY_ARRAY_BLOCKSIZE 8

struct mercury_libdef {
	uint8_t type = M_TYPE_CFUNC;
	void* dataptr = nullptr;
	const char* key = nullptr;
	const char* table = nullptr;
};

extern mercury_libdef** M_LIBS;
extern mercury_int M_NUM_LIBS;

//functions defined in the .cpp

//string
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_cstring_to_mstring(char* str, long size);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_cstring_const_to_mstring(char* str, long size);
MERCURY_DYNAMIC_LIBRARY char* mercury_mstring_to_cstring(mercury_stringliteral* str);
MERCURY_DYNAMIC_LIBRARY bool mercury_mstrings_equal(mercury_stringliteral* str1, mercury_stringliteral* str2);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_mstrings_concat(mercury_stringliteral* str1, mercury_stringliteral* str2);
MERCURY_DYNAMIC_LIBRARY void mercury_mstring_delete(mercury_stringliteral* str);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_mstring_substring(mercury_stringliteral* str, mercury_int start, mercury_int end);
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_tostring(mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY bool mercury_mstrings_append(mercury_stringliteral* basestr, mercury_stringliteral* appstr);
MERCURY_DYNAMIC_LIBRARY bool mercury_mstring_addchars(mercury_stringliteral* str, char* chars, mercury_int len=1);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_copystring(mercury_stringliteral* str);

//table
MERCURY_DYNAMIC_LIBRARY mercury_table* mercury_newtable();
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_getkey(mercury_table* table, mercury_variable* key, mercury_state* M=nullptr);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_setkey(mercury_table* table, mercury_variable* key, mercury_variable* value, mercury_state* M=nullptr);
MERCURY_DYNAMIC_LIBRARY bool mercury_tables_equal(mercury_table* table1, mercury_table* table2);
MERCURY_DYNAMIC_LIBRARY bool mercury_tablehaskey(mercury_table* table, mercury_variable* key);
MERCURY_DYNAMIC_LIBRARY void mercury_destroytable(mercury_table* table);

//array
MERCURY_DYNAMIC_LIBRARY mercury_array* mercury_newarray();
MERCURY_DYNAMIC_LIBRARY bool mercury_setarray(mercury_array* array, mercury_variable* var, mercury_int pos, mercury_state* M = nullptr);
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_getarray(mercury_array* array, mercury_int pos, mercury_state* M=nullptr);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_array_len(mercury_array* arr);


//state
MERCURY_DYNAMIC_LIBRARY mercury_state* mercury_newstate(mercury_state* parent=nullptr);
MERCURY_DYNAMIC_LIBRARY bool mercury_stepstate(mercury_state* M);
MERCURY_DYNAMIC_LIBRARY void mercury_destroystate(mercury_state* M);

//stack
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_popstack(mercury_state* M);
MERCURY_DYNAMIC_LIBRARY bool mercury_pushstack(mercury_state* M, mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_pullstack(mercury_state* M);

MERCURY_DYNAMIC_LIBRARY bool mercury_unassign_var(mercury_state* M, mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_assign_var(mercury_state* M);
MERCURY_DYNAMIC_LIBRARY void mercury_free_var(mercury_variable* var, bool keep_struct = false);
MERCURY_DYNAMIC_LIBRARY mercury_variable* mercury_clonevariable(mercury_variable* var, mercury_state* M=nullptr);

//misc
MERCURY_DYNAMIC_LIBRARY bool mercury_checkbool(mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_checkint(mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY mercury_float mercury_checkfloat(mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY void* mercury_checkpointer(mercury_variable* var);
MERCURY_DYNAMIC_LIBRARY bool mercury_vars_equal(mercury_variable* var1, mercury_variable* var2);


MERCURY_DYNAMIC_LIBRARY bool mercury_register_library(void* data, char* key, char* table, uint8_t type=M_TYPE_CFUNC);
MERCURY_DYNAMIC_LIBRARY void mercury_populate_enviroment_with_libs(mercury_state* M);

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

//gee i sure wish C++ had this as a standard keyword
#ifdef __GNUC__
#define M_CPP_restrict __restrict__
#else
#ifdef _MSC_VER
#define M_CPP_restrict __restrict
#else
#define M_CPP_restrict
#endif
#endif


#if INT64_MAX==INTPTR_MAX
typedef int64_t mercury_int; //typedefs to ensure that our variables occupy the same space in memory.
typedef uint64_t mercury_uint;
typedef double mercury_float;
#define MERCURY_64BIT
#else
typedef __int32 mercury_int;
typedef unsigned __int32 mercury_uint;
typedef float mercury_float;
#define MERCURY_32BIT
#endif

#define MERCURY_VERSION 0
#define MERCURY_VERSION_PATCH 6

#define MERCURY
#if defined(DEBUG) || defined(_DEBUG)
#define MERCURY_DEBUG
#endif

typedef uint16_t mercury_opcode;
constexpr size_t MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE = sizeof(mercury_int) / sizeof(mercury_opcode); //how many instructions fit into one variable. instructions are 16 bits, so on a 64 bit system this is 4. on a 32 bit system, this is 2. if for some reason you're running a 16 bit system, this is 1.

union mercury_rawdata { //to represent stored binary data of almost any type.
	mercury_int i;
	mercury_uint u;
	mercury_float f;
	void* p;
};

struct mercury_variable {
	uint8_t type = 0;
	uint8_t constant = 0;
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
	mercury_variable* keys = nullptr;
	mercury_variable* values = nullptr;
};

struct mercury_table {
	mercury_subtable** data;
	mercury_uint refrences = 0;
	bool enviromental = false;
};


//typedef mercury_subtable** mercury_table;

struct mercury_debug_token {
	char* chars=nullptr;
	mercury_int num_chars=0;
	mercury_int col=0;
	mercury_int line=0;
};

struct mercury_array { //gee bill, two storage types?
	mercury_uint refrences = 0;
#ifdef MERCURY_64BIT
	mercury_variable****** values = nullptr; //array of arrays of arrays of arrays of arrays of arrays of pointers to structs.  array ->  array ->  array ->  array -> array -> array -> struct*. who needs efficency, anyways? hey look pal, you wanted 64 bit indexes, you're gonna get 64 bit indexes.
#else
	mercury_variable*** values = nullptr; //array of arrays of arrays of pointers to structs. array -> array -> array -> struct*
#endif
};

/*
arrays are split into subarrays. this is to make it so that sparse arrays don't eat up all your dedotated wam.
this is done asymetrically, naturally. This is done so that iterating is still somewhat fast for close indexes.
*/
#ifdef MERCURY_64BIT
/*
on a 64 bit system, bits are layed out like this:
<---------------------------64 bits---------------------------->
<10 bits-><10 bits-><10 bits-><10 bits-><--12 bits-><--12 bits->
<most sig ------------------------------------------> least sig>
ensures that worst case, a single variable will consume 12kb 
*/
inline int get_array_index_from_mint_1(mercury_uint i) {
	return (i >> 54);
}
inline int get_array_index_from_mint_2(mercury_uint i) {
	return (i >> 44) & 0b1111111111;
}
inline int get_array_index_from_mint_3(mercury_uint i) {
	return (i >> 34) & 0b1111111111;
}
inline int get_array_index_from_mint_4(mercury_uint i) {
	return (i >> 24) & 0b1111111111;
}
inline int get_array_index_from_mint_5(mercury_uint i) {
	return (i >> 12) & 0b111111111111;
}
inline int get_array_index_from_mint_6(mercury_uint i) {
	return i & 0b111111111111;
}
constexpr int MERCURY_SIZE_SUBARRAY_1 = 0b1111111111 + 1;
constexpr int MERCURY_SIZE_SUBARRAY_2 = 0b1111111111 + 1;
constexpr int MERCURY_SIZE_SUBARRAY_3 = 0b1111111111 + 1;
constexpr int MERCURY_SIZE_SUBARRAY_4 = 0b1111111111 + 1;
constexpr int MERCURY_SIZE_SUBARRAY_5 = 0b111111111111 + 1;
constexpr int MERCURY_SIZE_SUBARRAY_6 = 0b111111111111 + 1;

constexpr int MERCURY_WIDTH_SUBARRAY_1 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_2 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_3 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_4 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_5 = 12;
constexpr int MERCURY_WIDTH_SUBARRAY_6 = 12;

inline mercury_int mercury_reconstruct_array_index(int i1, int i2, int i3, int i4, int i5, int i6) {
	return (((((((((i1 << MERCURY_WIDTH_SUBARRAY_2) | i2) << MERCURY_WIDTH_SUBARRAY_3) | i3) << MERCURY_WIDTH_SUBARRAY_4) | i4) << MERCURY_WIDTH_SUBARRAY_5) | i5) << MERCURY_WIDTH_SUBARRAY_6) | i6;
}
#else
/*
on a 32 bit system, bits are layed out like this:
<------------32 bits----------->
<10 bits-><10 bits-><--12 bits->
<most sig ----------> least sig>
ensures that worst case, a single variable will consume 6kb
*/
inline int get_array_index_from_mint_1(mercury_uint i) {
	return (i >> 22);
}
inline int get_array_index_from_mint_2(mercury_uint i) {
	return (i >> 12)&0b1111111111;
}
inline int get_array_index_from_mint_3(mercury_uint i) {
	return i&0b111111111111;
}
constexpr int MERCURY_SIZE_SUBARRAY_1 = 0b1111111111;
constexpr int MERCURY_SIZE_SUBARRAY_2 = 0b1111111111;
constexpr int MERCURY_SIZE_SUBARRAY_3 = 0b111111111111;

constexpr int MERCURY_WIDTH_SUBARRAY_1 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_2 = 10;
constexpr int MERCURY_WIDTH_SUBARRAY_3 = 12;

inline mercury_int mercury_reconstruct_array_index(int i1, int i2, int i3) {
	return (((i1 << MERCURY_WIDTH_SUBARRAY_2) | i2) << MERCURY_WIDTH_SUBARRAY_3) | i3;
}
#endif

struct mercury_function {
	mercury_uint refrences = 0;
	mercury_uint numberofinstructions = 0;
	mercury_opcode* instructions = nullptr;
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
	mercury_state* childstate = nullptr; // the designated substate used for function calls. ensures we don't have to allocate a bunch of states we don't need. only allocated if needed.

	mercury_uint sizeofstack = 0;  // number of elements on the stack
	mercury_variable* stack = nullptr;
	mercury_uint allocatedstacksize = 0; // the size of the stack array

	mercury_variable* registers = nullptr;
	mercury_variable* constants = nullptr;
	mercury_uint num_constants = 0;

	mercury_uint programcounter = 0;
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


typedef void (*mercury_cfunc) (mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out);

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
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_cstring_to_mstring(const char* const M_CPP_restrict str, const mercury_int size);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_cstring_const_to_mstring(const char* const M_CPP_restrict str, const mercury_int size);
MERCURY_DYNAMIC_LIBRARY char* mercury_mstring_to_cstring(const mercury_stringliteral* const M_CPP_restrict str);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_mstrings_concat(const mercury_stringliteral* const str1, const mercury_stringliteral* const str2);
MERCURY_DYNAMIC_LIBRARY void mercury_mstring_delete(mercury_stringliteral* const M_CPP_restrict str);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_mstring_substring(mercury_stringliteral* str, mercury_int start, mercury_int end);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_tostring(const mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY bool mercury_mstrings_append(mercury_stringliteral* const basestr, const mercury_stringliteral* const appstr);
MERCURY_DYNAMIC_LIBRARY bool mercury_mstring_addchars(mercury_stringliteral* const M_CPP_restrict str, const char* const chars, const mercury_int len=1);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_copystring(const mercury_stringliteral* const M_CPP_restrict str);

//table
MERCURY_DYNAMIC_LIBRARY mercury_table* mercury_newtable();
MERCURY_DYNAMIC_LIBRARY void mercury_destroyarray(mercury_array* const M_CPP_restrict arr);
MERCURY_DYNAMIC_LIBRARY bool mercury_getkey(const mercury_table* const table, mercury_variable* const key, mercury_variable* out);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_setkey(mercury_table* const table, mercury_variable* const key, const mercury_variable* const value);
MERCURY_DYNAMIC_LIBRARY bool mercury_tables_equal(const mercury_table* const table1, const mercury_table* const table2);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_tablehaskey(const mercury_table* const table, const mercury_variable* const key);
MERCURY_DYNAMIC_LIBRARY void mercury_destroytable(mercury_table* const table);
MERCURY_DYNAMIC_LIBRARY void mercury_cleartable(const mercury_table* const table);
MERCURY_DYNAMIC_LIBRARY bool mercury_table_get_cstring_keyvalue(const mercury_table* const table, const char* const key, mercury_variable* out);
MERCURY_DYNAMIC_LIBRARY bool mercury_table_set_cstring_keyvalue(mercury_table* const table, const char* const key, const mercury_variable* const value);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_table_has_cstring_key(const mercury_table* const table, const char* const key);
MERCURY_DYNAMIC_LIBRARY void mercury_prepare_table_for_state(mercury_table* table, mercury_state* M);

//array
MERCURY_DYNAMIC_LIBRARY mercury_array* mercury_newarray();
MERCURY_DYNAMIC_LIBRARY bool mercury_setarray(mercury_array* const array, const mercury_variable* const var, const mercury_int pos);
MERCURY_DYNAMIC_LIBRARY void mercury_getarray(mercury_array* const array, const mercury_int pos, mercury_variable* out);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_array_len(const mercury_array* const arr);


//state
MERCURY_DYNAMIC_LIBRARY mercury_state* mercury_newstate(const mercury_state* const parent=nullptr);
MERCURY_DYNAMIC_LIBRARY bool mercury_stepstate(mercury_state* const M_CPP_restrict M);
MERCURY_DYNAMIC_LIBRARY void mercury_clearstate(mercury_state* const M_CPP_restrict M, bool for_deletion=false);
MERCURY_DYNAMIC_LIBRARY void mercury_destroystate(mercury_state* const M_CPP_restrict M);

//stack
MERCURY_DYNAMIC_LIBRARY void mercury_popstack(mercury_state* const M_CPP_restrict M, mercury_variable* out);
MERCURY_DYNAMIC_LIBRARY bool mercury_pushstack(mercury_state* const M_CPP_restrict M, mercury_variable* const var);
MERCURY_DYNAMIC_LIBRARY void mercury_pullstack(mercury_state* const M_CPP_restrict M, mercury_variable* out);
MERCURY_DYNAMIC_LIBRARY bool mercury_pushstack_unrefed(mercury_state* const M_CPP_restrict M, mercury_variable* const var);

MERCURY_DYNAMIC_LIBRARY void mercury_free_var(mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY void mercury_clonevariable(const mercury_variable* const var, mercury_variable* out);

//misc
MERCURY_DYNAMIC_LIBRARY bool mercury_checkbool(const mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY mercury_int mercury_checkint(const mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY mercury_float mercury_checkfloat(const mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY void* mercury_checkpointer(const mercury_variable* const M_CPP_restrict var);
MERCURY_DYNAMIC_LIBRARY bool mercury_vars_equal(const mercury_variable* const var1, const mercury_variable* const var2);


MERCURY_DYNAMIC_LIBRARY bool mercury_register_library(void* data, char* key, char* table, uint8_t type=M_TYPE_CFUNC);
MERCURY_DYNAMIC_LIBRARY void mercury_populate_enviroment_with_libs(mercury_state* M);

MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_get_bytecode_debug(mercury_function* F);
MERCURY_DYNAMIC_LIBRARY mercury_stringliteral* mercury_get_bytecode_rawbinary_debug(mercury_function* F);
MERCURY_DYNAMIC_LIBRARY void mercury_debugdumptable(mercury_table* tab, int level = 0);

//inlines, for SPEED
inline void mercury_clear_variable(mercury_variable* var) {
	var->constant = false;
	var->data.i = 0;
	var->type = M_TYPE_NIL;
}

//frees the top stack and decrements the stack size. analagous to popstack freevar, albeit a bit faster.
inline void mercury_discard_top_of_stack(mercury_state* const M_CPP_restrict M) {
	if (M->sizeofstack) {
		M->sizeofstack--;
		mercury_free_var(M->stack + M->sizeofstack);
	}
}

inline void mercury_increment_variable_refrence_count(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_TABLE:
		((mercury_table*)var->data.p)->refrences++;
		return;
	case M_TYPE_ARRAY:
		((mercury_array*)var->data.p)->refrences++;
		return;
	case M_TYPE_FUNCTION:
		((mercury_function*)var->data.p)->refrences++;
		return;
	case M_TYPE_FILE:
		((mercury_filewrapper*)var->data.p)->refrences++;
		return;
	case M_TYPE_THREAD:
		((mercury_threadholder*)var->data.p)->refrences++;
		return;
	}
}
inline void mercury_decrement_variable_refrence_count(const mercury_variable* const M_CPP_restrict var) {
	switch (var->type) {
	case M_TYPE_TABLE:
		((mercury_table*)var->data.p)->refrences--;
		return;
	case M_TYPE_ARRAY:
		((mercury_array*)var->data.p)->refrences--;
		return;
	case M_TYPE_FUNCTION:
		((mercury_function*)var->data.p)->refrences--;
		return;
	case M_TYPE_FILE:
		((mercury_filewrapper*)var->data.p)->refrences--;
		return;
	case M_TYPE_THREAD:
		((mercury_threadholder*)var->data.p)->refrences--;
		return;
	}
}

//returns the top stack without removing it. do not free the returned pointer. will return nullptr if stack is 0.
inline void mercury_peek_stack(const mercury_state* M_CPP_restrict M, mercury_variable* out) {
	if (!M->sizeofstack) {
		mercury_clear_variable(out);
	};
	*out= M->stack[M->sizeofstack - 1];
}

inline mercury_state* mercury_get_child_state(mercury_state* const M_CPP_restrict M) {
	if (M->childstate)return M->childstate;
	M->childstate=mercury_newstate(M);
	return M->childstate;
}


inline bool mercury_mstrings_equal(const mercury_stringliteral* const str1, const mercury_stringliteral* const str2) {

	if (str1->size != str2->size) {
		return false;
	}
	if (str1->ptr == str2->ptr)return true;

	for (mercury_int c = 0; c < str1->size; c++) {
		if (str1->ptr[c] != str2->ptr[c]) {
			return false;
		}
	}

	return true;
}

inline bool mercury_mstring_equal_cstring(const mercury_stringliteral* const mstr, const char* const cstr) {
	if (mstr->ptr == cstr)return true;
	if (strlen(cstr) != mstr->size)return false;
	for (mercury_int i = 0; i < mstr->size; i++) {
		if (mstr->ptr[i] != cstr[i])return false;
	}
	return true;
}
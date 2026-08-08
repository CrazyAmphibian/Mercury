#include "../mercury.hpp"
#include "../mercury_error.hpp"
#include "mercury_lib_io.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <climits>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <direct.h>
#include <conio.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#endif




void mercury_lib_io_open(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //opens a file. nuff said.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2)) {
		return;
	}
	if (!args_out) {
		mercury_variable v;
		mercury_popstack(M, &v);
		mercury_free_var(&v);
		mercury_popstack(M, &v);
		mercury_free_var(&v);
		return;
	};

	mercury_variable mode_var;
	mercury_popstack(M,&mode_var);
	if (mode_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, mode_var.type, M_TYPE_STRING, 2);
		return;
	}


	mercury_variable file_var;
	mercury_popstack(M,&file_var);
	if (file_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, file_var.type, M_TYPE_STRING, 1);
		return;
	}

	mercury_variable out;

	char* file = mercury_mstring_to_cstring((mercury_string*)file_var.data.p);
	const char* mode = mercury_mstring_to_cstring((mercury_string*)mode_var.data.p);
	int mode_l = (int)strlen(mode);

	if (mode_l == 1) {
		char c = mode[0];
		free((char*)mode);
		mode = nullptr;
		if (c == 'r')mode = "rb";
		else if(c == 'w')mode = "wb";
		else if(c == 'a')mode = "ab";
	}
	else if (mode_l == 2) {
		char c = mode[0];
		char c2 = mode[1];
		free((char*)mode);
		mode = nullptr;
		if (c == 'r') {
			if (c2 == 'b')mode = "rb";
			else if (c2 == '+')mode = "rb+";
		}
		else if (c == 'w') {
			if (c2 == 'b')mode = "wb";
			else if (c2 == '+')mode = "wb+";
		}
		else if (c == 'a') {
			if (c2 == 'b')mode = "ab";
			else if (c2 == '+')mode = "ab+";
		}
	}
	else if (mode_l == 3) {
		char c = mode[0];
		char c2 = mode[1];
		char c3 = mode[2];
		free((char*)mode);
		mode = nullptr;
		if (c2 == 'b' && c3=='+') {
			if (c == 'r')mode="rb+";
			else if (c == 'w')mode="wb+";
			else if (c == 'a')mode="ab+";
		}
	}

	if (!mode) {
		mercury_free_var(&file_var);
		mercury_free_var(&mode_var);
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
		return;
	}

	FILE* F=fopen(file,mode);
	if (F) {
		out.type = M_TYPE_FILE;
		

		mercury_filewrapper* fw = (mercury_filewrapper*)malloc(sizeof(mercury_filewrapper));
		if (!fw) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			fclose(F);
			return;
		}
		fw->refrences = 1;
		fw->open = true;
		fw->file = F;

		out.data.p = fw;
	}
	else {
		out.type = M_TYPE_NIL;
		out.data.i = 0;
	}
	free(file);

	mercury_free_var(&file_var);
	mercury_free_var(&mode_var);

	mercury_pushstack_unrefed(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}



void mercury_lib_io_read(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable file_var;
	mercury_popstack(M, &file_var);
	if (file_var.type != M_TYPE_FILE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, file_var.type, M_TYPE_FILE, 1);
		return;
	}

	mercury_variable out;

	mercury_filewrapper* fw= (mercury_filewrapper*)file_var.data.p;
	FILE* F = fw->file;

	if (F && fw->open) {

		if (fseek(F, 0, SEEK_END)) {
			out.type = M_TYPE_NIL;
			out.data.i = 0;
		}
		else {
			mercury_int len = ftell(F);
			if (len == -1) {
				out.type = M_TYPE_NIL;
				out.data.i = 0;
			}
			else {
				char* s = (char*)malloc(sizeof(char) * len);
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				rewind(F);
				fread(s, 1, len, F);
				mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
				if (!str) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				str->ptr = s;
				str->size = len;
				str->refrences = 1;
				str->constant = false;
				out.type = M_TYPE_STRING;
				out.data.p = str;
			}
		}
	}
	else {
		out.type = M_TYPE_NIL;
		out.data.i = 0;
	}

	mercury_free_var(&file_var);

	mercury_pushstack_unrefed(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

void mercury_lib_io_close(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}

	mercury_variable file_var;
	mercury_popstack(M, &file_var);
	if (file_var.type != M_TYPE_FILE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, file_var.type, M_TYPE_FILE, 1);
		return;
	}


	mercury_filewrapper* fw = (mercury_filewrapper*)file_var.data.p;
	if (fw->open) {
		fw->open = false;
		if(fw->file)fclose(fw->file);
	}

	mercury_free_var(&file_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}

void mercury_lib_io_write(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 2)) {
		return;
	}

	mercury_variable data_var;
	mercury_popstack(M, &data_var);
	if (data_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, data_var.type, M_TYPE_STRING, 2);
		return;
	}

	mercury_variable file_var;
	mercury_popstack(M, &file_var);
	if (file_var.type != M_TYPE_FILE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, file_var.type, M_TYPE_FILE, 1);
		return;
	}
	mercury_string* str = (mercury_string*)data_var.data.p;
	mercury_filewrapper* fw = (mercury_filewrapper*)file_var.data.p;

	if (fw->open) {
		fwrite(str->ptr, 1, str->size, fw->file);
	}
	mercury_free_var(&data_var);
	mercury_free_var(&file_var);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}



void mercury_lib_io_getfiles(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //an array of strings
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable dir_var;
	mercury_popstack(M,&dir_var);
	if (dir_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, dir_var.type, M_TYPE_STRING, 1);
		return;
	}

	mercury_string* mstr = (mercury_string*)dir_var.data.p;
#ifdef _WIN32
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)"*", 1);
	}
	else {
		mercury_mstring_addchars(mstr, (char*)"/*", 2);
	}
#else
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)".", 1);
	}
#endif
	char* dir = mercury_mstring_to_cstring(mstr);
	
	mercury_array* arr=mercury_newarray();

	mercury_int num_fs = 0;


#ifdef _WIN32 //windows
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind=FindFirstFileA(dir, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		while (true) {
			if (!(FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DEVICE) )) {
				char* fn = FindFileData.cFileName;
				mercury_string* s = mercury_cstring_to_mstring(fn, strlen(fn));
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				mercury_variable v;
				
				v.type = M_TYPE_STRING;
				v.data.p = s;
				mercury_setarray(arr, &v, num_fs);
				num_fs++;
			}
			if (!FindNextFileA(hFind, &FindFileData))break;
		}
	}
	FindClose(hFind);

#else //linux and whatnot
	DIR* d = opendir(dir);
	if (d) {
		dirent* ent;
		while (true) {
			ent = readdir(d);
			if (!ent)break;
			if (ent->d_type == DT_REG) {
				mercury_string* s = mercury_cstring_to_mstring(ent->d_name, strlen(ent->d_name));
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				mercury_variable v;
				
				v.type = M_TYPE_STRING;
				v.data.p = s;
				mercury_setarray(arr, &v, num_fs);
				num_fs++;
			}
		}
		closedir(d);
	}
#endif
	mercury_free_var(&dir_var);
	dir_var.data.p = arr;
	dir_var.type = M_TYPE_ARRAY;
	mercury_pushstack(M, &dir_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}


void mercury_lib_io_getdirs(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //an array of strings
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable dir_var;
	mercury_popstack(M, &dir_var);
	if (dir_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, dir_var.type, M_TYPE_STRING, 1);
		return;
	}

	mercury_string* mstr = (mercury_string*)dir_var.data.p;
#ifdef _WIN32
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)"*", 1);
	}
	else {
		mercury_mstring_addchars(mstr, (char*)"/*", 2);
	}
#else
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)".", 1);
	}
#endif
	char* dir = mercury_mstring_to_cstring(mstr);

	
	mercury_array* arr = mercury_newarray();

	mercury_int num_fs = 0;


#ifdef _WIN32 //windows
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(dir, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		while (true) {
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(FindFileData.cFileName,".") && strcmp(FindFileData.cFileName, "..")) {
				char* fn = FindFileData.cFileName;
				mercury_string* s = mercury_cstring_to_mstring(fn, strlen(fn));
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				mercury_variable v;
				
				v.type = M_TYPE_STRING;
				v.data.p = s;
				mercury_setarray(arr, &v, num_fs);
				num_fs++;
			}
			if (!FindNextFileA(hFind, &FindFileData))break;
		}
	}
	FindClose(hFind);

#else //linux and whatnot
	DIR* d = opendir(dir);
	if (d) {
		dirent* ent;
		while (true) {
			ent = readdir(d);
			if (!ent)break;
			if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
				mercury_string* s = mercury_cstring_to_mstring(ent->d_name, strlen(ent->d_name));
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				mercury_variable v;
				
				v.type = M_TYPE_STRING;
				v.data.p = s;
				mercury_setarray(arr, &v, num_fs);
				num_fs++;
			}
		}
		closedir(d);
	}
#endif


	mercury_free_var(&dir_var);
	dir_var.data.p = arr;
	dir_var.type = M_TYPE_ARRAY;
	mercury_pushstack(M, &dir_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}




void mercury_lib_io_lines(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //returns an array of the lines, sans newline characters.
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable fil_var;
	mercury_popstack(M, &fil_var);
	if (fil_var.type != M_TYPE_FILE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, fil_var.type, M_TYPE_FILE, 1);
		return;
	}

	
	mercury_array* arr = mercury_newarray();
	mercury_int count = 0;

	mercury_filewrapper* fw = (mercury_filewrapper*)fil_var.data.p;
	if (fw->open) {
		FILE* f = fw->file;
		rewind(f);
		fseek(f, 0, SEEK_END);
		mercury_int total_len = ftell(f);
		rewind(f);
		char* buffer=(char*)malloc(total_len);
		if (!buffer && total_len) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		if (total_len) { //support for empty file reading
			fread(buffer, 1, total_len, f);
			rewind(f);
		}
		mercury_int run_end = 0;
		mercury_int run_start = 0;
		while (true) {
			if (run_end == total_len) { //end of file
				if (run_end - run_start) {
					mercury_string* s = mercury_cstring_to_mstring(buffer+ run_start, run_end -run_start);
					if (!s) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						free(buffer);
						mercury_destroyarray(arr);
						return;
					}
					mercury_variable v;
					v.type = M_TYPE_STRING;
					v.data.p = s;
					mercury_setarray(arr, &v, count);
					count++;
				}
				break;
			}
			char cur_char=buffer[run_end];
			if (cur_char == '\n' || cur_char == '\r') {
				if (run_end - run_start) {
					mercury_string* s = mercury_cstring_to_mstring(buffer + run_start, run_end - run_start);
					if (!s) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						free(buffer);
						mercury_destroyarray(arr);
						return;
					}
					mercury_variable v;
					v.type = M_TYPE_STRING;
					v.data.p = s;
					mercury_setarray(arr, &v, count);
					count++;
				}
				run_end++;
				run_start = run_end;
			}
			else {
				run_end++;
			}
		}
		free(buffer);

		
	}
	

	mercury_free_var(&fil_var);
	fil_var.type = M_TYPE_ARRAY;
	fil_var.data.p = arr;
	mercury_pushstack_unrefed(M, &fil_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


void mercury_lib_io_post(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //send characters to stdout directly
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}

	mercury_variable str_var;
	mercury_popstack(M,&str_var);
	if (str_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, str_var.type, M_TYPE_STRING, 1);
		return;
	}

	mercury_string* s = (mercury_string*)str_var.data.p;
	for (mercury_int c = 0; c < s->size; c++) {
		putchar(s->ptr[c]);
	}
	mercury_free_var(&str_var);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
}

void mercury_lib_io_prompt(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //read a line from stdin
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0)) {
		return;
	}

	mercury_int sizec = 200;
	mercury_int len = 0;
	char* c = (char*)malloc(sizec);
	if (!c) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}


	int ch = 0;
	while ((ch = fgetc(stdin)) != EOF && ch!='\n' && ch != '\r') {
		c[len] = ch;
		len++;
		if (len >= sizec) {
			sizec += 200;
			void* n=realloc(c, sizec);
			if (!n) {
				mercury_raise_error(M, M_ERROR_ALLOCATION);
				return;
			}
			c = (char*)n;
		}
	}

	if (args_out) {
		mercury_string* s=mercury_cstring_to_mstring(c, len);
		if (!s) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		mercury_variable v;
		
		v.type = M_TYPE_STRING;
		v.data.p = s;
		mercury_pushstack(M, &v);
	}
	free(c);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}



void mercury_lib_io_remove(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}

	mercury_variable dir_var;
	mercury_popstack(M, &dir_var);
	if (dir_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, dir_var.type, M_TYPE_STRING, 1);
		return;
	}
	mercury_string* fst = (mercury_string*)dir_var.data.p;

	char* cfilestr=mercury_mstring_to_cstring(fst);


	int r=remove(cfilestr);

	if (args_out) {
		mercury_variable out;
		out.type = M_TYPE_BOOL;
		out.data.i = r != 0 ? 1 : 0;
		mercury_pushstack(M, &out);
	}

	mercury_free_var(&dir_var);
	free(cfilestr);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out,1);
}


void mercury_lib_io_removedir(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}

	mercury_variable dir_var;
	mercury_popstack(M, &dir_var);
	if (dir_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, dir_var.type, M_TYPE_STRING, 1);
		return;
	}
	mercury_string* fst = (mercury_string*)dir_var.data.p;

	char* cfilestr = mercury_mstring_to_cstring(fst);

#if defined(_WIN32) || defined(_WIN64)
	int r = _rmdir(cfilestr);
#else
	int r = rmdir(cfilestr);
#endif

	if (args_out) {
		mercury_variable out;
		out.type = M_TYPE_BOOL;
		out.data.i = r != 0 ? 1 : 0;
		mercury_pushstack(M, &out);
	}

	mercury_free_var(&dir_var);
	free(cfilestr);
	

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}




void mercury_lib_io_createdir(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		return;
	}

	mercury_variable dir_var;
	mercury_popstack(M, &dir_var);
	if (dir_var.type != M_TYPE_STRING) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, dir_var.type, M_TYPE_STRING, 1);
		return;
	}
	mercury_string* fst = (mercury_string*)dir_var.data.p;

	char* cfilestr = mercury_mstring_to_cstring(fst);
#if defined(_WIN32) || defined(_WIN64)
	int r = _mkdir(cfilestr);
#else
	int r = mkdir(cfilestr,0755); //rwxr-xr-x
#endif
	

	if (args_out) {
		mercury_variable out;
		out.type = M_TYPE_BOOL;
		out.data.i = r != 0 ? 1 : 0;
		mercury_pushstack(M, &out);
	}

	mercury_free_var(&dir_var);
	free(cfilestr);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



void mercury_lib_io_input(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) { //read a single char stdin. no newline required! (platform dependant, though. :/)
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 0)) {
		return;
	}

	//got rid of it. should just be raw input. on the coder to not read it in a while loop (you can also manually check for ctrl+c)
	//mercury_variable* ck = mercury_popstack(M);

	char c;

#ifdef _WIN32
	c = _getch();
#else
	termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	c = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

#endif

	//ctrl+c interuption.
	//if (c == '\3' ){//&& mercury_checkbool(ck) ) {
	//	M->programcounter = M->bytecode.numberofinstructions;
	//}

	//mercury_free_var(ck);

	if (args_out) {
		mercury_string* s = mercury_cstring_to_mstring(&c, 1);
		if (!s) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
		mercury_variable v;
		
		v.type = M_TYPE_STRING;
		v.data.p = s;
		mercury_pushstack(M, &v);
	}

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}

const size_t char_array_blocksize = 16;
enum m_serialize_types:unsigned char{
	SERIALIZED_NIL = '\0',
	SERIALIZED_INT64,
	SERIALIZED_INT32,
	SERIALIZED_INT16,
	SERIALIZED_INT8,
	SERIALIZED_INT8UNSIGNED,
	SERIALIZED_FLOAT32,
	SERIALIZED_FLOAT64,
	SERIALIZED_BOOLTRUE,
	SERIALIZED_BOOLFALSE,
	SERIALIZED_STRING_SIZEEMPTY, //empty string
	SERIALIZED_STRING_SIZESINGLE, //1 character string
	SERIALIZED_STRING_SIZE8,
	SERIALIZED_STRING_SIZE16,
	SERIALIZED_STRING_SIZE32,
	SERIALIZED_STRING_SIZE64,
	SERIALIZED_ARRAY_SIZE8, //unused
	SERIALIZED_ARRAY_SIZE16, //unused
	SERIALIZED_ARRAY_SIZE32, //unused
	SERIALIZED_ARRAY_SIZE64, //unused
	SERIALIZED_ARRAY_SIZEBITWIDTH,
	SERIALIZED_TABLE_SIZE8, //unused
	SERIALIZED_TABLE_SIZE16, //unused
	SERIALIZED_TABLE_SIZE132, //unused
	SERIALIZED_TABLE_SIZE164, //unused
	SERIALIZED_TABLE_SIZEBITWIDTH,
};


inline bool check_chars_preallocation(unsigned char** chars, mercury_int* num_chars, mercury_int* chars_allocated, mercury_int chars_requested) {
#ifdef MERCURY_64BIT
	if (*num_chars > *chars_allocated) {
		printf("serialization called a preallocation with %zi chars set, but only %zi are allocated\n",*num_chars,*chars_allocated);
	}
#endif
	mercury_int defecit = *num_chars + chars_requested - *chars_allocated;
	if (defecit>0) {
		mercury_int blocks_needed = (defecit + char_array_blocksize - 1) / char_array_blocksize;
		void* nptr=realloc(*chars, (size_t)(*chars_allocated + (char_array_blocksize * blocks_needed) ) );
		if (!nptr)return false;
		*chars = (unsigned char*)nptr;
		(*chars_allocated) += char_array_blocksize * blocks_needed;
	}
	return true;
}

inline bool check_pointer_serial(void* ptr, const mercury_uint* num_pointers_covered, void*** pointers_covered) {
	for (mercury_uint i = 0; i < (*num_pointers_covered); i++) {
		if ((*pointers_covered)[i] == ptr) {
			return true;
		}
	}
	return false;
}

inline bool add_serial_pointer(void* ptr, mercury_uint* num_pointers_covered, void*** pointers_covered) {
	if ((*num_pointers_covered) % char_array_blocksize == 0) { //only reallocate every 16 to save speed, probly.
		void* nptr = realloc(*pointers_covered, sizeof(void*) * ((*num_pointers_covered) + char_array_blocksize));
		if (!nptr)return false;
		*pointers_covered = (void**)nptr;
	}

	(*pointers_covered)[*num_pointers_covered] = ptr;

	(*num_pointers_covered)++;
	return true;
}

inline bool can_serialize_var(const mercury_variable* var) {
	switch (var->type) {
	case M_TYPE_INT:
	case M_TYPE_FLOAT:
	case M_TYPE_BOOL:
	case M_TYPE_STRING:
	case M_TYPE_ARRAY:
	case M_TYPE_TABLE:
		return true;
	default:
		return false;
	}
}

bool m_serialize_variable(const mercury_variable* var, unsigned char** chars,mercury_int* num_chars,mercury_int* chars_allocated,void*** pointerscovered,mercury_uint* num_pointers_covered) {
	
	switch (var->type) {
		case M_TYPE_INT:
			if (var->data.i<= SCHAR_MAX && var->data.i >= SCHAR_MIN) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 2))return false;
				(*chars)[*num_chars] = SERIALIZED_INT8;
				(*num_chars)++;
				(*chars)[*num_chars] = (signed char)var->data.i;
				(*num_chars)++;
				return true;
			}
			else if (var->data.i<=UCHAR_MAX && var->data.i>0) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 2))return false;
				(*chars)[*num_chars] = SERIALIZED_INT8UNSIGNED;
				(*num_chars)++;
				(*chars)[*num_chars] = (unsigned char)var->data.i;
				(*num_chars)++;
				return true;
			}
			else if (var->data.i<= SHRT_MAX && var->data.i >= SHRT_MIN) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 3))return false;
				(*chars)[*num_chars] = SERIALIZED_INT16;
				(*num_chars)++;
				*(short*)(*chars+*num_chars) = (short)var->data.i;
				(*num_chars)+=2;
				return true;
			}
			else if (var->data.i<= INT_MAX && var->data.i >= INT_MIN) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 5))return false;
				(*chars)[*num_chars] = SERIALIZED_INT32;
				(*num_chars)++;
				*(int*)(*chars + *num_chars) = (int)var->data.i;
				(*num_chars) += 4;
				return true;
			}
			else {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 9))return false;
				(*chars)[*num_chars] = SERIALIZED_INT64;
				(*num_chars)++;
				*(int64_t*)(*chars + *num_chars) = (int64_t)var->data.i;
				(*num_chars) += 8;
				return true;
			}
			return false;
		case M_TYPE_FLOAT:
#ifdef MERCURY_64BIT
			if (!check_chars_preallocation(chars, num_chars, chars_allocated, 9))return false;
			(*chars)[*num_chars] = SERIALIZED_FLOAT64;
			(*num_chars)++;
			*(double*)(*chars + *num_chars) = (double)var->data.f;
			(*num_chars) += 8;
			return true;
#else
			if (!check_chars_preallocation(chars, num_chars, chars_allocated, 5))return false;
			(*chars)[*num_chars] = SERIALIZED_FLOAT32;
			(*num_chars)++;
			*(float*)(*chars + *num_chars) = (float)var->data.f;
			(*num_chars) += 4;
			return true;
#endif
			return false;
		case M_TYPE_BOOL:
			if (!check_chars_preallocation(chars, num_chars, chars_allocated, 1))return false;
			(*chars)[*num_chars] = var->data.i ? SERIALIZED_BOOLTRUE : SERIALIZED_BOOLFALSE;
			(*num_chars)++;
			return true;
		case M_TYPE_STRING:
			{
			mercury_string* str=(mercury_string*)var->data.p;
			if (!str->size) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 1))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZEEMPTY;
				(*num_chars)++;
				return true;
			}
			else if (str->size == 1) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 2))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZESINGLE;
				(*num_chars)++;
				(*chars)[*num_chars] = str->ptr[0];
				(*num_chars)++;
				return true;
			}
			else if (str->size <= UCHAR_MAX) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, str->size+2))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZE8;
				(*num_chars)++;
				*(unsigned char*)(*chars + *num_chars) = (unsigned char)str->size;
				(*num_chars)++;
				memcpy(*chars + *num_chars, str->ptr, str->size);
				(*num_chars)+=str->size;
				return true;
			}
			else if (str->size <= USHRT_MAX) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, str->size + 3))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZE16;
				(*num_chars)++;
				*(unsigned short*)(*chars + *num_chars) = (unsigned short)str->size;
				(*num_chars)+=2;
				memcpy(*chars + *num_chars, str->ptr, str->size);
				(*num_chars) += str->size;
				return true;
			}
			else if (str->size <= INT_MAX) {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, str->size + 5))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZE32;
				(*num_chars)++;
				*(int*)(*chars + *num_chars) = (int)str->size;
				(*num_chars) += 4;
				memcpy(*chars + *num_chars, str->ptr, str->size);
				(*num_chars) += str->size;
				return true;
			}
			else {
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, str->size + 9))return false;
				(*chars)[*num_chars] = SERIALIZED_STRING_SIZE32;
				(*num_chars)++;
				*(int64_t*)(*chars + *num_chars) = (int64_t)str->size;
				(*num_chars) += 8;
				memcpy(*chars + *num_chars, str->ptr, str->size);
				(*num_chars) += str->size;
				return true;
			}
			}
			return false;
		case M_TYPE_ARRAY:
			//if (check_pointer_serial(var->data.p, num_pointers_covered, pointerscovered))return true;
			{
			if (!check_chars_preallocation(chars, num_chars, chars_allocated, 1 + sizeof(mercury_int)))return false;
			(*chars)[*num_chars] = SERIALIZED_ARRAY_SIZEBITWIDTH;
			(*num_chars)++;
			*(mercury_int*)(*chars + *num_chars) = 0;
			mercury_int elements_offset = *num_chars;
			(*num_chars) += sizeof(mercury_int);
			mercury_int cur_elems = 0;
			mercury_array* arr = (mercury_array*)var->data.p;
			mercury_uint orig_n_ptr_conv = *num_pointers_covered;
			if (!add_serial_pointer(arr, num_pointers_covered, pointerscovered))return false;
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
							if (can_serialize_var(&var)) {
								mercury_variable indexvar;
								indexvar.type = M_TYPE_INT;
								indexvar.data.i = index;

								if (var.type == M_TYPE_ARRAY || var.type == M_TYPE_TABLE) {
									if (check_pointer_serial(var.data.p, num_pointers_covered, pointerscovered))continue;
								}

								if (!m_serialize_variable(&indexvar, chars, num_chars, chars_allocated, pointerscovered, num_pointers_covered))return false;
								if (!m_serialize_variable(&var, chars, num_chars, chars_allocated, pointerscovered, num_pointers_covered))return false;

								cur_elems++;
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

			*(mercury_int*)(*chars + elements_offset) = cur_elems;
			*num_pointers_covered = orig_n_ptr_conv;
			return true;
			}
			return false;
		case M_TYPE_TABLE:
			//if (check_pointer_serial(var->data.p, num_pointers_covered, pointerscovered))return true;
			{
				if (!check_chars_preallocation(chars, num_chars, chars_allocated, 1 + sizeof(mercury_int)))return false;
				(*chars)[*num_chars] = SERIALIZED_TABLE_SIZEBITWIDTH;
				(*num_chars)++;
				*(mercury_int*)(*chars + *num_chars) = 0;
				mercury_int elements_offset =  *num_chars;
				(*num_chars) += sizeof(mercury_int);
				mercury_int cur_elems = 0;
				mercury_table* tab = (mercury_table*)var->data.p;
				mercury_uint orig_n_ptr_conv = *num_pointers_covered;
				if (!add_serial_pointer(tab, num_pointers_covered, pointerscovered))return false;

				for (uint8_t t = 0; t < M_NUMBER_OF_TYPES; t++) {
					mercury_subtable st = tab->data[t];
					for (mercury_int i = 0; i < st.size; i++) {
						if (can_serialize_var(st.values+i) && can_serialize_var(st.keys + i)) {
							if(st.keys[i].type==M_TYPE_ARRAY || st.keys[i].type == M_TYPE_TABLE){
								if(check_pointer_serial(st.keys[i].data.p,num_pointers_covered,pointerscovered))continue;
							}
							if (st.values[i].type == M_TYPE_ARRAY || st.values[i].type == M_TYPE_TABLE) {
								if (check_pointer_serial(st.values[i].data.p, num_pointers_covered, pointerscovered))continue;
							}
							if (!m_serialize_variable(st.keys+i, chars, num_chars, chars_allocated, pointerscovered, num_pointers_covered))return false;
							if (!m_serialize_variable(st.values+i, chars, num_chars, chars_allocated, pointerscovered, num_pointers_covered))return false;
							cur_elems++;
						}

					}
				}

				*(mercury_int*)(*chars + elements_offset) = cur_elems;
				*num_pointers_covered = orig_n_ptr_conv;
				return true;
			}
			return false;
		case M_TYPE_NIL:
		default:
			if (!check_chars_preallocation(chars, num_chars, chars_allocated, 1))return false;
			(*chars)[*num_chars] = SERIALIZED_NIL;
			(*num_chars)++;
			return true;
	}
	return false;
}


void mercury_lib_io_serialize(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable in;
	mercury_popstack(M, &in);
	mercury_variable out;

	unsigned char* chars = nullptr;
	mercury_int num_chars=0;
	mercury_int chars_allocated=0;
	void** pointerscovered = nullptr;
	mercury_uint num_pointers_covered = 0;


	if (m_serialize_variable(&in, &chars, &num_chars, &chars_allocated, &pointerscovered, &num_pointers_covered)) {
		mercury_string* str=(mercury_string*)malloc(sizeof(mercury_string));
		if (!str) {
			free(chars);
			free(pointerscovered);
			mercury_raise_error_nonpointer(M, M_ERROR_ALLOCATION);
			return;
		}
		str->ptr = (char*)chars;
		str->constant = false;
		str->refrences = 1;
		str->size = num_chars;
		out.data.p = str;
		out.type = M_TYPE_STRING;
	}
	else {
		free(chars);
		free(pointerscovered);
		out.type = M_TYPE_NIL;
		out.data.i = 0;
	}

	mercury_pushstack_unrefed(M,&out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


inline bool deser_chars_avalible(mercury_int size,mercury_int position,mercury_int requested) {
#ifdef MERCURY_DEBUG 
	if (!(position + requested <= size)) {
		printf("deserialize failed to find needed chars! we need %zi more, but only have %zi remaining\n",requested,size-position);
	}
#endif
	return position + requested <= size;
}

bool m_deserialize_variable(mercury_variable* out,const unsigned char* chars,const mercury_int size,mercury_int* position) {
	if (!deser_chars_avalible(size, *position, 1))return false;
	unsigned char cur_char = chars[*position];
	switch (cur_char) {
		case SERIALIZED_NIL:
			out->data.i = 0;
			out->type = M_TYPE_NIL;
			(*position)++;
			return true;
		case SERIALIZED_BOOLFALSE:
			out->data.i = 0;
			out->type = M_TYPE_BOOL;
			(*position)++;
			return true;
		case SERIALIZED_BOOLTRUE:
			out->data.i = 1;
			out->type = M_TYPE_BOOL;
			(*position)++;
			return true;
		case SERIALIZED_INT8:
			(*position)++;
			if(!deser_chars_avalible(size, *position, 1))return false;
			out->data.i = *(signed char*)(chars+*position);
			out->type = M_TYPE_INT;
			(*position)++;
			return true;
		case SERIALIZED_INT8UNSIGNED:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 1))return false;
			out->data.u = *(unsigned char*)(chars + *position);
			out->type = M_TYPE_INT;
			(*position)++;
			return true;
		case SERIALIZED_INT16:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 2))return false;
			out->data.i = *(short*)(chars + *position);
			out->type = M_TYPE_INT;
			(*position)+=2;
			return true;
		case SERIALIZED_INT32:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 4))return false;
			out->data.i = *(int*)(chars + *position);
			out->type = M_TYPE_INT;
			(*position) += 4;
			return true;
		case SERIALIZED_INT64:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 8))return false;
			out->data.i = *(int64_t*)(chars + *position);
			out->type = M_TYPE_INT;
			(*position) += 8;
			return true;
		case SERIALIZED_FLOAT32:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 4))return false;
			out->data.f = *(float*)(chars + *position);
			out->type = M_TYPE_FLOAT;
			(*position) += 4;
			return true;
		case SERIALIZED_FLOAT64:
			(*position)++;
			if (!deser_chars_avalible(size, *position, 8))return false;
			out->data.f = *(double*)(chars + *position);
			out->type = M_TYPE_FLOAT;
			(*position) += 8;
			return true;
		case SERIALIZED_STRING_SIZEEMPTY:
			{
			(*position)++;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			out->type = M_TYPE_STRING;
			out->data.p = str;
			}
			return true;
		case SERIALIZED_STRING_SIZESINGLE:
		{
			(*position)++;
			if (!deser_chars_avalible(size, *position, 1))return false;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			str->size = 1;
			str->ptr = (char*)malloc(1);
			if (!str->ptr) { free(str); return false; }
			str->ptr[0] = chars[*position];
			(*position)++;
			out->type = M_TYPE_STRING;
			out->data.p = str;
		}
			return true;
		case SERIALIZED_STRING_SIZE8:
		{
			(*position)++;

			if (!deser_chars_avalible(size, *position, 1))return false;
			mercury_int needed_size=*(unsigned char*)(chars+*position);
			(*position)++;

			if (!deser_chars_avalible(size, *position, needed_size))return false;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			str->size = needed_size;
			str->ptr = (char*)malloc(needed_size);
			if (!str->ptr) { free(str); return false; }
			memcpy(str->ptr, chars + *position, needed_size);
			(*position) += needed_size;
			out->type = M_TYPE_STRING;
			out->data.p = str;
		}
			return true;
		case SERIALIZED_STRING_SIZE16:
		{
			(*position)++;

			if (!deser_chars_avalible(size, *position, 2))return false;
			mercury_int needed_size = *(unsigned short*)(chars + *position);
			(*position)+=2;

			if (!deser_chars_avalible(size, *position, needed_size))return false;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			str->size = needed_size;
			str->ptr = (char*)malloc(needed_size);
			if (!str->ptr) { free(str); return false; }
			memcpy(str->ptr, chars + *position, needed_size);
			(*position) += needed_size;
			out->type = M_TYPE_STRING;
			out->data.p = str;
		}
			return true;
		case SERIALIZED_STRING_SIZE32:
		{
			(*position)++;

			if (!deser_chars_avalible(size, *position, 4))return false;
			mercury_int needed_size = *(int*)(chars + *position);
			(*position) += 4;

			if (!deser_chars_avalible(size, *position, needed_size))return false;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			str->size = needed_size;
			str->ptr = (char*)malloc(needed_size);
			if (!str->ptr) { free(str); return false; }
			memcpy(str->ptr, chars + *position, needed_size);
			(*position) += needed_size;
			out->type = M_TYPE_STRING;
			out->data.p = str;
		}
			return true;
		case SERIALIZED_STRING_SIZE64:
		{
			(*position)++;

			if (!deser_chars_avalible(size, *position, 8))return false;
			mercury_int needed_size = *(int*)(chars + *position);
			(*position) += 8;

			if (!deser_chars_avalible(size, *position, needed_size))return false;
			mercury_string* str = (mercury_string*)malloc(sizeof(mercury_string));
			if (!str)return false;
			memset(str, 0, sizeof(mercury_string));
			str->refrences = 1;
			str->size = needed_size;
			str->ptr = (char*)malloc(needed_size);
			if (!str->ptr) { free(str); return false; }
			memcpy(str->ptr, chars + *position, needed_size);
			(*position) += needed_size;
			out->type = M_TYPE_STRING;
			out->data.p = str;
		}
			return true;
		case SERIALIZED_ARRAY_SIZEBITWIDTH:
			{
				(*position)++;
				mercury_array* arr=mercury_newarray();
				if (!deser_chars_avalible(size, *position, sizeof(mercury_int) ))return false;
				if (!arr)return false;
				mercury_int asize = *(mercury_int*)(chars + *position);
				(*position) += sizeof(mercury_int);
				while (asize) {
					mercury_variable key;
					mercury_variable value;
					if (!m_deserialize_variable(&key, chars, size, position))return false;
					if (key.type != M_TYPE_INT) {
						mercury_free_var(&key);
						return false;
					}
					if (!m_deserialize_variable(&value, chars, size, position))return false;
					if (!mercury_setarray(arr, &value, key.data.i)) {
						mercury_free_var(&value);
						return false;
					}
					asize--;
				}
				out->type = M_TYPE_ARRAY;
				out->data.p = arr;
			}
			return true;
		case SERIALIZED_TABLE_SIZEBITWIDTH:
		{
			(*position)++;
			mercury_table* arr = mercury_newtable();
			if (!deser_chars_avalible(size, *position, sizeof(mercury_int)))return false;
			if (!arr)return false;
			mercury_int asize = *(mercury_int*)(chars + *position);
			(*position) += sizeof(mercury_int);
			while (asize) {
				mercury_variable key;
				mercury_variable value;
				if (!m_deserialize_variable(&key, chars, size, position))return false;
				if (!m_deserialize_variable(&value, chars, size, position)) {
					mercury_free_var(&key);
					return false;
				};
				if (mercury_setkey(arr, &key,&value)==-1) {
					mercury_free_var(&key);
					mercury_free_var(&value);
					return false;
				}
				asize--;
			}
			out->type = M_TYPE_TABLE;
			out->data.p = arr;
		}	
			return true;
		default:
			return false;
	}
}

void mercury_lib_io_deserialize(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1)) {
		MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out);
		return;
	}
	if (!args_out) {
		mercury_discard_top_of_stack(M);
		return;
	}

	mercury_variable in;
	mercury_popstack(M, &in);

	if (in.type != M_TYPE_STRING) {
		mercury_free_var(&in);
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_STRING, 1);
		return;
	}

	mercury_variable out;
	out.data.i = 0;
	out.type = M_TYPE_NIL;

	mercury_int pos = 0;
	mercury_string* str = (mercury_string*)in.data.p;

	if (!m_deserialize_variable(&out, (unsigned char*)str->ptr, str->size, &pos)) {
		mercury_free_var(&out);
		out.data.i = 0;
		out.type = M_TYPE_NIL;
	}


	mercury_free_var(&in);
	mercury_pushstack_unrefed(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}
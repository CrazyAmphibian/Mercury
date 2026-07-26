#include "../mercury.hpp"
#include "../mercury_error.hpp"
#include "mercury_lib_io.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>


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
	if (!args_out)return;

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

	mercury_pushstack(M, &out);

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
	if (!args_out)return;

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
	if (!args_out)return;

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
	if (!args_out)return;

	mercury_variable fil_var;
	mercury_popstack(M, &fil_var);
	if (fil_var.type != M_TYPE_FILE) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, fil_var.type, M_TYPE_FILE, 1);
		return;
	}

	
	mercury_array* arr = mercury_newarray();

	mercury_int bsize = 256;
	mercury_int cbuf = 0;
	char* buf = (char*)malloc(bsize);
	if (!buf) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	mercury_int count = 0;

	mercury_filewrapper* fw = (mercury_filewrapper*)fil_var.data.p;
	if (fw->open) {
		FILE* f = fw->file;

		while (true) {
			int c=fgetc(f);
			if (c == '\n' || c == '\r' || c==EOF) {
				if (cbuf) {
					mercury_string* s= mercury_cstring_to_mstring(buf,cbuf);
					if (!s) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						return;
					}
					mercury_variable v;
					
					v.type = M_TYPE_STRING;
					v.data.p = s;
					mercury_setarray(arr, &v, count);
					count++;
					cbuf = 0;
				}
				if (c == EOF)break;
			}
			else {
				buf[cbuf] = (char)c;
				cbuf++;
				if (cbuf >= bsize) {
					void* n=realloc(buf, bsize * 2);
					if (!n) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						free(buf);
						rewind(f);
						return;
					}
					buf = (char*)n;
					bsize *= 2;
				}
			}
		}
		rewind(f);
	}
	free(buf);

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

#include "../mercury.h"
#include "../mercury_error.h"

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




void mercury_lib_io_open(mercury_state* M, mercury_int args_in, mercury_int args_out) { //opens a file. nuff said.
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	if (!args_out)return;
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* mode_var = mercury_popstack(M);
	if (mode_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)mode_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* file_var = mercury_popstack(M);
	if (file_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)file_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* out=mercury_assign_var(M);

	char* mode = mercury_mstring_to_cstring((mercury_stringliteral*)mode_var->data.p);
	char* file = mercury_mstring_to_cstring((mercury_stringliteral*)file_var->data.p);



	FILE* F=fopen(file,mode);
	if (F) {
		out->type = M_TYPE_FILE;
		

		mercury_filewrapper* fw = (mercury_filewrapper*)malloc(sizeof(mercury_filewrapper));
		if (!fw) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			fclose(F);
			return;
		}
		fw->refrences = 1;
		fw->open = true;
		fw->file = F;

		out->data.p = fw;
	}
	else {
		out->type = M_TYPE_NIL;
		out->data.i = 0;
	}

	mercury_unassign_var(M, file_var);
	mercury_unassign_var(M, mode_var);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_io_read(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out)return;
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* file_var = mercury_popstack(M);
	if (file_var->type != M_TYPE_FILE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)file_var->type, (void*)M_TYPE_FILE);
		return;
	}

	mercury_variable* out = mercury_assign_var(M);

	mercury_filewrapper* fw= (mercury_filewrapper*)file_var->data.p;
	FILE* F = fw->file;

	if (F && fw->open) {

		if (fseek(F, 0, SEEK_END)) {
			out->type = M_TYPE_NIL;
			out->data.i = 0;
		}
		else {
			mercury_int len = ftell(F);
			if (len == -1) {
				out->type = M_TYPE_NIL;
				out->data.i = 0;
			}
			else {
				char* s = (char*)malloc(sizeof(char) * len);
				if (!s) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				rewind(F);
				fread(s, 1, len, F);
				mercury_stringliteral* str = (mercury_stringliteral*)malloc(sizeof(mercury_stringliteral));
				if (!str) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					return;
				}
				str->ptr = s;
				str->size = len;

				out->type = M_TYPE_STRING;
				out->data.p = str;
			}
		}
	}
	else {
		out->type = M_TYPE_NIL;
		out->data.i = 0;
	}

	mercury_unassign_var(M, file_var);

	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_io_close(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* file_var = mercury_popstack(M);
	if (file_var->type != M_TYPE_FILE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)file_var->type, (void*)M_TYPE_FILE);
		return;
	}

	mercury_filewrapper* fw = (mercury_filewrapper*)file_var->data.p;
	if (fw->open) {
		fw->open = false;
		if(fw->file)fclose(fw->file);
	}

	mercury_unassign_var(M, file_var);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_io_write(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	if (args_in < 2) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)2);
		return;
	};
	for (mercury_int i = 2; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* data_var = mercury_popstack(M);
	if (data_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)data_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_variable* file_var = mercury_popstack(M);
	if (file_var->type != M_TYPE_FILE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)file_var->type, (void*)M_TYPE_FILE);
		return;
	}
	mercury_stringliteral* str = (mercury_stringliteral*)data_var->data.p;
	mercury_filewrapper* fw = (mercury_filewrapper*)file_var->data.p;

	if (fw->open) {
		fwrite(str->ptr, 1, str->size, fw->file);
	}
	mercury_unassign_var(M, data_var);
	mercury_unassign_var(M, file_var);


	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_io_getfiles(mercury_state* M, mercury_int args_in, mercury_int args_out) { //an array of strings
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out)return;
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* dir_var = mercury_popstack(M);
	if (dir_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)dir_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_stringliteral* mstr = (mercury_stringliteral*)dir_var->data.p;
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)"*", 1);
	}
	else {
		mercury_mstring_addchars(mstr, (char*)"/*", 2);
	}
	char* dir = mercury_mstring_to_cstring(mstr);
	mercury_variable* arr_v=mercury_assign_var(M);
	mercury_array* arr=mercury_newarray();
	arr_v->data.p = arr;
	arr_v->type = M_TYPE_ARRAY;

	mercury_int num_fs = 0;


#if defined(_WIN32) || defined(_WIN64) //windows
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind=FindFirstFileA(dir, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		while (true) {
			if (!(FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DEVICE) )) {
				char* fn = FindFileData.cFileName;
				mercury_stringliteral* s = mercury_cstring_to_mstring(fn, strlen(fn));
				mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
				if (!v) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					FindClose(hFind);
					return;
				}
				v->type = M_TYPE_STRING;
				v->data.p = s;
				mercury_setarray(arr, v, num_fs);
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
				mercury_stringliteral* s = mercury_cstring_to_mstring(ent->d_name, strlen(ent->d_name));
				mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
				if (!v) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					closedir(d);
					return;
				}
				v->type = M_TYPE_STRING;
				v->data.p = s;
				mercury_setarray(arr, v, num_fs);
				num_fs++;
			}
		}
		closedir(d);
	}
#endif
	mercury_unassign_var(M, dir_var);

	mercury_pushstack(M, arr_v);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_io_getdirs(mercury_state* M, mercury_int args_in, mercury_int args_out) { //an array of strings
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out)return;
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* dir_var = mercury_popstack(M);
	if (dir_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)dir_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_stringliteral* mstr = (mercury_stringliteral*)dir_var->data.p;
	if (mstr->size == 0) {
		mercury_mstring_addchars(mstr, (char*)"*", 1);
	}
	else {
		mercury_mstring_addchars(mstr, (char*)"/*", 2);
	}
	char* dir = mercury_mstring_to_cstring(mstr);

	mercury_variable* arr_v = mercury_assign_var(M);
	mercury_array* arr = mercury_newarray();
	arr_v->data.p = arr;
	arr_v->type = M_TYPE_ARRAY;

	mercury_int num_fs = 0;


#if defined(_WIN32) || defined(_WIN64) //windows
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(dir, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		while (true) {
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(FindFileData.cFileName,".") && strcmp(FindFileData.cFileName, "..")) {
				char* fn = FindFileData.cFileName;
				mercury_stringliteral* s = mercury_cstring_to_mstring(fn, strlen(fn));
				mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
				if (!v) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					FindClose(hFind);
					return;
				}
				v->type = M_TYPE_STRING;
				v->data.p = s;
				mercury_setarray(arr, v, num_fs);
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
				mercury_stringliteral* s = mercury_cstring_to_mstring(ent->d_name, strlen(ent->d_name));
				mercury_variable* v = (mercury_variable*)malloc(sizeof(mercury_variable));
				if (!v) {
					mercury_raise_error(M, M_ERROR_ALLOCATION);
					closedir(d);
					return;
				}
				v->type = M_TYPE_STRING;
				v->data.p = s;
				mercury_setarray(arr, v, num_fs);
				num_fs++;
			}
		}
		closedir(d);
	}
#endif

	mercury_unassign_var(M, dir_var);

	mercury_pushstack(M, arr_v);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




void mercury_lib_io_lines(mercury_state* M, mercury_int args_in, mercury_int args_out) { //returns an array of the lines, sans newline characters.
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	if (!args_out)return;
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* fil_var = mercury_popstack(M);
	if (fil_var->type != M_TYPE_FILE) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)fil_var->type, (void*)M_TYPE_FILE);
		return;
	}

	mercury_variable* out = mercury_assign_var(M);
	mercury_array* arr = mercury_newarray();

	mercury_int bsize = 256;
	mercury_int cbuf = 0;
	char* buf = (char*)malloc(bsize);
	if (!buf) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}
	mercury_int count = 0;

	mercury_filewrapper* fw = (mercury_filewrapper*)fil_var->data.p;
	if (fw->open) {
		FILE* f = fw->file;

		while (true) {
			int c=fgetc(f);
			if (c == '\n' || c == '\r' || c==EOF) {
				if (cbuf) {
					mercury_stringliteral* s= mercury_cstring_to_mstring(buf,cbuf);
					mercury_variable* v=(mercury_variable*)malloc(sizeof(mercury_variable));
					if (!v) {
						mercury_raise_error(M, M_ERROR_ALLOCATION);
						rewind(f);
						return;
					}
					v->type = M_TYPE_STRING;
					v->data.p = s;
					mercury_setarray(arr, v, count);
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

	mercury_unassign_var(M, fil_var);
	

	out->type = M_TYPE_ARRAY;
	out->data.p = arr;
	mercury_pushstack(M, out);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_io_post(mercury_state* M, mercury_int args_in, mercury_int args_out) { //send characters to stdout directly
	if (args_in < 1) {
		mercury_raise_error(M, M_ERROR_NOT_ENOUGH_ARGS, (void*)args_in, (void*)1);
		return;
	};
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* str_var = mercury_popstack(M);
	if (str_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)str_var->type, (void*)M_TYPE_STRING);
		return;
	}

	mercury_stringliteral* s = (mercury_stringliteral*)str_var->data.p;
	for (mercury_int c = 0; c < s->size; c++) {
		putchar(s->ptr[c]);
	}
	mercury_unassign_var(M, str_var);

	for (mercury_int a = 0; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

void mercury_lib_io_prompt(mercury_state* M, mercury_int args_in, mercury_int args_out) { //read a line from stdin
	for (mercury_int i = 0; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
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
		mercury_stringliteral* s=mercury_cstring_to_mstring(c, len);
		mercury_variable* v=mercury_assign_var(M);
		v->type = M_TYPE_STRING;
		v->data.p = s;
		mercury_pushstack(M, v);
	}
	free(c);


	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_io_remove(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* dir_var = mercury_popstack(M);
	if (dir_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)dir_var->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* fst = (mercury_stringliteral*)dir_var->data.p;

	char* cfilestr=mercury_mstring_to_cstring(fst);


	int r=remove(cfilestr);

	if (args_out) {
		mercury_variable* out=mercury_assign_var(M);
		out->type = M_TYPE_BOOL;
		out->data.i = r != 0 ? 1 : 0;
	}

	mercury_unassign_var(M, dir_var);
	free(cfilestr);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}


void mercury_lib_io_removedir(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* dir_var = mercury_popstack(M);
	if (dir_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)dir_var->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* fst = (mercury_stringliteral*)dir_var->data.p;

	char* cfilestr = mercury_mstring_to_cstring(fst);

#if defined(_WIN32) || defined(_WIN64)
	int r = _rmdir(cfilestr);
#else
	int r = rmdir(cfilestr);
#endif

	if (args_out) {
		mercury_variable* out = mercury_assign_var(M);
		out->type = M_TYPE_BOOL;
		out->data.i = r != 0 ? 1 : 0;
	}

	mercury_unassign_var(M, dir_var);
	free(cfilestr);
	

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}




void mercury_lib_io_createdir(mercury_state* M, mercury_int args_in, mercury_int args_out) {
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
	}

	mercury_variable* dir_var = mercury_popstack(M);
	if (dir_var->type != M_TYPE_STRING) {
		mercury_raise_error(M, M_ERROR_WRONG_TYPE, (void*)dir_var->type, (void*)M_TYPE_STRING);
		return;
	}
	mercury_stringliteral* fst = (mercury_stringliteral*)dir_var->data.p;

	char* cfilestr = mercury_mstring_to_cstring(fst);
#if defined(_WIN32) || defined(_WIN64)
	int r = _mkdir(cfilestr);
#else
	int r = mkdir(cfilestr,0755); //rwxr-xr-x
#endif
	

	if (args_out) {
		mercury_variable* out = mercury_assign_var(M);
		out->type = M_TYPE_BOOL;
		out->data.i = r != 0 ? 1 : 0;
	}

	mercury_unassign_var(M, dir_var);
	free(cfilestr);

	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}



void mercury_lib_io_input(mercury_state* M, mercury_int args_in, mercury_int args_out) { //read a single char stdin. no newline required! (platform dependant, though. :/)
	for (mercury_int i = 1; i < args_in; i++) {
		mercury_unassign_var(M, mercury_popstack(M));
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
		mercury_stringliteral* s = mercury_cstring_to_mstring(&c, 1);
		mercury_variable* v = mercury_assign_var(M);
		v->type = M_TYPE_STRING;
		v->data.p = s;
		mercury_pushstack(M, v);
	}


	for (mercury_int a = 1; a < args_out; a++) {
		mercury_variable* mv = mercury_assign_var(M);
		mv->type = M_TYPE_NIL;
		mv->data.i = 0;
		mercury_pushstack(M, mv);
	}
}

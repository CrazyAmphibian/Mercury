#include "mercury.hpp"
#include "mercury_compiler.hpp"
#include "mercury_bytecode.hpp"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

inline bool arg_is_string(const char* arg, const char* check) {
	return !strncmp(arg, check, strlen(check));
}

int main(int argc, char** argv) {
	bool interactivemode = false;
	mercury_array* arg_arr=mercury_newarray();
	
	char* code=nullptr;// = (char*)"";
	const char* fpath = nullptr;

	int arg_offset = 1;
	bool fileread = false;
	for (int i = 1; i < argc; i++) { //start at 1 because 0 is the executable path.
		//printf("\t%i %s\n", i, argv[i]);
		arg_offset++;
		if (arg_is_string(argv[i],"--help") || arg_is_string(argv[i], "-?") || arg_is_string(argv[i], "-h") || arg_is_string(argv[i], "/?")) {
			printf("\nusage: mercury [options] <source file> [arguments]\n");
			printf("\nMercury will load the first argument listed that is not in the [options] list as the input file. All arguments after will be passed to the runtime under the _ARGS global array starting at index 0.\n");
			printf("\n[options]\n");
			printf("%-20s Display this text, then exits\n", "-?, -h, --help, /?");
			printf("%-20s Displays the program version, then exits\n", "-v, --version, /v");
			printf("%-20s Enables interactive mode. Implied if no file is provided\n", "-i, /i");
			printf("%-20s Starts Mercury with no input file, treating all following args as input args. Implies interactive mode.\n", "--no-file, -n, /n");
			return 0;
		}
		else if(arg_is_string(argv[i], "--version") || arg_is_string(argv[i], "-v") || arg_is_string(argv[i], "/v")){
			printf("Merecury version %i.%i, %u bit\n", MERCURY_VERSION, MERCURY_VERSION_PATCH, (unsigned int)(sizeof(mercury_int) << 3));
			return 0;
		}
		else if (arg_is_string(argv[i], "-i") || arg_is_string(argv[i], "/i")) {
			interactivemode = true;
		}
		else if (arg_is_string(argv[i],"--no-file") || arg_is_string(argv[i], "-n") || arg_is_string(argv[i], "/n")) {
			interactivemode = true;
			break;
		}
		else {
			fpath = argv[i];
			break;
		}
		
	}

	for (int i = 0; i < argc; i++) {
		mercury_variable av;
		av.type = M_TYPE_STRING;
		av.data.p = mercury_cstring_to_mstring(argv[i], strlen(argv[i]));
		mercury_setarray(arg_arr, &av, (i- arg_offset) );
	}

	if (fpath) {
		const char* fpath = argv[argc-1];
		FILE* f=fopen(fpath,"rb");
		if (!f) {
			printf("error opening file %s\n",fpath);
			return 1;
		}
		if (fseek(f, 0, SEEK_END)) {
			printf("error reading file (1).\n");
			return 1;
		}
		else {
			mercury_int len = ftell(f);
			//printf("length is %lli\n",len);
			if (len == -1) {
				printf("error reading file (2).\n");
				return 1;
			}
			else {
				char* s = (char*)malloc(sizeof(char) * (len+1) );
				if (!s) {
					printf("error reading file %s: unable to allocate memory.\n",fpath);
					return 1;
				}
				rewind(f);
				fread(s, 1, len, f);
				s[len] = '\0';

				code = s;
				//printf("%s", code);
			}
		}
		fclose(f);

	}
	else {
		//printf("no file supplied.\n");
		//return -1;
		//code = (char*)"print(\"hello, world!\")";
		interactivemode = true;
	}

	if (interactivemode) {
		//printf("Mercury Alpha %i (c)2026 interactive mode\n",MERCURY_VERSION_PATCH);
		printf("Mercury %i.%i (c)2025 interactive mode\n",MERCURY_VERSION,MERCURY_VERSION_PATCH); //uncomment for later when this gets out of alpha (yeah right)
	}

	

	mercury_state* M=mercury_newstate();

	mercury_variable at_v;
	at_v.type = M_TYPE_ARRAY;
	at_v.data.p = arg_arr;
	mercury_table_set_cstring_keyvalue(M->enviroment,"_ARGS",&at_v);

	mercury_populate_enviroment_with_libs(M);

#ifdef MERCURY_DEBUG
	mercury_debugdumptable(M->enviroment);
#endif

	start:
	if (interactivemode) {
		putchar('>');
		mercury_int sizec = 200;
		mercury_int len = 0;
		char* c = (char*)malloc(sizec);
		if (!c) {
			return -1;
		}


		int ch = 0;
		while ((ch = fgetc(stdin)) != EOF && ch != '\n' && ch != '\r') {
				c[len] = ch;
				len++;
				if (len >= sizec) {
					sizec += 200;
					void* n = realloc(c, sizec);
					if (!n) {
						return -1;
					}
					c = (char*)n;
				}
		}
		c[len] = '\0';
		code = c;
	}


	mercury_string* tstr = mercury_cstring_const_to_mstring((char*)code, strlen(code));
	mercury_variable funcy;
	mercury_compile_mstring(tstr,&funcy);
	if (funcy.type != M_TYPE_FUNCTION) {
		if (funcy.type == M_TYPE_STRING) {
			mercury_string* s = (mercury_string*)funcy.data.p;
			for (mercury_int n = 0; n < s->size; n++) {
				putchar(s->ptr[n]);
			}
			putchar('\n');
		}
		else {
			printf("failed to compile, unknown error\n");
		}
		if (!interactivemode)return 1;
	}
	else {

		mercury_function* compiled = (mercury_function*)funcy.data.p;

#ifdef MERCURY_DEBUG
		mercury_string* rs= mercury_get_bytecode_debug(compiled);
		if (rs) {
			const char* str = rs->ptr;
			for (mercury_int c = 0; c < (const mercury_int)rs->size; c++) {
				putchar(str[c]);
			}
		}
#endif

		M->programcounter = 0;

		M->bytecode.instructions = compiled->instructions;
		M->bytecode.numberofinstructions = compiled->numberofinstructions;
		M->bytecode.dbg_tokens = compiled->dbg_tokens;
		M->bytecode.num_dbg_tokens = compiled->num_dbg_tokens;
		M->bytecode.instruction_dbg_lookup = compiled->instruction_dbg_lookup;

		//printf("current stack: %i\n", M->sizeofstack);

		while (mercury_stepstate(M));
	}

	if (interactivemode) {
		if (funcy.type == M_TYPE_FUNCTION) {
			M->programcounter = 0;
			free(code);
			M->bytecode.numberofinstructions = 0;

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
			M->bytecode.num_dbg_tokens = 0;

			

			for (mercury_uint i = 0; i < M->sizeofstack;i++) {
				mercury_variable v;
				mercury_popstack(M, &v);
				mercury_free_var(&v); //clean up the stack
			}
		}
		goto start;
	}

	return 0;
}
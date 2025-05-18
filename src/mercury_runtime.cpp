#include "mercury.h"
#include "mercury_compiler.h"
#include "mercury_bytecode.h"

#include "stdio.h"
#include "malloc.h"
#include "string.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif





void mercury_debugdumpbytecode(uint32_t* instructions, mercury_int number_instructions) {
	mercury_int offset = 0;


	while (offset < number_instructions)
	{
		uint16_t instr = instructions[offset] & 0xFFFF;
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
		case M_OPCODE_SWPT:
			printf("SWPT\n");
			break;
		case M_OPCODE_CLS:
			printf("CLS \n");
			break;
		case M_OPCODE_EXIT:
			printf("EXIT\n");
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











int main(int argc, char** argv) {
	
#ifdef _WIN32
	HMODULE mercury_main = LoadLibraryA("Mercury.dll");
	if (!mercury_main) {
		printf("failed to load mercury dynamic link.\n");
		return 2;
	}
	FreeLibrary(mercury_main);
#else
	void* mercury_main = dlopen("mercury.so", RTLD_LAZY);
	if (!mercury_main) {
		printf("failed to load mercury dynamic link.\n");
		return 2;
	}
	dlclose(mercury_main);
#endif

	bool interactivemode = false;

	
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
	
	char* code;// = (char*)"";


	if (argc>=2) {
		const char* fpath= argv[1];
		FILE* f=fopen(fpath,"rb");
		if (!f) {
			printf("error opening file.\n");
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
					printf("error reading file (3).\n");
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
		code = (char*)"print(\"hello, world!\")";
		interactivemode = true;
	}


	

	mercury_state* M=mercury_newstate();

	mercury_variable* at_v =mercury_assign_var(M);
	mercury_variable* atk_v = mercury_assign_var(M);

	at_v->type = M_TYPE_ARRAY;
	at_v->data.p = arg_arr;
	atk_v->type = M_TYPE_STRING;
	atk_v->data.p = mercury_cstring_const_to_mstring((char*)"_ARGS",5);
	mercury_setkey(M->enviroment, atk_v, at_v);
	mercury_populate_enviroment_with_libs(M);


	start:
	if (interactivemode) {
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
		printf("%s\n", code);
	}


	mercury_stringliteral* tstr = mercury_cstring_const_to_mstring((char*)code, strlen(code));
	mercury_variable* funcy = mercury_compile_mstring(tstr);

	if (funcy->type != M_TYPE_FUNCTION) {
		if (funcy->type == M_TYPE_STRING) {
			mercury_stringliteral* s = (mercury_stringliteral*)funcy->data.p;
			for (mercury_int n = 0; n < s->size; n++) {
				putchar(s->ptr[n]);
			}
			putchar('\n');
		}
		if (!interactivemode)return 1;
	}
	else {

		mercury_function* compiled = (mercury_function*)funcy->data.p;

#if defined(DEBUG) || defined(_DEBUG)
		mercury_debugdumpbytecode(compiled->instructions, compiled->numberofinstructions);
#endif

		M->programcounter = 0;

		M->bytecode.instructions = compiled->instructions;
		M->bytecode.numberofinstructions = compiled->numberofinstructions;
		M->bytecode.debug_info = compiled->debug_info;

		//printf("current stack: %i\n", M->sizeofstack);

		while (mercury_stepstate(M));
	}

	if (interactivemode) {

		free(code);
		free(M->bytecode.instructions);
		M->bytecode.numberofinstructions = 0;
		free(M->bytecode.debug_info);
		goto start;
	}

	return 0;
}
#include "mercury_compiler.h"
#include "mercury_bytecode.h"
#include "mercury_error.h"
#include "mercury.h"
#include "malloc.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"


mercury_int COMPILER_NUMBER_ARGS_OUT = 0;
mercury_int COMPILER_LOOP_START = 0;
mercury_int* COMPILER_LOOP_ENDJMPS = nullptr;
mercury_int COMPILER_LOOP_ENDCOUNT = 0;

mercury_int POSITION_IN_DATASTRUCTURE = 0; //arrays and tables [1,2,3] 0,1,2

mercury_int COMPILER_CONTINUE_JUMP_POSITION = 0; // continue jumps to the start. far easier to implement.
bool COMPILER_INSIDE_LOOP = false;
mercury_int COMPILER_BREAK_AMOUNTS = 0;
mercury_int* COMPILER_BREAK_ADDRS = nullptr; //array of positions where we need them to jump to the end.



struct JUMP_POINT {
	char* label;
	mercury_int label_size;
	mercury_int* positions;
	mercury_int num_positions;
	mercury_int label_point;
	bool defined = true; //if not defined, should act like a NOP
};
/* so basically
"label:"
goto label is at instruction #12, goto label is at instruction #91
is at instruction number #123
has been defined.
*/

JUMP_POINT** COMPILER_JUMP_DATABASE = nullptr;
mercury_int COMPILER_JUMP_NUMBERS = 0;

bool m_compile_register_jump_point_goto(char* label, mercury_int label_size, mercury_int position) {
	for (mercury_int i = 0; i < COMPILER_JUMP_NUMBERS; i++) {
		JUMP_POINT* JP = COMPILER_JUMP_DATABASE[i];
		if (JP->label_size == label_size) {
			for (mercury_int c = 0; c < label_size; c++){
				if (label[c] != JP->label[c])goto next;
			}
			void* npt = realloc(JP->positions,sizeof(mercury_int)*(JP->num_positions+1));
			if (!npt)return false;
			JP->positions = (mercury_int*)npt;
			JP->positions[JP->num_positions] = position;
			JP->num_positions++;
			return true;
		}
		next:
		{}
	}
	void* npt=realloc(COMPILER_JUMP_DATABASE,sizeof(JUMP_POINT*)*(COMPILER_JUMP_NUMBERS+1));
	if (!npt)return false;
	COMPILER_JUMP_DATABASE = (JUMP_POINT**)npt;

	JUMP_POINT* J=(JUMP_POINT*)malloc(sizeof(JUMP_POINT));
	if (!J)return false;

	J->defined = false;
	J->label = (char*)malloc(sizeof(char)*label_size);
	if (!J->label)return false;
	memcpy(J->label, label, label_size * sizeof(char));
	J->label_point = 0;
	J->label_size = label_size;
	J->num_positions = 1;
	J->positions = (mercury_int*)malloc(sizeof(mercury_int));
	if (!J->positions)return false;
	J->positions[0] = position;

	COMPILER_JUMP_DATABASE[COMPILER_JUMP_NUMBERS]=J;
	COMPILER_JUMP_NUMBERS++;
	return true;
}

bool m_compile_register_jump_point_label(char* label, mercury_int label_size, mercury_int position) {
	for (mercury_int i = 0; i < COMPILER_JUMP_NUMBERS; i++) {
		JUMP_POINT* JP = COMPILER_JUMP_DATABASE[i];
		if (JP->label_size == label_size) {
			for (mercury_int c = 0; c < label_size; c++) {
				if (label[c] != JP->label[c])goto next;
			}
			JP->label_point = position;
			JP->defined = true;
			return true;
		}
	next:
		{}
	}
	void* npt = realloc(COMPILER_JUMP_DATABASE, sizeof(JUMP_POINT*) * (COMPILER_JUMP_NUMBERS + 1));
	if (!npt)return false;
	COMPILER_JUMP_DATABASE = (JUMP_POINT**)npt;

	JUMP_POINT* J = (JUMP_POINT*)malloc(sizeof(JUMP_POINT));
	if (!J)return false;

	J->defined = true;
	J->label = (char*)malloc(sizeof(char) * label_size);
	if (!J->label)return false;
	memcpy(J->label, label, label_size * sizeof(char));
	J->label_point = position;
	J->label_size = label_size;
	J->num_positions = 0;
	J->positions = nullptr;

	COMPILER_JUMP_DATABASE[COMPILER_JUMP_NUMBERS] = J;
	COMPILER_JUMP_NUMBERS++;
	return true;
}





enum comment_type:unsigned char {
	COMMENT_NONE=0,
	COMMENT_LINE=1,
	COMMENT_MULTI=2,
};

enum token_flags {
	TOKEN_OPERATOR = 1 << 0,
	TOKEN_UANRY = 1 << 1,
	TOKEN_BINARY = 1 << 2,
	TOKEN_TAKESTATICNUM = 1 << 3,
	TOKEN_STATICSTRING = 1 << 4,
	TOKEN_STATICNUMBER = 1 << 5,
	TOKEN_VARIABLE = 1 << 6,
	TOKEN_ENVVARNAME = 1 << 7,
	TOKEN_KEYWORD = 1 << 8,
	TOKEN_STATICBOOLEAN = 1 << 9,
	TOKEN_STATICNIL = 1 << 10,
	TOKEN_SCOPESPECIFIER = 1 << 11,
	TOKEN_SPECIALVARIABLECREATE = 1<<12,
	TOKEN_LOOP_MODIFIER = 1<<13,
	TOKEN_JUMP = 1<<14,
	TOKEN_EXIT = 1<<15,
	TOKEN_SELFMODIFY = 1<<16,
};


compiler_function* init_comp_func() {
	compiler_function* out=(compiler_function*)malloc(sizeof(compiler_function));
	if (!out)return nullptr;
	out->errorcode = 0;
	out->instructions = nullptr;
	out->number_instructions = 0;
	out->token_error_num = 0;
	out->instruction_tokens = nullptr;

	return out;
}



//forward refrence this so that indexing and calls can use it.
int m_compile_read_var_statment_recur(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max, compiler_function* operation_append=nullptr);
int m_compile_read_variable(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max);

compiler_function* mercury_compile_compile_tokens(compiler_token** tokens, mercury_int num_tokens, mercury_int* endatend = nullptr);


void delete_comp_func(compiler_function* func) {
	if (!func)return;
	if (func->instructions)free(func->instructions);
	if (func->instruction_tokens)free(func->instruction_tokens);
	free(func);
}

void concat_comp_func_appends(compiler_function* fb , compiler_function* fa) { // appends fa to base function fb
	compiler_function* out = init_comp_func();
	if (!out)return;
	uint32_t* np=(uint32_t*)realloc( fb->instructions ,sizeof(uint32_t) * (fb->number_instructions + fa->number_instructions));
	if (!np)return;
	fb->instructions = np;
	memcpy(fb->instructions + fb->number_instructions, fa->instructions, fa->number_instructions * sizeof(uint32_t));

	mercury_int* np2 = (mercury_int*)realloc(fb->instruction_tokens, sizeof(mercury_int) * (fb->number_instructions + fa->number_instructions));
	if (!np2)return;
	fb->instruction_tokens = np2;
	memcpy(fb->instruction_tokens + fb->number_instructions, fa->instruction_tokens, fa->number_instructions * sizeof(mercury_int));
	fb->number_instructions += fa->number_instructions;
}



inline bool char_is_character(char c) {
	return (c>=0x41 && c<=0x5A) || (c>=0x61 && c<=0x7A) || c=='_';
}
inline bool char_is_number(char c) {
	return (c >= 0x30 && c <= 0x39);
}
inline bool char_is_symbol(char c) {
	return ((c>=0x21 && c<=0x2F) || (c>= 0x3C && c<= 0x40) || (c>= 0x5B && c<= 0x60) || (c>= 0x7B && c<= 0x7E) || c==0x3A);
}
inline bool char_is_whitespace(char c) {
	return (!(char_is_symbol(c) || char_is_number(c) || char_is_character(c)));
}

bool add_char_to_token(compiler_token* t, char c) {
	t->num_chars++;
	void* naddr=realloc(t->chars,sizeof(char)*t->num_chars);
	if (!naddr) {
		return false;
	}
	t->chars = (char*)naddr;
	t->chars[t->num_chars - 1] = c;
	return true;
}

int read_char_from_hex_chars(char* chars,int* offset_out) { //this isn't pretty
	int num = 0;
	char c1 = chars[1];
	char c2 = chars[2];
	*offset_out = 0;
	switch (c1) {
	case '0':
		num |= 0x0;
		break;
	case '1':
		num |= 0x1;
		break;
	case '2':
		num |= 0x2;
		break;
	case '3':
		num |= 0x3;
		break;
	case '4':
		num |= 0x4;
		break;
	case '5':
		num |= 0x5;
		break;
	case '6':
		num |= 0x6;
		break;
	case '7':
		num |= 0x7;
		break;
	case '8':
		num |= 0x8;
		break;
	case '9':
		num |= 0x9;
		break;
	case 'A':
	case 'a':
		num |= 0xA;
		break;
	case 'B':
	case 'b':
		num |= 0xB;
		break;
	case 'C':
	case 'c':
		num |= 0xC;
		break;
	case 'D':
	case 'd':
		num |= 0xD;
		break;
	case 'E':
	case 'e':
		num |= 0xE;
		break;
	case 'F':
	case 'f':
		num |= 0xF;
		break;
	default:
		return 0xBEEF;
	}
	num <<= 4;
	*offset_out = 1;
	switch (c2) {
	case '0':
		num |= 0x0;
		break;
	case '1':
		num |= 0x1;
		break;
	case '2':
		num |= 0x2;
		break;
	case '3':
		num |= 0x3;
		break;
	case '4':
		num |= 0x4;
		break;
	case '5':
		num |= 0x5;
		break;
	case '6':
		num |= 0x6;
		break;
	case '7':
		num |= 0x7;
		break;
	case '8':
		num |= 0x8;
		break;
	case '9':
		num |= 0x9;
		break;
	case 'A':
	case 'a':
		num |= 0xA;
		break;
	case 'B':
	case 'b':
		num |= 0xB;
		break;
	case 'C':
	case 'c':
		num |= 0xC;
		break;
	case 'D':
	case 'd':
		num |= 0xD;
		break;
	case 'E':
	case 'e':
		num |= 0xE;
		break;
	case 'F':
	case 'f':
		num |= 0xF;
		break;
	default:
		return num>>4;
	}
	*offset_out = 2;
	return num;
}


int read_char_from_dec_chars(char* chars,int* offset_out) {
	int num = 0;
	char c1 = chars[1];
	char c2 = chars[2];
	char c3 = chars[3];

	printf("%c %c %c\n", c1, c2, c3);

	if (c1 < '0' || c1>'9') {
		*offset_out = 0;
		return 0xBEEF;
	}
	num += (c1 - '0');
	if (c2 < '0' || c2>'9') {
		*offset_out = 1;
		return num;
	}
	num *= 10;
	num += (c2 - '0');
	if (c3 < '0' || c3>'9') {
		*offset_out = 2;
		return num;
	}
	num *= 10;
	num += (c3 - '0');
	*offset_out = 3;
	return num;
}


bool symbols_can_join(char c1, char c2) {
	switch (c1) {
		case '=':	// ==
		case '|':	// || |= ||=
		case '&':	// && &= &&=
		case '~':	// ~~ ~= ~~=
		case '.':	// .. ..=
		case '!':	// !! !=
		case '>':	// >= >> >>=
		case '<':	// <= << <<=
			return c2 == c1 || c2 == '=';
		case '+':	// +=
		case '-':	// -=
		case '*':	// *=
		case '/':	// /=
		case '\\':	// \=
		case '^':	// ^=
		case '%':	// %=
			return c2 == '=';
		default:
			return false;
	}	
}



compiler_token** mercury_compile_tokenize_mstring(mercury_stringliteral* str) {
	compiler_token** out = (compiler_token**)malloc(sizeof(compiler_token*) * 1);
	mercury_int num_tokens = 0;
	compiler_token* temp_token = (compiler_token*)malloc(sizeof(compiler_token));;
	temp_token->chars = nullptr;
	temp_token->num_chars = 0;
	temp_token->line_col = 0;
	temp_token->line_num = 0;
	temp_token->token_flags = 0;


	mercury_int strsize = str->size;
	char* cstr = str->ptr;

	mercury_int col = 0;
	mercury_int line = 0;
	bool readstring = false;
	unsigned char commentmode = 0;



	for (mercury_int c = 0; c < strsize; c++) {
		char c_char = cstr[c];
		char p_char = c==0 ? '\0' : cstr[c - 1];
		char n_char = c >= strsize - 1 ? '\0' : cstr[c + 1];
		col++;

		if (!commentmode) {
			if (readstring) {
				if (c_char == '\"') {
					readstring = false;
					temp_token->token_flags = TOKEN_STATICSTRING | TOKEN_VARIABLE;
					out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
					out[num_tokens] = temp_token;
					num_tokens++;
					temp_token = (compiler_token*)malloc(sizeof(compiler_token));
					temp_token->chars = nullptr;
					temp_token->num_chars = 0;
					temp_token->line_col = col;
					temp_token->line_num = line;
					temp_token->token_flags = 0;
				}
				else {

					if (c_char == '\\') {
						switch (n_char)
						{
						case 'n': //newline
							add_char_to_token(temp_token, '\n');
							break;
						case 'r': //carriage return
							add_char_to_token(temp_token, '\r');
							break;
						case '\\': // backslash
							add_char_to_token(temp_token, '\\');
							break;
						case '\"': //quote
							add_char_to_token(temp_token, '\"');
							break;
						case '\'': //single quote
							add_char_to_token(temp_token, '\'');
							break;
						case '\?': //question
							add_char_to_token(temp_token, '?');
							break;
						case '\a': //bell
							add_char_to_token(temp_token, '\a');
							break;
						case '\b': //backspace
							add_char_to_token(temp_token, '\b');
							break;
						case '\e': //escape
							add_char_to_token(temp_token, '\e');
							break;
						case '\f': //formfeed
							add_char_to_token(temp_token, '\f');
							break;
						case '\t': //tab
							add_char_to_token(temp_token, '\t');
							break;
						case '\v': //vertical tab
							add_char_to_token(temp_token, '\v');
							break;
						case 'x': //hex input
						{
							int off = 0;
							int  r = read_char_from_hex_chars(cstr + c + 1,&off);
							if (r != 0xBEEF) { //magic numbers, yay
								add_char_to_token(temp_token, (char)r);
							}
							c += off;
							col += off;
						}
						break;
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						{
							int adv = 0;
							int  r = read_char_from_dec_chars(cstr + c ,&adv);
							if (r != 0xBEEF) { //magic numbers, yay
								add_char_to_token(temp_token, (char)r);
							}
							adv -= 1;
							c += adv;
							col += adv;
						}
							break;
						default:
							break;
						}
						c++;
						col++;
					}
					else {
						add_char_to_token(temp_token, c_char);
					}


				}
			}
			else {

				if (c_char == '\"') {
					readstring = true;
				}
				else if (c_char=='/' && (n_char == '/' || n_char == '*') ) {
					if (n_char == '/') {
						commentmode = COMMENT_LINE;
					}
					else if (n_char == '*') {
						commentmode = COMMENT_MULTI;
					}
				}
				else if (char_is_whitespace(c_char) && temp_token->num_chars) {
					out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
					out[num_tokens] = temp_token;
					num_tokens++;
					temp_token = (compiler_token*)malloc(sizeof(compiler_token));
					temp_token->chars = nullptr;
					temp_token->num_chars = 0;
					temp_token->line_col = col;
					temp_token->line_num = line;
					temp_token->token_flags = 0;
				}
				else if ((char_is_number(c_char))) {
					add_char_to_token(temp_token, c_char);
					bool allow_nonnumber = !(temp_token->token_flags & TOKEN_ENVVARNAME);
					if(!temp_token->token_flags)temp_token->token_flags = TOKEN_STATICNUMBER | TOKEN_VARIABLE;
					if (!char_is_number(n_char) && (!allow_nonnumber || !char_is_character(n_char)) ) {
						out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
						out[num_tokens] = temp_token;
						num_tokens++;
						temp_token = (compiler_token*)malloc(sizeof(compiler_token));
						temp_token->chars = nullptr;
						temp_token->num_chars = 0;
						temp_token->line_col = col;
						temp_token->line_num = line;
						temp_token->token_flags = 0;
					}
				}
				else if (char_is_character(c_char)) {
					add_char_to_token(temp_token, c_char);
					temp_token->token_flags = TOKEN_ENVVARNAME | TOKEN_VARIABLE;
					if (!(char_is_character(n_char) ||char_is_number(n_char) )) {
						out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
						out[num_tokens] = temp_token;
						num_tokens++;
						temp_token = (compiler_token*)malloc(sizeof(compiler_token));
						temp_token->chars = nullptr;
						temp_token->num_chars = 0;
						temp_token->line_col = col;
						temp_token->line_num = line;
						temp_token->token_flags = 0;
					}
				}
				else if (char_is_symbol(c_char)) {
					add_char_to_token(temp_token, c_char);
					temp_token->token_flags = TOKEN_OPERATOR;
					if (!char_is_symbol(n_char)) {
						out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
						out[num_tokens] = temp_token;
						num_tokens++;
						temp_token = (compiler_token*)malloc(sizeof(compiler_token));
						temp_token->chars = nullptr;
						temp_token->num_chars = 0;
						temp_token->line_col = col;
						temp_token->line_num = line;
						temp_token->token_flags = 0;
					}
					else if (!symbols_can_join(c_char, n_char)) {
						out = (compiler_token**)realloc(out, sizeof(compiler_token*) * (num_tokens + 1));
						out[num_tokens] = temp_token;
						num_tokens++;
						temp_token = (compiler_token*)malloc(sizeof(compiler_token));
						temp_token->chars = nullptr;
						temp_token->num_chars = 0;
						temp_token->line_col = col;
						temp_token->line_num = line;
						temp_token->token_flags = 0;
					}
				}



			}
		}
		else {
			if (commentmode == COMMENT_MULTI) {
				if(c_char == '/' && p_char == '*')commentmode = COMMENT_NONE;
			}
			else if (commentmode == COMMENT_LINE) {
				if (c_char == '*' && p_char == '/')commentmode = COMMENT_MULTI;
			}
		}

		if ((c_char == '\n' && p_char != '\r') || c_char == '\r') {
			col = 0;
			line++;
			commentmode = (commentmode == COMMENT_LINE ? COMMENT_NONE : commentmode);
		}
	}



	for (mercury_int i = 0; i < num_tokens; i++) {
		compiler_token* token = out[i];

		if (token->token_flags & TOKEN_ENVVARNAME) { //extracting keywords from vars
			char* tc = token->chars;
			if (token->num_chars == 2) {
				if (tc[0] == 'i' && tc[1] == 'f') {
					token->token_flags = TOKEN_KEYWORD;
				}
				else if (tc[0] == 'd' && tc[1] == 'o') {
					token->token_flags = TOKEN_KEYWORD;
				}
			}
			else if (token->num_chars == 3) {
				if (tc[0] == 'n' && tc[1] == 'i' && tc[2] == 'l') {
					token->token_flags = TOKEN_STATICNIL | TOKEN_VARIABLE;
				}
				else if (tc[0] == 'e' && tc[1] == 'n' && tc[2] == 'd') {
					token->token_flags = TOKEN_KEYWORD;
				}
				else if (tc[0] == 'n' && tc[1] == 'a' && tc[2] == 'n') { //nan
					token->token_flags = TOKEN_VARIABLE | TOKEN_STATICNUMBER;
				}
				else if (tc[0] == 'i' && tc[1] == 'n' && tc[2] == 'f') { //inf
					token->token_flags = TOKEN_VARIABLE | TOKEN_STATICNUMBER;
				}
			}
			else if (token->num_chars == 4) {
				if (tc[0] == 't' && tc[1] == 'r' && tc[2] == 'u' && tc[3] == 'e') {
					token->token_flags = TOKEN_STATICBOOLEAN | TOKEN_VARIABLE;
				}
				else if (tc[0] == 't' && tc[1] == 'h' && tc[2] == 'e' && tc[3] == 'n') {
					token->token_flags = TOKEN_KEYWORD;
				}
				else if (tc[0] == 'e' && tc[1] == 'l' && tc[2] == 's' && tc[3] == 'e') {
					token->token_flags = TOKEN_KEYWORD;
				}
				else if (tc[0] == 'g' && tc[1] == 'o' && tc[2] == 't' && tc[3] == 'o') {
					token->token_flags = TOKEN_JUMP;
				}
			}
			else if (token->num_chars == 5) {
				if (tc[0] == 'w' && tc[1] == 'h' && tc[2] == 'i' && tc[3] == 'l' && tc[4] == 'e') {
					token->token_flags = TOKEN_KEYWORD;
				}
				if (tc[0] == 'f' && tc[1] == 'a' && tc[2] == 'l' && tc[3] == 's' && tc[4] == 'e') {
					token->token_flags = TOKEN_STATICBOOLEAN | TOKEN_VARIABLE;
				}
				if (tc[0] == 'l' && tc[1] == 'o' && tc[2] == 'c' && tc[3] == 'a' && tc[4] == 'l') {
					token->token_flags = TOKEN_SCOPESPECIFIER;
				}
				if (tc[0] == 'b' && tc[1] == 'r' && tc[2] == 'e' && tc[3] == 'a' && tc[4] == 'k') {
					token->token_flags = TOKEN_LOOP_MODIFIER;
				}
			}
			else if (token->num_chars == 6) {
				if (tc[0] == 'e' && tc[1] == 'l' && tc[2] == 's' && tc[3] == 'e' && tc[4] == 'i' && tc[5] == 'f') {
					token->token_flags = TOKEN_KEYWORD;
				}
				if (tc[0] == 'r' && tc[1] == 'e' && tc[2] == 't' && tc[3] == 'u' && tc[4] == 'r' && tc[5] == 'n') {
					token->token_flags = TOKEN_EXIT;
				}
				if (tc[0] == 'g' && tc[1] == 'l' && tc[2] == 'o' && tc[3] == 'b' && tc[4] == 'a' && tc[5] == 'l') {
					token->token_flags = TOKEN_SCOPESPECIFIER;
				}
			}
			else if (token->num_chars == 8) {
				if (tc[0] == 'c' && tc[1] == 'o' && tc[2] == 'n' && tc[3] == 't' && tc[4] == 'i' && tc[5] == 'n' && tc[6] == 'u' && tc[7] == 'e') {
					token->token_flags = TOKEN_LOOP_MODIFIER;
				} else if (tc[0] == 'f' && tc[1] == 'u' && tc[2] == 'n' && tc[3] == 'c' && tc[4] == 't' && tc[5] == 'i' && tc[6] == 'o' && tc[7] == 'n') {
					token->token_flags = TOKEN_KEYWORD;
				}
			}


		}
		else if (token->token_flags & TOKEN_OPERATOR) {
			char* tc = token->chars;
			if (token->num_chars == 1) {
				switch (tc[0]) {
				case '>':
				case '<':
				case '+':
				case '*':
				case '/':
				case '^':
				case '%':
				case '&':
				case '|':
				case '~':
				case '\\':
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM;
					break;
				case '#':
				case '!':
					token->token_flags |= TOKEN_UANRY;
					break;
				case '-':
					token->token_flags |= TOKEN_BINARY | TOKEN_UANRY | TOKEN_TAKESTATICNUM;
					break;
				case '[':
				case ']':
				case '{':
				case '}':
					token->token_flags |= TOKEN_SPECIALVARIABLECREATE;
					break;
				}
			}
			else if (token->num_chars == 2) {
				if (tc[0] == '!' && tc[1] == '!') {
					token->token_flags |= TOKEN_UANRY;
				}
				else if (tc[0] == '.' && tc[1] == '.') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '&' && tc[1] == '&') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '|' && tc[1] == '|') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '~' && tc[1] == '~') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '>' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM;
				}
				else if (tc[0] == '<' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM;
				}
				else if (tc[0] == '>' && tc[1] == '>') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM;
				}
				else if (tc[0] == '<' && tc[1] == '<') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM;
				}
				else if (tc[0] == '=' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '!' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY;
				}
				else if (tc[0] == '+' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '-' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '*' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '/' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '\\' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '^' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '%' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '&' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '|' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '~' && tc[1] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
			}
			else if (token->num_chars == 3) {
				if (tc[0] == '.' && tc[1] == '.' && tc[2] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_SELFMODIFY;
				} else if (tc[0] == '<' && tc[1] == '<' && tc[2] == '=') {
						token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}else if (tc[0] == '>' && tc[1] == '>' && tc[2] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_TAKESTATICNUM | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '|' && tc[1] == '|' && tc[2] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '&' && tc[1] == '&' && tc[2] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_SELFMODIFY;
				}
				else if (tc[0] == '~' && tc[1] == '~' && tc[2] == '=') {
					token->token_flags |= TOKEN_BINARY | TOKEN_SELFMODIFY;
				}
				//TOKEN_SELFMODIFY
			}

		}

	}

	void* nptr = realloc(out, sizeof(compiler_token*)*(num_tokens + 1) );
	if (!nptr){
		free(out);
		return nullptr;
	}
	out = (compiler_token**)nptr;
	out[num_tokens] = nullptr;
	return out;
}



uint16_t m_compile_get_operator_bytecode(compiler_token* token,int forcetype) {
	if (token->num_chars == 1) {
		char c1 = token->chars[0];
		switch (c1) {
		case '>':
			return M_OPCODE_GRT;
		case '<':
			return M_OPCODE_LET;
		case '+':
			return M_OPCODE_ADD;
		case '-':
			return M_OPCODE_SUB;
		case '*':
			return M_OPCODE_MUL;
		case '/':
			return M_OPCODE_DIV;
		case '\\':
			return M_OPCODE_IDIV;
		case '%':
			return M_OPCODE_MOD;
		case '^':
			return M_OPCODE_POW;
		case '|':
			return M_OPCODE_BOR;
		case '&':
			return M_OPCODE_BAND;
		case '~':
			return M_OPCODE_BXOR;
		case '!':
			return M_OPCODE_BNOT;
		case '#':
			return M_OPCODE_LEN;
		}
	}
	else if (token->num_chars == 2) {
		char c1 = token->chars[0];
		char c2 = token->chars[1];
		if (c1=='=' && c2=='=') {
			return M_OPCODE_EQL;
		}else if (c1 == '!' && c2 == '=') {
			return M_OPCODE_NEQ;
		}else if (c1 == '>' && c2 == '=') {
			return M_OPCODE_GTE;
		}
		else if (c1 == '<' && c2 == '=') {
			return M_OPCODE_LTE;
		}
		else if (c1 == '<' && c2 == '<') {
			return M_OPCODE_BSHL;
		}
		else if (c1 == '>' && c2 == '>') {
			return M_OPCODE_BSHR;
		}
		else if (c1 == '!' && c2 == '!') {
			return M_OPCODE_LNOT;
		}
		else if (c1 == '|' && c2 == '|') {
			return M_OPCODE_LOR;
		}
		else if (c1 == '&' && c2 == '&') {
			return M_OPCODE_LAND;
		}
		else if (c1 == '~' && c2 == '~') {
			return M_OPCODE_LXOR;
		}
		else if (c1 == '.' && c2 == '.') {
			return M_OPCODE_CNCT;
		}
		else if (c1 == '|' && c2 == '=') {
			return M_OPCODE_BOR;
		}
		else if (c1 == '&' && c2 == '=') {
			return M_OPCODE_BAND;
		}
		else if (c1 == '~' && c2 == '=') {
			return M_OPCODE_BXOR;
		}
		else if (c1 == '+' && c2 == '=') {
			return M_OPCODE_ADD;
		}
		else if (c1 == '-' && c2 == '=') {
			return M_OPCODE_SUB;
		}
		else if (c1 == '*' && c2 == '=') {
			return M_OPCODE_MUL;
		}
		else if (c1 == '/' && c2 == '=') {
			return M_OPCODE_DIV;
		}
		else if (c1 == '\\' && c2 == '=') {
			return M_OPCODE_IDIV;
		}
		else if (c1 == '^' && c2 == '=') {
			return M_OPCODE_POW;
		}
		else if (c1 == '%' && c2 == '=') {
			return M_OPCODE_MOD;
		}
	}
	else if (token->num_chars == 3) {
		char c1 = token->chars[0];
		char c2 = token->chars[1];
		char c3 = token->chars[2];
		if (c1 == '&' && c2 == '&' && c2 == '=') {
			return M_OPCODE_LAND;
		}
		else if (c1 == '|' && c2 == '|' && c2 == '=') {
			return M_OPCODE_LOR;
		}
		else if (c1 == '~' && c2 == '~' && c2 == '=') {
			return M_OPCODE_LXOR;
		}
		else if (c1 == '<' && c2 == '<' && c2 == '=') {
			return M_OPCODE_BSHL;
		}
		else if (c1 == '>' && c2 == '>' && c2 == '=') {
			return M_OPCODE_BSHR;
		}
		else if (c1 == '.' && c2 == '.' && c2 == '=') {
			return M_OPCODE_CNCT;
		}
	}

	return M_OPCODE_NOP;
}


void m_compile_add_instruction(compiler_function* func , uint16_t opcode ,uint16_t flags=0,mercury_int tokennum=0) {
	//printf("adding instruction %i [%i] to function %p as number %i from token %i\n",opcode,flags,func,func->number_instructions+1,tokennum);
	uint32_t* naddr= (uint32_t*)realloc(func->instructions, sizeof(uint32_t) * (func->number_instructions+1) );
	if (!naddr) {
		return;
	}
	func->instructions = naddr;
	uint32_t inst = 0;
	inst |= flags;
	inst <<= 16;
	inst |= opcode;
	func->instructions[func->number_instructions] = inst;

	mercury_int* naddr2 =(mercury_int*)realloc(func->instruction_tokens, sizeof(mercury_int) * (func->number_instructions + 1));
	if (!naddr2) {
		return;
	}
	func->instruction_tokens = naddr2;
	func->instruction_tokens[func->number_instructions]= tokennum;

	func->number_instructions++;

	
	return;
}
void m_compile_add_rawdata(compiler_function* func, uint32_t dat, mercury_int tokennum = 0) {
	uint32_t* naddr = (uint32_t*)realloc(func->instructions, sizeof(uint32_t) * (func->number_instructions + 1));
	if (!naddr) {
		return;
	}
	func->instructions = naddr;
	func->instructions[func->number_instructions] = dat;

	mercury_int* naddr2 = (mercury_int*)realloc(func->instruction_tokens, sizeof(mercury_int) * (func->number_instructions + 1));
	if (!naddr2) {
		return;
	}
	func->instruction_tokens = naddr2;
	func->instruction_tokens[func->number_instructions] = tokennum;

	func->number_instructions++;
	return;
}
void m_compile_add_rawdatadouble(compiler_function* func, uint64_t dat, mercury_int tokennum = 0) {
	uint32_t* naddr = (uint32_t*)realloc(func->instructions, sizeof(uint32_t) * (func->number_instructions + 2));
	if (!naddr) {
		return;
	}
	func->instructions = naddr;
	uint64_t* ad = (uint64_t*)(func->instructions + func->number_instructions);
	*ad = dat;

	mercury_int* naddr2 = (mercury_int*)realloc(func->instruction_tokens, sizeof(mercury_int) * (func->number_instructions + 2));
	if (!naddr2) {
		return;
	}
	func->instruction_tokens = naddr2;
	func->instruction_tokens[func->number_instructions] = tokennum;
	func->instruction_tokens[func->number_instructions+1] = tokennum;

	func->number_instructions +=2;
	return;
}



void m_compile_cstring_load(compiler_function* func, char* str, mercury_int len=-1, mercury_int position=0) {
	len = len < 0 ? strlen(str) : len;

	m_compile_add_instruction(func,M_OPCODE_NSTR,0);
	#ifdef MERCURY_64BIT
	m_compile_add_rawdatadouble(func,len);
	#else
	m_compile_add_rawdata(func,len);
	#endif


	mercury_int sizetoallocate = (len + 3) / 4; // 4 chars in 32 bits.
	uint32_t* naddr = (uint32_t*)realloc(func->instructions, sizeof(uint32_t) * (func->number_instructions + sizetoallocate ));
	if (!naddr) {
		return;
	}
	func->instructions = naddr;
	char* ad = (char*)(func->instructions + func->number_instructions);
	memcpy(ad,str,len);

	mercury_int* naddr2 = (mercury_int*)realloc(func->instruction_tokens, sizeof(mercury_int) * (func->number_instructions + sizetoallocate));
	if (!naddr2) {
		return;
	}
	func->instruction_tokens = naddr2;
	for (mercury_int i = 0; i < sizetoallocate; i++) {
		func->instruction_tokens[func->number_instructions + i] = position; //PLACEHOLDER
	}

	func->number_instructions += sizetoallocate;
}

void m_compile_number_string_load(compiler_function* func, char* str, mercury_int len = -1, mercury_int position = 0) {
	len = len < 0 ? strlen(str) : len;
	char* cstr = (char*)malloc(len + 1);
	memcpy(cstr, str,len);
	cstr[len] = '\0';
	char* end;



		mercury_int i = strtoll(cstr, &end, 0);
		if (*end=='\0') {
		m_compile_add_instruction(func, M_OPCODE_NINT,0, position);
#ifdef MERCURY_64BIT
		m_compile_add_rawdatadouble(func, i, position);
#else
		m_compile_add_rawdata(func, i, position);
#endif
			return;
	}




	mercury_float f = strtod(cstr, &end);
		m_compile_add_instruction(func, M_OPCODE_NFLO,0, position);
		mercury_int d = *((mercury_int*)&f);
#ifdef MERCURY_64BIT
		m_compile_add_rawdatadouble(func, d, position);
#else
		m_compile_add_rawdata(func, d, position);
#endif
			return;



}

//for tables an arrays. supports commas, as well as bracket notation. eg, [1,2,3] [1,2,[5]=1]
int m_compile_read_stored_variable(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	if (offset > token_max) {
		return 0;
	}
	compiler_token* t = tokens[offset];

	//m_compile_add_instruction(func, M_OPCODE_CPYT); //copy the structure so that it remains on the stack still.
	//remember that the stack order is table -> key -> value.

	if (t->token_flags & TOKEN_SPECIALVARIABLECREATE && t->chars[0] == '[') { //either create a new array, or index. we need to find out which.
		mercury_int o = 1;
		m_compile_add_instruction(func, M_OPCODE_CPYT, 0,offset+o);
		mercury_int a = m_compile_read_var_statment_recur(func, tokens, offset + o, token_max);
		if (!a)return 0;
		o += a;
		t = tokens[offset + o];
		if ( t->token_flags & TOKEN_SPECIALVARIABLECREATE && t->chars[0] == ']' ) {
			o++;
			t = tokens[offset + o];
			if (t->token_flags & TOKEN_OPERATOR && t->num_chars==1 && t->chars[0] == '=') { // it is an indexed setting
				o++;
				a= m_compile_read_var_statment_recur(func, tokens, offset + o, token_max);
				if (a)m_compile_add_instruction(func, M_OPCODE_SET, 0, offset + o);
				return o + a;
			}
			else {
				func->errorcode = M_COMPERR_INVALID_SYMBOL;
				func->token_error_num = offset + o;
				return 0;
			}
		}
		else {
			func->errorcode = M_COMPERR_INVALID_SYMBOL;
			func->token_error_num = offset + o;
			return 0;
		} //nevermind, it's actually just a new array.

		m_compile_add_instruction(func, M_OPCODE_NINT,0, offset + o);
#ifdef MERCURY_64BIT
		m_compile_add_rawdatadouble(func, POSITION_IN_DATASTRUCTURE, offset + o);
#else
		m_compile_add_rawdata(func, POSITION_IN_DATASTRUCTURE, offset + o);
#endif
		POSITION_IN_DATASTRUCTURE++;
		a=m_compile_read_variable(func, tokens, offset, token_max);
		if (a)m_compile_add_instruction(func, M_OPCODE_SET, 0, offset + a);
		if(a)return a;
	}
	else {
		compiler_function* f2=init_comp_func();
		if (!f2)return 0;

		m_compile_add_instruction(f2, M_OPCODE_CPYT, 0, offset);
		m_compile_add_instruction(f2, M_OPCODE_NINT, 0, offset);
#ifdef MERCURY_64BIT
		m_compile_add_rawdatadouble(f2, POSITION_IN_DATASTRUCTURE);
#else
		m_compile_add_rawdata(func, POSITION_IN_DATASTRUCTURE);
#endif
		POSITION_IN_DATASTRUCTURE++;
		mercury_int a=m_compile_read_var_statment_recur(f2, tokens, offset, token_max);
		if (a) { 
			m_compile_add_instruction(f2, M_OPCODE_SET, 0, offset+a);
			concat_comp_func_appends(func,f2);
			delete_comp_func(f2);
		}
		return a;
	}


	return 0;
}

int mercury_compile_read_function_define(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max, bool requiresname) {
	mercury_int adv = 0;
	compiler_token* token = tokens[offset + adv];

	if (!(token->token_flags & TOKEN_KEYWORD && token->num_chars == 8 && token->chars[0] == 'f' && token->chars[7] == 'n')) { //function keyword
		return 0;
	}
	adv++;
	token = tokens[offset + adv];

	compiler_function* ffunc = init_comp_func();

	if (!ffunc) {
		func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
		func->token_error_num = offset + adv;
		return 0;
	}

	compiler_function* fvarget = init_comp_func();
	if (!fvarget) {
		func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
		func->token_error_num = offset + adv;
		return 0;
	}


	/*
	steps to do this in bytecode:
		define function
		commit to variable
	of course, we actually do the reverse in code writting...


	now, a function can be broken down into pieces

	function stuff(var1,var2) return var1+var2 end
	declaration, name, variables, body

	*/



	if (requiresname) {
		if (token->token_flags & TOKEN_ENVVARNAME) {
			m_compile_cstring_load(ffunc,token->chars,token->num_chars,offset+adv);
			adv++;
			token = tokens[offset + adv];
		}
		else {
			func->errorcode = M_COMPERR_EXPECTED_VARIABLE;
			func->token_error_num = offset + adv;
			return 0;
		}
	}


	if (!(token->token_flags & TOKEN_OPERATOR && token->num_chars == 1 && token->chars[0] == '(')) {
		func->errorcode = M_COMPERR_INVALID_SYMBOL;
		func->token_error_num = offset+ adv;
		return 0;
	}
	adv++;

	while (true) { //gotta search for vars.
		token = tokens[offset + adv];
		if (token->token_flags & TOKEN_OPERATOR) { //check for )
			if (token->chars[0] == ')' && token->num_chars == 1) {
				adv++;
				break;
			}
			else {
				func->errorcode = M_COMPERR_INVALID_SYMBOL;
				func->token_error_num = offset + adv;
				return 0;
			}
		}
		else if (token->token_flags & TOKEN_ENVVARNAME) { //set var from stack
			// fvarget

			m_compile_cstring_load(fvarget, token->chars, token->num_chars, offset + adv); //first we need to get the string
			m_compile_add_instruction(fvarget, M_OPCODE_SWPT, 0,offset+adv); // but, the key must be second on the stack
			m_compile_add_instruction(fvarget, M_OPCODE_SETL, 0, offset + adv); //now set it as a local.
			adv++;
			token = tokens[offset + adv];
			if (token->token_flags & TOKEN_OPERATOR) {
				if (token->chars[0] == ')' && token->num_chars == 1) {
					adv++;
					break;
				}
				else if (token->chars[0] == ',' && token->num_chars == 1) {
					adv++;
				}
				else {
					func->errorcode = M_COMPERR_INVALID_SYMBOL;
					func->token_error_num = offset + adv;
					return 0;
				}
			}
			else {
				func->errorcode = M_COMPERR_INVALID_SYMBOL;
				func->token_error_num = offset + adv;
				return 0;
			}
		}
		else { //error!
			func->errorcode = M_COMPERR_INVALID_SYMBOL;
			func->token_error_num = offset + adv;
			return 0;
		}
	}
	m_compile_add_instruction(fvarget, M_OPCODE_CLS, 0, offset + adv); // clear the stack after assigning variables so that if more variables are given than we need, it won't result in bad return values


	// compiler_function* mercury_compile_compile_tokens(compiler_token** tokens, mercury_int num_tokens , mercury_int* endatend=nullptr )
	mercury_int functokensused = 0;
	compiler_function* fnfunc = mercury_compile_compile_tokens(tokens+offset+adv, token_max-adv-offset, &functokensused); //shift the tokens to match it.

	adv += functokensused;

	adv++;

	concat_comp_func_appends(fvarget, fnfunc);
	delete_comp_func(fnfunc);


	m_compile_add_instruction(ffunc, M_OPCODE_NFUN, 0, offset + adv);
#ifdef MERCURY_64BIT
	m_compile_add_rawdatadouble(ffunc, fvarget->number_instructions);
#else
	m_compile_add_rawdata(ffunc, fvarget->number_instructions);
#endif


	if (requiresname) {
		m_compile_add_instruction(fvarget, M_OPCODE_SENV, 0, offset + adv);
	}

	concat_comp_func_appends(func, ffunc);
	concat_comp_func_appends(func, fvarget);
	delete_comp_func(fvarget);
	delete_comp_func(ffunc);

	return adv;
}



int m_compile_read_variable(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {

	compiler_token* t=tokens[offset];
	if (t->token_flags & TOKEN_SCOPESPECIFIER) {
		compiler_token* t2 = tokens[offset + 1];
		if (t2->token_flags & TOKEN_ENVVARNAME) {
			m_compile_cstring_load(func, t2->chars, t2->num_chars, offset);
			if (t->chars[0] == 'l') { //local
				m_compile_add_instruction(func, M_OPCODE_GETL, 0,offset);
			}
			else if (t->chars[0] == 'g') { //global
				m_compile_add_instruction(func, M_OPCODE_GETG, 0, offset);
			}
			return 2;
		}
		else {
			return 0;
		}
	}
	else if (t->token_flags & TOKEN_ENVVARNAME) {
		m_compile_cstring_load(func,t->chars, t->num_chars, offset);
		m_compile_add_instruction(func, M_OPCODE_GENV, 0, offset);
		return 1;
	}
	else if (t->token_flags & TOKEN_STATICSTRING) {
		m_compile_cstring_load(func, t->chars, t->num_chars, offset);
		return 1;
	}
	else if (t->token_flags & TOKEN_STATICNIL) {
		m_compile_add_instruction(func,M_OPCODE_NNIL, 0, offset);
		return 1;
	}
	else if (t->token_flags & TOKEN_STATICBOOLEAN) {
		if (t->chars[0] == 't') {
			m_compile_add_instruction(func, M_OPCODE_NTRU, 0, offset);
		}
		else {
			m_compile_add_instruction(func, M_OPCODE_NFAL, 0, offset);
		}
		return 1;
	}
	else if (t->token_flags & TOKEN_STATICNUMBER) { //numbers are complex, since we have a lot of formats. eg, with + or - at the front, 0x for hex, and floats with decimals.
		if (offset + 1 < token_max) {
			compiler_token* t2 = tokens[offset + 1];
			if (t2->token_flags & TOKEN_OPERATOR && t2->num_chars == 1 && t2->chars[0] == '.') { //decimal

				if (offset + 2 < token_max) {
					compiler_token* t3 = tokens[offset + 2];
					if (t3->token_flags & TOKEN_STATICNUMBER) { //places!
						char* s2 = (char*)malloc(sizeof(char) * (t->num_chars + t2->num_chars + t3->num_chars));
						if (!s2)return 0;
						memcpy(s2, t->chars, t->num_chars);
						memcpy(s2 + t->num_chars, t2->chars, t2->num_chars);
						memcpy(s2 + t->num_chars + t2->num_chars, t3->chars, t3->num_chars);
						m_compile_number_string_load(func, s2, t->num_chars + t2->num_chars + t3->num_chars,offset);
						return 3;
					}
				}
				char* s = (char*)malloc(sizeof(char) * (t->num_chars + t2->num_chars));
				if (!s)return 0;
				memcpy(s, t->chars, t->num_chars);
				memcpy(s + t->num_chars, t2->chars, t2->num_chars);
				m_compile_number_string_load(func, s, t->num_chars + t2->num_chars, offset);
				return 2;
			}
		}
		m_compile_number_string_load(func,t->chars,t->num_chars, offset);
		return 1;
	}
	else if (t->token_flags & TOKEN_SPECIALVARIABLECREATE) { //arrays and tables
		mercury_int o = 0;
		if (t->chars[0] == '[') { // a ray
			o++;
			m_compile_add_instruction(func, M_OPCODE_NARR, 0, offset+o);
			mercury_int prev_datapos = POSITION_IN_DATASTRUCTURE;
			POSITION_IN_DATASTRUCTURE = 0;
			while (true) {

				o += m_compile_read_stored_variable(func, tokens, offset + o, token_max);

				t = tokens[offset + o];
				if (t->token_flags & TOKEN_SPECIALVARIABLECREATE && t->chars[0]==']') {
					return o + 1;
				}
				else if (t->token_flags & TOKEN_OPERATOR && t->chars[0] == ',') {
					o++;
				}
				else {
					func->token_error_num = offset + o;
					func->errorcode = M_COMPERR_INVALID_SYMBOL;
					return 0;
				}
				
			}
			POSITION_IN_DATASTRUCTURE = prev_datapos;
			return o;
		}
		else if (t->chars[0] == '{'){// ta ble
			//printf("reading table\n");
			o++;
			m_compile_add_instruction(func, M_OPCODE_NTAB, 0,offset + o);
			mercury_int prev_datapos = POSITION_IN_DATASTRUCTURE;
			POSITION_IN_DATASTRUCTURE = 0;
			while (true) {

				o += m_compile_read_stored_variable(func, tokens, offset + o, token_max);

				t = tokens[offset + o];
				if (t->token_flags & TOKEN_SPECIALVARIABLECREATE && t->chars[0] == '}') {
					//printf("ret'd %i\n",o);
					return o + 1;
				}
				else if (t->token_flags & TOKEN_OPERATOR && t->chars[0] == ',') {
					o++;
				}
				else {
					//printf("aw fuck\n");
					func->token_error_num = offset + o;
					func->errorcode = M_COMPERR_INVALID_SYMBOL;
					return 0;
				}

			}
			POSITION_IN_DATASTRUCTURE = prev_datapos;
			return o;
		}
		else {
			return 0;
		}
	}
	else if (t->token_flags & TOKEN_OPERATOR) {
	if (t->num_chars == 1 && t->chars[0] == '.') {
		compiler_token* t2 = tokens[offset + 1];
		if (t2->token_flags & TOKEN_STATICNUMBER) {
			char* s = (char*)malloc(sizeof(char) * (t->num_chars + t2->num_chars));
			if (!s)return 0;
			memcpy(s, t->chars, t->num_chars);
			memcpy(s + t->num_chars, t2->chars, t2->num_chars);
			m_compile_number_string_load(func, s, t->num_chars + t2->num_chars,offset);
			return 2;
		}
	}
	return 0;
	}
	else if (t->token_flags & TOKEN_KEYWORD) {
		if (t->num_chars == 8 && t->chars[0] == 'f' && t->chars[7] == 'n') { //read an anonymous function
			return mercury_compile_read_function_define(func,tokens,offset,token_max,false);
		}
	}
	return 0;
}


int m_compile_read_unary_op(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	if (offset >= token_max)return 0;
	compiler_token* t = tokens[offset];
	if ( (t->token_flags & TOKEN_OPERATOR) && (t->token_flags & TOKEN_UANRY) ) {
		uint16_t op = m_compile_get_operator_bytecode(t, 0);
		m_compile_add_instruction(func, op, 0, offset);
		return 1;
	}
	return 0;
}

int m_compile_read_binary_op(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	if (offset >= token_max)return 0;
	compiler_token* t = tokens[offset];
	if ( (t->token_flags & TOKEN_OPERATOR ) && (t->token_flags & TOKEN_BINARY) ) {
		uint16_t op = m_compile_get_operator_bytecode(t, 0);
		m_compile_add_instruction(func, op, 0,offset);
		return 1;
	}
	return 0;
}




int m_compile_read_indexing(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	if (offset >= token_max)return 0;
	int adv = 0;
	bool periodmode = false;

	if (!(tokens[offset]->num_chars == 1)) {
		return 0;
	}
	if (tokens[offset]->chars[0] == '[')
	{

	}else if (tokens[offset]->chars[0] == '.') {
		periodmode = true;
	}else {
		return 0;
	}
	adv++;
	offset++;

	if (offset >= token_max)return 0;


	int a = 0;
	if (periodmode) {
		compiler_token* t = tokens[offset];
		if (t->token_flags & TOKEN_ENVVARNAME) {
			m_compile_cstring_load(func, t->chars, t->num_chars, offset);
			a = 1;
		}
		else {
			return 0;
		}
	}
	else {
		a = m_compile_read_var_statment_recur(func, tokens, offset, token_max);
	}
	if (!a) {
		func->errorcode = M_COMPERR_EXPECTED_VARIABLE;
		return 0;
	}
	offset += a;
	adv += a;

	if (!periodmode) {
		if (offset >= token_max)return 0;
		if (!(tokens[offset]->num_chars == 1 && tokens[offset]->chars[0] == ']')) {
			return 0;
		}
	}

	if(adv)m_compile_add_instruction(func, M_OPCODE_GET, 0,offset);

	if(!periodmode)adv++;

	return adv;
}

int m_compile_read_fcall(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max, compiler_function* get_var_bytecode) {
	if (offset >= token_max)return 0;
	int advanced = 0;
	mercury_int args_in = 0;
	mercury_int args_out = COMPILER_NUMBER_ARGS_OUT;

	if ( !(tokens[offset]->num_chars == 1 && tokens[offset]->chars[0] == '(') ) {
		return 0;
	}
	offset++;
	advanced++;

	mercury_int PREV_COMP_NUM_ARG_OUT = COMPILER_NUMBER_ARGS_OUT;

	COMPILER_NUMBER_ARGS_OUT = 1;

	while (!(tokens[offset]->num_chars == 1 && tokens[offset]->chars[0] == ')')) {
		if (offset >= token_max) { COMPILER_NUMBER_ARGS_OUT = PREV_COMP_NUM_ARG_OUT;  return 0; }

		int a= m_compile_read_var_statment_recur(func, tokens, offset, token_max);
		offset += a;
		advanced+=a;
		args_in++;
		if (offset > token_max) { COMPILER_NUMBER_ARGS_OUT = PREV_COMP_NUM_ARG_OUT;  return 0; }
		if (!(tokens[offset]->num_chars == 1 && tokens[offset]->chars[0] == ',')) {
			break;
		}
		offset++;
		advanced++;
	}
	advanced++;


	COMPILER_NUMBER_ARGS_OUT = PREV_COMP_NUM_ARG_OUT;

	concat_comp_func_appends(func, get_var_bytecode);
	
	m_compile_add_instruction(func, M_OPCODE_CALL, 0, offset-advanced+1);
#ifdef MERCURY_64BIT
	m_compile_add_rawdatadouble(func, args_in, offset - advanced + 1);
	m_compile_add_rawdatadouble(func, args_out, offset - advanced + 1);
#else
	m_compile_add_rawdata(func, args_in, offset);
	m_compile_add_rawdata(func, args_out, offset);
#endif

	return advanced;
}

int m_compile_read_var_statment_recur(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max, compiler_function* operation_append) {
	if (offset >= token_max) {
		return 0;
	}
	mercury_int addo = 0;
	bool parentheses = false;

	compiler_function* f1 = init_comp_func();
	compiler_function* f2 = init_comp_func();
	compiler_function* f3 = init_comp_func();
	compiler_function* f4 = init_comp_func();
	compiler_function* ff = init_comp_func();
	if (!(f1 && f2 && f3 && f4 && ff)) {
		return 0;
	}

	/*
	if (tokens[offset]->token_flags & TOKEN_OPERATOR && tokens[offset]->chars[0]=='(') {
		offset++;
		parentheses = true; //we should process this bytecode first before the others.
		addo++;
	}
	*/

	int unary_off= m_compile_read_unary_op(f2, tokens, offset, token_max);
	offset += unary_off;

	int varblock_off = 0;
	if (tokens[offset]->token_flags & TOKEN_OPERATOR && tokens[offset]->chars[0] == '(') {
		offset++;
		addo++;
		varblock_off = m_compile_read_var_statment_recur(f1, tokens, offset, token_max);
		if (!varblock_off) {
			delete_comp_func(f1);
			delete_comp_func(f2);
			delete_comp_func(f3);
			delete_comp_func(f4);
			delete_comp_func(ff);
			return 0;
		}
		offset += varblock_off;
		if (tokens[offset]->token_flags & TOKEN_OPERATOR && tokens[offset]->chars[0] == ')') {
			offset++;
			addo++;
		}
		else {
			func->errorcode = M_COMPERR_PAREN_NOT_CLOSED;
			func->token_error_num = offset;
			return 0;
		}
	}
	else {
		varblock_off = m_compile_read_variable(f1, tokens, offset, token_max);
		if (!varblock_off) {
			delete_comp_func(f1);
			delete_comp_func(f2);
			delete_comp_func(f3);
			delete_comp_func(f4);
			delete_comp_func(ff);
			return 0;
		}
		offset += varblock_off;
	}



	int index_off = m_compile_read_indexing(f1,tokens,offset,token_max);
	offset += index_off;

	mercury_int PREV_COMP_NUM_ARG_OUT = COMPILER_NUMBER_ARGS_OUT;
	COMPILER_NUMBER_ARGS_OUT = 1;
	int func_off = m_compile_read_fcall(ff , tokens, offset, token_max,f1);
	COMPILER_NUMBER_ARGS_OUT = PREV_COMP_NUM_ARG_OUT;

	offset += func_off;
	if (func_off) {
		delete_comp_func(f1);
		f1 = ff;
	}

	/*
	if (parentheses && tokens[offset]->token_flags & TOKEN_OPERATOR && tokens[offset]->chars[0] == ')') {
		offset++;
		parentheses = false;
		addo++;
	}
	*/

	int binop_off = m_compile_read_binary_op(f4, tokens, offset, token_max);
	if (binop_off) {
		offset += binop_off;
		int next_off = m_compile_read_var_statment_recur(f3, tokens, offset, token_max,f4);
		if (!next_off) {
			delete_comp_func(f1);
			delete_comp_func(f2);
			delete_comp_func(f3);
			delete_comp_func(f4);
			delete_comp_func(ff);
			return 0;
		}
		offset += next_off;

		binop_off += next_off;
	}

	/*
	if (parentheses && tokens[offset]->token_flags & TOKEN_OPERATOR && tokens[offset]->chars[0] == ')') {
		offset++;
		addo++;
	}
	else if (parentheses) {
		func->errorcode= M_COMPERR_PAREN_NOT_CLOSED;
		func->token_error_num = offset;
		return 0;
	}
	*/

	/*
		f1 - base
		f2 - unary op
		f3 - bin op statment
		f4 - binary op
	*/
	
	concat_comp_func_appends(f1, f2);
	if (parentheses) {
		concat_comp_func_appends(f1, f3);
		if (operation_append)concat_comp_func_appends(f1, operation_append);;
	}
	else {
		if (operation_append)concat_comp_func_appends(f1, operation_append);;
		concat_comp_func_appends(f1, f3); //was f1 f3
		//concat_comp_func_appends(f1, f4); //was f1 f4
	}


	concat_comp_func_appends(func, f1); //was func f1

	delete_comp_func(f1);
	delete_comp_func(f2);
	delete_comp_func(f3);
	delete_comp_func(f4);

	return unary_off+varblock_off+index_off+func_off+binop_off + addo;
}



//a block is something like print("hello, world!") or a=5. this is to say that it is either a function call or variable assignment
//code is comprised of many blocks
//bytecode order of operations:
// get -> index -> call -> unary -> binary -> set ; a=-b[c]()*4
//how it's written:
// set -> unary -> get -> index -> call -> binary
int mercury_compile_compile_block(compiler_function* func, compiler_token** tokens,mercury_int offset,mercury_int token_max, int flags) {
	//compiler_function* func= init_comp_func();
	if (!func)return 0;

	if (token_max < offset + 2) {
		func->errorcode = M_COMPERR_ENDS_TOO_SOON;
		func->token_error_num = offset;
		return 0;
	}

	compiler_token* cur_tok = tokens[offset];
	compiler_token* next_tok = tokens[offset + 1];

	if (cur_tok->token_flags & TOKEN_LOOP_MODIFIER) {
		if (COMPILER_INSIDE_LOOP) {
			if (cur_tok->chars[0] == 'c') { //continue
				m_compile_add_instruction(func, M_OPCODE_JMPR, 0,offset);
#ifdef MERCURY_64BIT
				m_compile_add_rawdatadouble(func, COMPILER_CONTINUE_JUMP_POSITION - func->number_instructions-2);
#else
				m_compile_add_rawdata(func, COMPILER_CONTINUE_JUMP_POSITION - func->number_instructions -1);
#endif
			}
			else if (cur_tok->chars[0] == 'b') { //break
				void* nptr=realloc(COMPILER_BREAK_ADDRS, (COMPILER_BREAK_AMOUNTS+1) * sizeof(mercury_int));
				if (!nptr) {
					func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
					func->token_error_num = offset;
					return 0;
				}
				COMPILER_BREAK_ADDRS = (mercury_int*)nptr;
				m_compile_add_instruction(func, M_OPCODE_JMPR, 0,offset);
				COMPILER_BREAK_ADDRS[COMPILER_BREAK_AMOUNTS] = func->number_instructions;
				COMPILER_BREAK_AMOUNTS++;
#ifdef MERCURY_64BIT
				m_compile_add_rawdatadouble(func, func->number_instructions);
#else
				m_compile_add_rawdata(func, func->number_instructions);
#endif
			}
			return 1;
		}
		else {
			func->errorcode = M_COMPERR_KEYWORD_REQUIRES_LOOP;
			func->token_error_num = offset;
			return 0;
		}
	}


	//goto labels:
	if (cur_tok->token_flags & TOKEN_ENVVARNAME && next_tok->token_flags & TOKEN_OPERATOR && next_tok->chars[0] == ':' && next_tok->num_chars==1) {
		if (!m_compile_register_jump_point_label(cur_tok->chars, cur_tok->num_chars, func->number_instructions)) {
			func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
			func->token_error_num = offset;
			return 0;
		}
		return 2;
	}
	if (cur_tok->token_flags & TOKEN_JUMP && next_tok->token_flags & TOKEN_ENVVARNAME) {
		if( !m_compile_register_jump_point_goto(next_tok->chars, next_tok->num_chars, func->number_instructions+1)) {
			func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
			func->token_error_num = offset;
			return 0;
		}
		m_compile_add_instruction(func, M_OPCODE_JMPR, 0,offset);
#ifdef MERCURY_64BIT
		m_compile_add_rawdatadouble(func, 0, offset);
#else
		m_compile_add_rawdata(func, 0, offset);
#endif

		return 2;
	}

	uint16_t set_opcode=M_OPCODE_SENV;
	uint16_t get_opcode=M_OPCODE_GENV;

	mercury_int addoff = 0;



	if (cur_tok->token_flags & TOKEN_EXIT) { //return is handled here because it's easier.
		addoff++;

		while (true) {
			int a=m_compile_read_var_statment_recur(func, tokens, offset+ addoff, token_max);
			if (!a)break;
			addoff += a;
			cur_tok = tokens[offset+ addoff];
			if (cur_tok->token_flags & TOKEN_OPERATOR && cur_tok->chars[0] == ',') {
				addoff++;

			}
			else {
				break;
			}
		}

		m_compile_add_instruction(func, M_OPCODE_EXIT, 0, offset);
		return addoff;
	}


	if (cur_tok->token_flags & TOKEN_SCOPESPECIFIER) {
		if (cur_tok->chars[0]=='l') {
			get_opcode = M_OPCODE_GETL;
			set_opcode = M_OPCODE_SETL;
		}else if (cur_tok->chars[0] == 'g') {
			get_opcode = M_OPCODE_GETG;
			set_opcode = M_OPCODE_SETG;
		}

		addoff = 1;
	}

	//scan for variables
	compiler_function* var_code = init_comp_func();
	compiler_function* var_code_final = init_comp_func();
	if (!var_code) {
		return 0;
	}
	if (!var_code_final) {
		return 0;
	}
	bool tokenpass = false;
	while (true) {
		concat_comp_func_appends(var_code, var_code_final);
		delete_comp_func(var_code_final);
		var_code_final=init_comp_func();
		if (!var_code_final)return 0;

		if (offset + addoff > token_max) {
			delete_comp_func(var_code);
			delete_comp_func(var_code_final);
			return 0;
		}
		cur_tok = tokens[offset + addoff];
		next_tok = tokens[offset + addoff + 1];

		if (tokenpass) {
			tokenpass = false;
			if ((cur_tok->token_flags & TOKEN_ENVVARNAME)) {
				break;
			}
		}
		else {
			if (!(cur_tok->token_flags & TOKEN_ENVVARNAME)) {
				break;
			}

			m_compile_cstring_load(var_code_final, cur_tok->chars, cur_tok->num_chars, offset+addoff);
			//m_compile_add_instruction(var_code, get_opcode);
			addoff++;
			cur_tok = tokens[offset + addoff];
			next_tok = tokens[offset + addoff + 1];
		}

		

		if (cur_tok->token_flags & TOKEN_OPERATOR) {
			if (cur_tok->chars[0] == '.' && cur_tok->num_chars == 1) { //period indexing will skip to the next itteration so that cstring_load will capture what you're trying to get.
				m_compile_add_instruction(var_code_final, get_opcode, 0, offset+addoff);
				addoff++;
				get_opcode = M_OPCODE_GET;
				set_opcode = M_OPCODE_SET;
			}
			else if (cur_tok->chars[0] == '[' && cur_tok->num_chars == 1) { // bracket indexing must first read a statment, and will additonally not call cstring_load.
				m_compile_add_instruction(var_code_final, get_opcode, 0, offset + addoff);
				addoff++;
				tokenpass = true;
				get_opcode = M_OPCODE_GET;
				set_opcode = M_OPCODE_SET;
				addoff+=m_compile_read_var_statment_recur(var_code_final,tokens,offset+addoff,token_max);
				addoff++;
			}
			else {
				break;
			}
		}
		else {
			func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
			func->token_error_num = offset + addoff + 1;
			return 0;
		}

	}

	cur_tok = tokens[offset + addoff];
	next_tok = tokens[offset + addoff + 1];


	if (cur_tok->token_flags & TOKEN_OPERATOR) {
		if (cur_tok->chars[0] == '(' && cur_tok->num_chars == 1) {
			mercury_int PREV_COMP_NUM_ARG_OUT = COMPILER_NUMBER_ARGS_OUT;
			COMPILER_NUMBER_ARGS_OUT = 0;
			concat_comp_func_appends(var_code, var_code_final);
			m_compile_add_instruction(var_code, get_opcode, 0, offset + addoff);
			int ad = m_compile_read_fcall(func, tokens, offset + addoff, token_max, var_code);
			delete_comp_func(var_code);
			delete_comp_func(var_code_final);

			PREV_COMP_NUM_ARG_OUT = COMPILER_NUMBER_ARGS_OUT;

			addoff += ad;
			return addoff;
		}
		else if (cur_tok->chars[0] == '=' && cur_tok->num_chars == 1) {
			addoff++;
			concat_comp_func_appends(func,var_code);
			concat_comp_func_appends(func, var_code_final);
			delete_comp_func(var_code);
			delete_comp_func(var_code_final);

			int ad = 0;
			if (next_tok->token_flags & TOKEN_KEYWORD && next_tok->num_chars == 8 && next_tok->chars[0] == 'f' && next_tok->chars[7] == 'n') {
				ad=mercury_compile_read_function_define(func, tokens, offset+addoff, token_max, false);
			}
			else {
				ad = m_compile_read_var_statment_recur(func, tokens, offset + addoff, token_max);
			}
			m_compile_add_instruction(func, set_opcode, 0, offset + addoff);
			addoff += ad;
			return addoff;
		}
		else if (cur_tok->token_flags & TOKEN_SELFMODIFY) { // TODO: we need to bifrucate var_code into 2 sections, the first being the bulk and the seconds being the final variable. eg a is a, a[0] is 0, a[0][n] is n.
			addoff++; // THEN, we need to replace the following code with a CPYX 2, so that way doing += and whatnot is actually faster (hopefully)
			concat_comp_func_appends(func, var_code);
			concat_comp_func_appends(func, var_code_final);
			if (var_code->number_instructions) //we only copy top 2 if we need to index
			{
				m_compile_add_instruction(func, M_OPCODE_CPYX, 0, offset + addoff);
#ifdef MERCURY_64BIT
				m_compile_add_rawdatadouble(func, 2, offset + addoff);
#else
				m_compile_add_rawdata(func, 2, offset + addoff);
#endif
			}
			else { //otherwise we can just copy top.
				m_compile_add_instruction(func, M_OPCODE_CPYT, 0, offset + addoff);
			}
			m_compile_add_instruction(func, get_opcode, 0, offset + addoff);
			delete_comp_func(var_code);
			delete_comp_func(var_code_final);
			//m_compile_add_instruction(func, M_OPCODE_CPYT, 0, offset + addoff);
			int ad = 0;
			if (next_tok->token_flags & TOKEN_KEYWORD && next_tok->num_chars == 8 && next_tok->chars[0] == 'f' && next_tok->chars[7] == 'n') {
				func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
				func->token_error_num = offset + addoff + 1;
				return 0;
			}
			else {
				ad = m_compile_read_var_statment_recur(func, tokens, offset + addoff, token_max);
			}
			m_compile_add_instruction(func, m_compile_get_operator_bytecode(cur_tok,0), 0, offset + addoff);
			m_compile_add_instruction(func, set_opcode, 0, offset + addoff);
			addoff += ad;
			return addoff;
		}
		else {
			func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
			func->token_error_num = offset + addoff + 1;
			return 0;
		}
	}
	else {
		func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
		func->token_error_num = offset + addoff + 1;
		return 0;
	}

}

int mercury_compile_compile_blocks_until_keyword(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max, int flags) {
	int ad = 0;
	while (true) {
		if (offset + ad >= token_max) {
			return ad;
		}
		compiler_token* cur_tok = tokens[offset+ ad];
		if (cur_tok->token_flags & TOKEN_KEYWORD) {
			return ad;
		}
		int a=mercury_compile_compile_block(func, tokens, offset+ ad, token_max, flags);
		if (!a) {
			return ad;
		}
		ad += a;
	}
}

int mercury_compile_read_if_statment(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max);
int mercury_compile_read_while_statment(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max);


int mercury_compile_read_if_statment(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	mercury_int instruction_position = 0;
	mercury_int* instruction_position_blockends = nullptr;
	int adv = 0;
	mercury_int cur_off = offset;
	int bodycount = 0;

	if ((tokens[offset]->token_flags & TOKEN_KEYWORD) && tokens[offset]->num_chars==2 && tokens[offset]->chars[0]=='i' && tokens[offset]->chars[1] == 'f') {
		adv++;
		cur_off++;
	}
	else {
		return 0;
	}


	int a = m_compile_read_var_statment_recur(func, tokens, offset + adv, token_max);
	if (!a) {
		return 0;
	}
	adv += a;
	cur_off += a;

	if ((tokens[cur_off]->token_flags & TOKEN_KEYWORD) && tokens[cur_off]->num_chars == 4 && tokens[cur_off]->chars[0] == 't' && tokens[cur_off]->chars[1] == 'h' && tokens[cur_off]->chars[2] == 'e' && tokens[cur_off]->chars[3] == 'n') {
		adv++;
		cur_off++;
	}
	else {
		func->errorcode = M_COMPERR_IF_NEEDS_THEN;
		func->token_error_num = cur_off+1;
		return 0;
	}

	m_compile_add_instruction(func, M_OPCODE_JRNI, 0,cur_off);
	instruction_position = func->number_instructions;
	#ifdef MERCURY_64BIT
	m_compile_add_rawdatadouble(func, 0, cur_off);
	#else
	m_compile_add_rawdata(func, 0, cur_off);
	#endif

	while (true)
	{
		if (func->errorcode) {
			return 0;
		}
		if (cur_off >= token_max) {
			free(instruction_position_blockends);
			func->errorcode = M_COMPERR_ENDS_TOO_SOON;
			func->token_error_num = token_max-1;
			return adv;
		}
		compiler_token* ct = tokens[cur_off];
			if (ct->token_flags & TOKEN_KEYWORD) {
				if (ct->num_chars == 2 && ct->chars[0] == 'i' && ct->chars[1] == 'f') {
					int a = mercury_compile_read_if_statment(func,tokens, cur_off,token_max);
					if (!a) {
						return 0;
					}
					adv += a;
					cur_off += a;
				}
				else if(ct->num_chars == 6 && ct->chars[0] == 'e' && ct->chars[1] == 'l' && ct->chars[2] == 's' && ct->chars[3] == 'e' && ct->chars[4] == 'i' && ct->chars[5] == 'f') {
					adv++;
					cur_off++;
					bodycount++;
					m_compile_add_instruction(func, M_OPCODE_JMPR, 0, cur_off);

					mercury_int* nptr = (mercury_int*)realloc(instruction_position_blockends, sizeof(mercury_int) * bodycount);
					if (!nptr) {
						func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
						func->token_error_num = cur_off;
						free(instruction_position_blockends);
						return 0;
					}
					instruction_position_blockends = nptr;
					instruction_position_blockends[bodycount - 1] = func->number_instructions;

					#ifdef MERCURY_64BIT
					m_compile_add_rawdatadouble(func, 0, cur_off);
					#else
					m_compile_add_rawdata(func, 0, cur_off);
					#endif

#ifdef MERCURY_64BIT
					mercury_int diff = func->number_instructions- instruction_position -2;
#else
					mercury_int diff = func->number_instructions - instruction_position - 1;
#endif
					
					#ifdef MERCURY_64BIT
					*(uint64_t*)(func->instructions + instruction_position) = *((uint64_t*)(&diff));
					#else
					*(func->instructions + instruction_position) = *((uint32_t*)(&diff));
					#endif
					
					int a = m_compile_read_var_statment_recur(func, tokens, offset + adv, token_max);
					if (!a) {
						return 0;
					}
					adv += a;
					cur_off += a;

					if ((tokens[cur_off]->token_flags & TOKEN_KEYWORD) && tokens[cur_off]->num_chars == 4 && tokens[cur_off]->chars[0] == 't' && tokens[cur_off]->chars[1] == 'h' && tokens[cur_off]->chars[2] == 'e' && tokens[cur_off]->chars[3] == 'n') {
						adv++;
						cur_off++;
					}
					else {
						func->errorcode = M_COMPERR_ELSEIF_NEEDS_THEN;
						func->token_error_num = cur_off + 1;
						return 0;
					}

					m_compile_add_instruction(func, M_OPCODE_JRNI, 0, cur_off);
					instruction_position = func->number_instructions;
					#ifdef MERCURY_64BIT
					m_compile_add_rawdatadouble(func, 0, cur_off);
					#else
					m_compile_add_rawdata(func, 0, cur_off);
					#endif

					a =mercury_compile_compile_blocks_until_keyword(func,tokens,offset,token_max,0);
					adv += a;
					cur_off += a;

				}
				else if (ct->num_chars == 4 && ct->chars[0] == 'e' && ct->chars[1] == 'l' && ct->chars[2] == 's' && ct->chars[3] == 'e' ) {
					adv++;
					cur_off++;

					bodycount++;
					m_compile_add_instruction(func, M_OPCODE_JMPR, 0, cur_off);

					mercury_int* nptr = (mercury_int*)realloc(instruction_position_blockends, sizeof(mercury_int) * bodycount);
					if (!nptr) {
						func->errorcode = M_COMPERR_MEMORY_ALLOCATION;
						func->token_error_num = cur_off;
						free(instruction_position_blockends);
						return 0;
					}
					instruction_position_blockends = nptr;
					instruction_position_blockends[bodycount - 1] = func->number_instructions;


					mercury_int ip2= func->number_instructions;

					#ifdef MERCURY_64BIT
					m_compile_add_rawdatadouble(func, 0, cur_off);
					#else
					m_compile_add_rawdata(func, 0, cur_off);
					#endif

#ifdef MERCURY_64BIT
					mercury_int diff = func->number_instructions - instruction_position - 2;
#else
					mercury_int diff = func->number_instructions - instruction_position - 1;
#endif

					#ifdef MERCURY_64BIT
					* (uint64_t*)(func->instructions + instruction_position) = *((uint64_t*)(&diff));
					#else
					* (func->instructions + instruction_position) = *((uint32_t*)(&diff));
					#endif
					int a=mercury_compile_compile_blocks_until_keyword(func, tokens, offset + adv, token_max,0);
					adv += a;
					cur_off += a;

					instruction_position = ip2;

				}
				else if (ct->num_chars == 3 && ct->chars[0] == 'e' && ct->chars[1] == 'n' && ct->chars[2] == 'd') {
					mercury_int diff =  func->number_instructions - instruction_position - 2;
					#ifdef MERCURY_64BIT
					* (uint64_t*)(func->instructions + instruction_position) = *((uint64_t*)(&diff));
					#else
					* (func->instructions + instruction_position) = *((uint32_t*)(&diff));
					#endif

					for (int i = 0; i < bodycount; i++) {
						mercury_int diff = func->number_instructions - instruction_position_blockends[i] -2;
						#ifdef MERCURY_64BIT
						* (uint64_t*)(func->instructions + instruction_position_blockends[i]) = *((uint64_t*)(&diff));
						#else
						* (func->instructions + instruction_position_blockends[i]) = *((uint32_t*)(&diff));
						#endif
					}
					return adv+1;
				}
				else if (ct->num_chars ==5 && ct->chars[0] == 'w' && ct->chars[1] == 'h' && ct->chars[2] == 'i' && ct->chars[3] == 'l' && ct->chars[4] == 'e') {
					int a=mercury_compile_read_while_statment(func,tokens,cur_off,token_max);
					if (!a) {
						return 0;
					}
					adv += a;
					cur_off += a;
				}
				else if (ct->num_chars == 8 && ct->chars[0] == 'f' && ct->chars[7] == 'n') {
					a = mercury_compile_read_function_define(func, tokens, cur_off, token_max, true);
					if (!a) {
						return adv;
					}
				}
				else
				{
				free(instruction_position_blockends);
				func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
				func->token_error_num = cur_off;
				return adv;
				}
			}
			else {
				int a = mercury_compile_compile_blocks_until_keyword(func, tokens, cur_off, token_max,0);
				//if (!a) {
				//	return 0;
				//}
				adv += a;
				cur_off += a;
			}


	}
	return 0;

}

int mercury_compile_read_while_statment(compiler_function* func, compiler_token** tokens, mercury_int offset, mercury_int token_max) {
	mercury_int cur_off = offset;
	int adv = 0;

	compiler_token* ct = tokens[offset];
	if (!(ct->token_flags & TOKEN_KEYWORD && ct->num_chars == 5 && ct->chars[0] == 'w' && ct->chars[1] == 'h' && ct->chars[2] == 'i' && ct->chars[3] == 'l' && ct->chars[4] == 'e')) {
		return 0;
	}
	adv++;
	cur_off++;

	mercury_int addrstart= func->number_instructions;

	int a = m_compile_read_var_statment_recur(func, tokens, cur_off, token_max);
	if (!a) {
		func->errorcode = M_COMPERR_EXPECTED_VARIABLE;
		func->token_error_num = cur_off;
		return 0;
	}
	cur_off += a;
	adv += a;

	ct = tokens[cur_off];
	if (!(ct->token_flags & TOKEN_KEYWORD && ct->num_chars == 2 && ct->chars[0] == 'd' && ct->chars[1] == 'o' )) {
		func->errorcode = M_COMPERR_WHILE_NEEDS_DO;
		func->token_error_num = cur_off;
		return 0;
	}
	adv++;
	cur_off++;

	m_compile_add_instruction(func, M_OPCODE_JRNI, 0, cur_off);
	mercury_int addrjmp = func->number_instructions;



	mercury_int PREV_CJP = COMPILER_CONTINUE_JUMP_POSITION;
	bool PREV_IL = COMPILER_INSIDE_LOOP;
	mercury_int PREV_BA = COMPILER_BREAK_AMOUNTS;
	mercury_int* PREV_BADDRS = COMPILER_BREAK_ADDRS;

	COMPILER_CONTINUE_JUMP_POSITION = addrstart;
	COMPILER_INSIDE_LOOP = true;
	COMPILER_BREAK_AMOUNTS = 0;
	COMPILER_BREAK_ADDRS = nullptr;



	#ifdef MERCURY_64BIT
	m_compile_add_rawdatadouble(func, 0, cur_off);
	#else
	m_compile_add_rawdata(func, 0, cur_off);
	#endif





	while (true) {
		if (func->errorcode) {
			return 0;
		}
		cur_off = offset + adv;
		if (cur_off > token_max) {
			func->errorcode = M_COMPERR_ENDS_TOO_SOON;
			func->token_error_num = cur_off;
			return 0;
		}
		ct = tokens[cur_off];
		int a=0;
		
		if (ct->token_flags & TOKEN_KEYWORD) {
			if (ct->num_chars == 3 && ct->chars[0]=='e' && ct->chars[1] == 'n' && ct->chars[2] == 'd') {

				m_compile_add_instruction(func, M_OPCODE_JMPR, 0, cur_off);
#ifdef MERCURY_64BIT
				m_compile_add_rawdatadouble(func, addrstart - func->number_instructions-2, cur_off);
#else
				m_compile_add_rawdata(func, addrstart - func->number_instructions -2, cur_off);
#endif

#ifdef MERCURY_64BIT
				mercury_int r = func->number_instructions - addrjmp - 2;
				* (uint64_t*)(func->instructions + addrjmp) = *((uint64_t*)&r);
#else
				mercury_int r = func->number_instructions - addrjmp - 1;
				* (uint32_t*)(func->instructions + addrjmp) = *((uint32_t*)&r);
#endif


				for (mercury_int i = 0; i < COMPILER_BREAK_AMOUNTS;i++) {
					mercury_int addr = COMPILER_BREAK_ADDRS[i];
#ifdef MERCURY_64BIT
					mercury_int r = func->number_instructions - addr - 2;
					*(uint64_t*)(func->instructions + addr) = *((uint64_t*)&r);
#else
					mercury_int r = func->number_instructions - addr - 1;
					*(uint32_t*)(func->instructions + addr) = *((uint32_t*)&r);
#endif
				}
				free(COMPILER_BREAK_ADDRS);

				COMPILER_CONTINUE_JUMP_POSITION = PREV_CJP;
				COMPILER_INSIDE_LOOP = PREV_IL;
				COMPILER_BREAK_AMOUNTS = PREV_BA;
				COMPILER_BREAK_ADDRS= PREV_BADDRS;

				adv++;
				return adv;
			}else if (ct->num_chars == 5 && ct->chars[0] == 'w' && ct->chars[1] == 'h' && ct->chars[2] == 'i' && ct->chars[3] == 'l' && ct->chars[4] == 'e') {
				a = mercury_compile_read_while_statment(func, tokens, cur_off, token_max);
				if (!a) {
					return adv;
				}
				adv += a;
			}
			else if (ct->num_chars == 2 && ct->chars[0] == 'i' && ct->chars[1] == 'f') {
				a = mercury_compile_read_if_statment(func, tokens, cur_off, token_max);
				if (!a) {
					return adv;
				}
				adv += a;
			}
			else if (ct->num_chars == 8 && ct->chars[0] == 'f' && ct->chars[7] == 'n') {
				a = mercury_compile_read_function_define(func, tokens, cur_off, token_max, true);
				if (!a) {
					return adv;
				}
			}
			else {
				func->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
				func->token_error_num = cur_off;
				return 0;
			}
		}
		else {
			a=mercury_compile_compile_blocks_until_keyword(func,tokens,cur_off,token_max,0);
			adv += a;
			cur_off += a;
		}


		//break;
	}

	return 0;
}



compiler_function* mercury_compile_compile_tokens(compiler_token** tokens, mercury_int num_tokens , mercury_int* endatend ) {
	mercury_int cur_token = 0;
	compiler_function* out=init_comp_func();
	if (!out) { return nullptr; }


	JUMP_POINT** OLD_JUMPDATABASE = COMPILER_JUMP_DATABASE;
	mercury_int OLD_JUMPAMTS = COMPILER_JUMP_NUMBERS;

	COMPILER_JUMP_DATABASE = nullptr;
	COMPILER_JUMP_NUMBERS = 0;

	while (true) {
		if (cur_token >= num_tokens || out->errorcode) {
			break;
		}
		compiler_token* thistoken = tokens[cur_token];

		if (!(thistoken->token_flags & TOKEN_KEYWORD)) {
			int adv = mercury_compile_compile_blocks_until_keyword(out, tokens, cur_token, num_tokens, 0);
			cur_token += adv;
		}
		else {
			if (thistoken->num_chars == 2 && thistoken->chars[0] == 'i' && thistoken->chars[1] == 'f') {
				int adv=mercury_compile_read_if_statment(out, tokens, cur_token, num_tokens);
				if (!adv) {
					return out;
				}
				cur_token += adv;
			}else if (thistoken->num_chars == 5 && thistoken->chars[0] == 'w' && thistoken->chars[1] == 'h' && thistoken->chars[2] == 'i' && thistoken->chars[3] == 'l' && thistoken->chars[4] == 'e') {
				int adv = mercury_compile_read_while_statment(out, tokens, cur_token, num_tokens);
				if (!adv) {
					return out;
				}
				cur_token += adv;
			}
			else if (thistoken->num_chars == 8 && thistoken->chars[0] == 'f' && thistoken->chars[7] == 'n') {
				int adv = mercury_compile_read_function_define(out,tokens,cur_token,num_tokens,true);
				if (!adv) {
					return out;
				}
				cur_token += adv;
			
			}
			else {
				if (endatend && thistoken->num_chars == 3 && thistoken->chars[0] == 'e' && thistoken->chars[1] == 'n' && thistoken->chars[2] == 'd')
				{
					*endatend = cur_token;
					break;
				}
				else {
					out->errorcode = M_COMPERR_DID_NOT_CALL_OR_SET;
					out->token_error_num = cur_token;
					return out;
				}
			}
		}

	}




	if (COMPILER_JUMP_DATABASE) {
		for (mercury_int i = 0; i < COMPILER_JUMP_NUMBERS; i++) {
			JUMP_POINT* J = COMPILER_JUMP_DATABASE[i];
			if (!J->defined)J->num_positions = 0; //do not attempt gotos if no label.

			for (mercury_int n = 0; n < J->num_positions; n++) {
				void* varaddr=(void*)(out->instructions+J->positions[n]);
#ifdef MERCURY_64BIT
				*(int64_t*)varaddr = J->label_point - J->positions[n]-2;
#else
				*(int32_t*)varaddr = J->label_point - J->positions[n]-1;
#endif
			}

			free(J->label);
			free(J->positions);
			free(J);
		}
	}
	free(COMPILER_JUMP_DATABASE);


	COMPILER_JUMP_DATABASE=OLD_JUMPDATABASE;
	COMPILER_JUMP_NUMBERS=OLD_JUMPAMTS;

	


	return out;
}






mercury_variable* mercury_compile_mstring(mercury_stringliteral* str) {

	static const char* empty_token_string = "";

	mercury_variable* out = (mercury_variable*)malloc(sizeof(mercury_variable));
	if (!out)return nullptr;

	compiler_token** tokens=mercury_compile_tokenize_mstring(str);

	mercury_int num_t = 0;
	while (tokens[num_t]) { num_t++; }

#if defined(DEBUG) || defined(_DEBUG)
	for (mercury_int i = 0; i < num_t; i++) {
		compiler_token* t = tokens[i];
		for (mercury_int n = 0; n < t->num_chars; n++) {
			putchar(t->chars[n]);
		}
		putchar('\t');
		int flags = t->token_flags;
		if (flags & TOKEN_OPERATOR) {
			printf("OPERATOR ");
		}
		if (flags & TOKEN_UANRY) {
			printf("UNARY ");
		}
		if (flags & TOKEN_BINARY) {
			printf("BINARY ");
		}
		if (flags & TOKEN_TAKESTATICNUM) {
			printf("TAKESSTATICNUM ");
		}
		if (flags & TOKEN_SELFMODIFY) {
			printf("SELFMODIFY ");
		}
		if (flags & TOKEN_STATICSTRING) {
			printf("STATICSTRING ");
		}
		if (flags & TOKEN_STATICNUMBER) {
			printf("STATICNUMBER ");
		}
		if (flags & TOKEN_VARIABLE) {
			printf("VARIABLE ");
		}
		if (flags & TOKEN_ENVVARNAME) {
			printf("ENVVARNAME ");
		}
		if (flags & TOKEN_KEYWORD) {
			printf("KEYWORD ");
		}
		if (flags & TOKEN_STATICBOOLEAN) {
			printf("STATICBOOLEAN ");
		}
		if (flags & TOKEN_STATICNIL) {
			printf("STATICNIL ");
		}
		if (flags & TOKEN_SCOPESPECIFIER) {
			printf("SCOPESPECIFIER ");
		}
		if (flags & TOKEN_SPECIALVARIABLECREATE) {
			printf("SPECIALVARIABLECREATE ");
		}
		if (flags & TOKEN_LOOP_MODIFIER) {
			printf("LOOP_MODIFIER ");
		}
		if (flags & TOKEN_JUMP) {
			printf("JUMP ");
		}
		if (flags & TOKEN_EXIT) {
			printf("EXIT ");
		}
		
		putchar('\n');
	}
#endif


	compiler_function* f = mercury_compile_compile_tokens(tokens, num_t);

	if (f->errorcode) {
		mercury_stringliteral* s=mercury_generate_compiler_error_string(tokens,f->errorcode,f->token_error_num,num_t);
		out->data.p = s;
		out->type = M_TYPE_STRING;
		return out;
	}



	out->type = M_TYPE_FUNCTION;
	mercury_function* mf=(mercury_function*)malloc(sizeof(mercury_function));
	if (!mf)return nullptr;

	mf->debug_info = (mercury_debug_token*)malloc(sizeof(mercury_debug_token)*f->number_instructions);
	if (!mf->debug_info) {
		free(mf);
		return nullptr;
	}

	for (mercury_int i = 0; i < f->number_instructions; i++) {
		mercury_int token_n = f->instruction_tokens[i];
		
		//printf("%i %i\n",i,token_n);
		

		compiler_token* t = tokens[token_n];

		mf->debug_info[i].col = t->line_col;
		mf->debug_info[i].line = t->line_num;
		mf->debug_info[i].token = (char*)malloc(sizeof(char) * (t->num_chars+1));
		memcpy((void*)mf->debug_info[i].token, t->chars, t->num_chars);
		mf->debug_info[i].token[t->num_chars] = '\0';

		mf->debug_info[i].token_next = (char*)empty_token_string;
		mf->debug_info[i].token_next_next = (char*)empty_token_string;
		mf->debug_info[i].token_prev = (char*)empty_token_string;
		mf->debug_info[i].token_prev_prev = (char*)empty_token_string;

		if (token_n < num_t-1) {
			compiler_token* t = tokens[token_n+1];
			mf->debug_info[i].token_next = (char*)malloc(sizeof(char) * (t->num_chars + 1));
			memcpy((void*)mf->debug_info[i].token_next, t->chars, t->num_chars);
			mf->debug_info[i].token_next[t->num_chars] = '\0';
		}
		if (token_n < num_t - 2) {
			compiler_token* t = tokens[token_n + 2];
			mf->debug_info[i].token_next_next = (char*)malloc(sizeof(char) * (t->num_chars + 1));
			memcpy((void*)mf->debug_info[i].token_next_next, t->chars, t->num_chars);
			mf->debug_info[i].token_next_next[t->num_chars] = '\0';
		}
		if (token_n > 0) {
			compiler_token* t = tokens[token_n-1];
			mf->debug_info[i].token_prev = (char*)malloc(sizeof(char) * (t->num_chars + 1));
			memcpy((void*)mf->debug_info[i].token_prev, t->chars, t->num_chars);
			mf->debug_info[i].token_prev[t->num_chars] = '\0';
		}
		if (token_n > 1) {
			compiler_token* t = tokens[token_n - 2];
			mf->debug_info[i].token_prev_prev = (char*)malloc(sizeof(char) * (t->num_chars + 1));
			memcpy((void*)mf->debug_info[i].token_prev_prev, t->chars, t->num_chars);
			mf->debug_info[i].token_prev_prev[t->num_chars] = '\0';
		}
	}

	mf->refrences = 1;
	mf->numberofinstructions = f->number_instructions;
	mf->instructions = (uint32_t*)malloc(f->number_instructions * sizeof(uint32_t));
	if (!mf->instructions)return nullptr;
	memcpy(mf->instructions,f->instructions, f->number_instructions * sizeof(uint32_t));
	out->data.p = mf;
	delete_comp_func(f);
	free(tokens);

	return out;
}
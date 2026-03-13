#include "mercury_compiler.hpp"
#include "mercury_bytecode.hpp"
#include "mercury.hpp"
#include <malloc.h>
#include <stdint.h>
#include <string.h>

enum comment_type:uint8_t {
	COMMENT_NONE=0,
	COMMENT_LINE=1,
	COMMENT_MULTI=2,
};

enum token_flags {
	TOKEN_OPERATOR = 1 << 1,
	TOKEN_UNARY_OP = 1 << 2,
	TOKEN_BINARY_OP = 1 << 3,
	TOKEN_MISC_OP = 1 << 4,
	TOKEN_SELFMODIFY_OP = 1 << 5,
	TOKEN_VARIABLE = 1 << 6,
	TOKEN_TAKES_STATIC_NUM = 1 << 7,
	TOKEN_TAKES_STATIC_STRING = 1 << 8,
	TOKEN_STATIC_NUMBER = 1 << 9,
	TOKEN_STATIC_STRING = 1 << 10,
	TOKEN_STATIC_BOOLEAN = 1 << 11,
	TOKEN_STATIC_NIL = 1 << 12,
	TOKEN_ENV_VAR = 1 << 13,
	TOKEN_KEYWORD = 1 << 14,
	TOKEN_SCOPESPECIFIER = 1 << 15,
	TOKEN_LOOP_MODIFIER = 1 << 16,
};


enum compiler_error_codes {
	M_COMPERR_MEMORY_ALLOCATION,
	M_COMPERR_NO_MORE_TOKENS,

	M_COMPERR_MALFORMED_NUMBER,
	M_COMPERR_WRONG_SYMBOL,
	M_COMPERR_NO_OPERATION_FOUND, //if we don't do anything with a var, so instead of a=5, we just do a

	M_COMPERR_WHILE_NEEDS_DO,
	M_COMPERR_IF_NEEDS_THEN,
	M_COMPERR_LOOPKEYWORD_WRONG_SPOT,
	M_COMPERR_ARRAY_NOT_CLOSED,
	M_COMPERR_TABLE_NOT_CLOSED,
	M_COMPERR_CALL_NOT_CLOSED,
	M_COMPERR_NESTEDSTATMENT_NOT_CLOSED, // ()
	M_COMPERR_NO_VARIABLE, //for any variable of any type with any syntax, including variable statments such as a>5
	M_COMPERR_NO_NAMED_VARIABLE, //this is for implicit variable names, under TOKEN_ENV_VAR.
};


enum token_type {
	TOKEN_TYPE_OPERATOR = 1,
	TOKEN_TYPE_KEYWORD = 2,
	TOKEN_TYPE_STRING = 3,
	TOKEN_TYPE_NUMBER = 4,
};


struct compiler_token {
	mercury_int line_num = 0;
	mercury_int line_col = 0;
	char* chars = nullptr;
	int num_chars = 0;
	int token_flags = 0;
	int token_type = 0;
};

struct compiler_function {
	mercury_fullinstruction* instructions = nullptr;
	mercury_int* instruction_tokens = nullptr; //which token each instruction points to
	mercury_int number_instructions = 0;
	mercury_int tokens_consumed = 0;
	
	
	mercury_int token_error_num = 0;
	int errorcode = 0;
};



inline bool char_is_character(unsigned char c){
	return (c>=0x41 && c<=0x5A) || (c>=0x61 && c<=0x7A) || c=='_';
}

inline bool char_is_number(unsigned char c) {
	return (c >= 0x30 && c <= 0x39);
}
//semicolon is considered whitespace, not a symbol. is is to appease c users who go;
inline bool char_is_symbol(unsigned char c) {
	return ((c>=0x21 && c<=0x2F) || (c>= 0x3C && c<= 0x40) || (c>= 0x5B && c<= 0x60) || (c>= 0x7B && c<= 0x7E) || c==0x3A);
}

inline bool char_is_whitespace(unsigned char c) {
	return (!(char_is_symbol(c) || char_is_number(c) || char_is_character(c)));
}


inline compiler_token* new_compiler_token(mercury_int col_num, mercury_int line_num){
	compiler_token* out=(compiler_token*)malloc(sizeof(compiler_token));
	if(out){
		out->line_num= line_num;
		out->line_col= col_num;
		out->chars = nullptr;
		out->num_chars = 0;
		out->token_flags = 0;
		out->token_type=0;
	}
	return out;
}

inline bool append_char_to_compiler_token(compiler_token* token, char c){
	char* naddr=(char*)realloc(token->chars,sizeof(char)*(token->num_chars+1));
	if (!naddr)return false;
	token->chars=naddr;
	token->chars[token->num_chars]=c;
	token->num_chars++;
	return true;
}

inline void advance_position_from_char(char c,char c_prev,mercury_int* col,mercury_int* line){
	(*col)++;
	if(c=='\n' || c=='\r' && !(c_prev=='\r' && c=='\n') ){
		*col=1;
		(*line)++;
	}
}

inline bool add_token_or_destroy_array(compiler_token*** array,mercury_int* size,compiler_token* toadd){
	if (!toadd->num_chars && toadd->token_type!=TOKEN_TYPE_STRING) {
		free(toadd->chars);
		free(toadd);
		return false;
	}
	compiler_token** nptr=(compiler_token**)realloc(*array,sizeof(compiler_token*)*(*size+1) );
	if(!nptr){
		for(mercury_int i=0;i<(const mercury_int)*size;i++){
			free((*array)[i]);
		}
		free(array);
		*size=0;
		return true;
	}else{
		*array=nptr;
		(*array)[*size]=toadd;
		(*size)++;
		return false;
	}
}


inline bool token_matches_chars(compiler_token* t,const char* s){
	if(strlen(s)!=t->num_chars)return false;
	for (mercury_int c = 0; c < t->num_chars; c++) {
		if (t->chars[c] != s[c])return false;
	}
	return true;
}

bool symbol_can_merge_token(compiler_token* t,const char c){
	if(!t->num_chars)return true;
	const char last_char = t->chars[t->num_chars-1];
	switch(last_char){
		//symbols that can be repeated with themselves only. ==
		case '=': // ==
		// "where's (), {}, and []?" you may ask. well, it's more helpful for the compiler if we read each segment individually, so we don't merge those.
			return last_char == c;
		//symbols that can be repeated with themselves and equals. += ++ && &=
		case '!': // !! !=
		case '&': // && &=
		case '|': // || |=
		case '~': // ~~ ~=
		case '+': // ++ +=
		case '-': // -- -=
		case '<': // << <=
		case '>': // >> >=
		case '.': // .. ..=
			return last_char == c || c == '=';
		//symbols that can only be repeated with equals
		case '*': // *=
		case '/': // /=
		case '\\': // \=
		case '%': // %=
		case '^': // ^=
			return c == '=';
		default: //otherwise, the symbol is not repeatable
			return false;
	}
}

inline bool char_is_hex(char c){
	return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');
}

inline unsigned char read_hex_string_as_num(char c1,char c2){ //big endian
	unsigned char out=0;
	if(c1>='0' && c1<='9'){
		out=c1-'0';
	}else if(c1>='a' && c1<='f'){
		out=10+c1-'a';
	}else if(c1>='A' && c1<='F'){
		out=10+c1-'A';
	}
	
	out<<=4;
	
	if(c2>='0' && c2<='9'){
		out|=c2-'0';
	}else if(c2>='a' && c2<='f'){
		out|=10+c2-'a';
	}else if(c2>='A' && c2<='F'){
		out|=10+c2-'A';
	}
	
	return out;
}

inline bool char_is_dec(char c){
	return (c>='0' && c<='9');
}

inline unsigned char read_dec_string_as_num(char c1,char c2,char c3){
	unsigned char out=0;
	if(c1>='0' && c1<='9'){
		out=c1-'0';
	}
	
	out*=10;
	
	if(c2>='0' && c2<='9'){
		out+=c2-'0';
	}
	
	out*=10;
	
	if(c3>='0' && c3<='9'){
		out+=c3-'0';
	}
	
	return out;
}

compiler_token** mercury_compile_tokenize_mstring(mercury_stringliteral* str,mercury_int* num_out){
	
	*num_out = 0;

	compiler_token* cur_tok=new_compiler_token(1,1);
	compiler_token** out=nullptr;
	
	bool reading_string=false;
	int8_t comment_mode=COMMENT_NONE;
	
	mercury_int line_num=1;
	mercury_int col_num=1;
	
	mercury_int i=0;
	const char* s=str->ptr;
	const mercury_int size=str->size;
	while(i<size){
		char c=s[i];
		char c_prev=i ? s[i-1] : '\0';

		if(comment_mode){
			advance_position_from_char(c,c_prev,&col_num,&line_num);
			i++;
			if(comment_mode==COMMENT_LINE && (c=='\n' || c=='\r') ){
				comment_mode=COMMENT_NONE;
			}else if(comment_mode==COMMENT_MULTI && c=='/' && c_prev=='*'){
				comment_mode=COMMENT_NONE;
			}
		}
		else{
			if(!cur_tok){
				for(mercury_int i=0;i<(const mercury_int)*num_out;i++){
					free(out[i]);
				}
				free(out);
				*num_out=0;
				return nullptr;
			}
				
			switch(cur_tok->token_type){
				case TOKEN_TYPE_STRING:
					if(c=='\"'){ //end string reading.
						if(add_token_or_destroy_array(&out,num_out,cur_tok))return nullptr;
						cur_tok=new_compiler_token(col_num, line_num);
						advance_position_from_char(c, c_prev, &col_num, &line_num);
						i++;
					}else if(c=='\\'){ //escapes.
						advance_position_from_char(c, c_prev, &col_num, &line_num);
						i++;
						char nc=i<size?s[i]:'\0';
						advance_position_from_char(nc, c, &col_num, &line_num);
						i++; //chars used: 2
						switch(nc){
							case 'a':
								append_char_to_compiler_token(cur_tok,'\a');
								break;
							case 'b':
								append_char_to_compiler_token(cur_tok,'\b');
								break;
							case 'c':
								append_char_to_compiler_token(cur_tok,'\c');
								break;
							case 'e':
								append_char_to_compiler_token(cur_tok,'\e');
								break;
							case 'f':
								append_char_to_compiler_token(cur_tok,'\f');
								break;
							case 'n':
								append_char_to_compiler_token(cur_tok,'\n');
								break;
							case 'r':
								append_char_to_compiler_token(cur_tok,'\r');
								break;
							case 't':
								append_char_to_compiler_token(cur_tok,'\t');
								break;	
							case 'v':
								append_char_to_compiler_token(cur_tok,'\v');
								break;
							case '\\':
								append_char_to_compiler_token(cur_tok,'\\');
								break;
							case '\"':
								append_char_to_compiler_token(cur_tok,'\"');
								break;
							case 'x':
							case 'X': //raw hex char reading
								{
									char hc1=i<size?s[i]:'\0';
									i++;
									char hc2=i<size?s[i]:'\0';
									i++;
									
									if(char_is_hex(hc1) && char_is_hex(hc2)){
										advance_position_from_char(hc1, nc, &col_num, &line_num);
										advance_position_from_char(hc2, hc1, &col_num, &line_num);
										append_char_to_compiler_token(cur_tok,read_hex_string_as_num(hc1,hc2));
									}else if(char_is_hex(hc1)){
										append_char_to_compiler_token(cur_tok,read_hex_string_as_num('0',hc1));
										advance_position_from_char(hc1, nc, &col_num, &line_num);
										i--;
									}else{
										append_char_to_compiler_token(cur_tok,'\0');
										i-=2;
									}
								}
								break;
							case '0': //decimal char reading
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
									char dc2=i<size?s[i]:'\0';
									i++;
									char dc3=i<size?s[i]:'\0';
									i++;
									
									if(char_is_dec(nc) && char_is_dec(dc2) && char_is_dec(dc3)){
										advance_position_from_char(dc2, nc, &col_num, &line_num);
										advance_position_from_char(dc3, dc2, &col_num, &line_num);
										append_char_to_compiler_token(cur_tok,read_dec_string_as_num(nc,dc2,dc3));
									}else if(char_is_hex(nc) && char_is_hex(dc2)){
										advance_position_from_char(dc2, nc, &col_num, &line_num);
										append_char_to_compiler_token(cur_tok,read_dec_string_as_num('0',nc,dc2));
										i--;
									}else{
										append_char_to_compiler_token(cur_tok, read_dec_string_as_num('0','0',nc));
										i-=2;
									}
									
								}
								break;
							default: //UB: if we can't recognize the escape, just give the two chars themselves.
								append_char_to_compiler_token(cur_tok,'\\');
								append_char_to_compiler_token(cur_tok,nc);
						}
						
					}else{ //add char to string.
						append_char_to_compiler_token(cur_tok,c);
						i++;
					}
					break;
				case TOKEN_TYPE_NUMBER:
					if(!char_is_number(c) && c!='.' && c!='x' && !char_is_hex(c) ){ //for decimal and hex
						if(add_token_or_destroy_array(&out,num_out,cur_tok))return nullptr;
						cur_tok=new_compiler_token(col_num, line_num);
					}else{
						append_char_to_compiler_token(cur_tok,c);
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}
					break;
				case TOKEN_TYPE_OPERATOR:
					if(c=='/' && c_prev=='/'){
						cur_tok->num_chars--; //remove the previous /
						comment_mode=COMMENT_LINE;
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}else if(c=='*' && c_prev=='/'){
						cur_tok->num_chars--; //remove the previous /
						comment_mode=COMMENT_MULTI;
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}else if(char_is_symbol(c) && symbol_can_merge_token(cur_tok,c) ){
						append_char_to_compiler_token(cur_tok,c);
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}else{
						if(add_token_or_destroy_array(&out,num_out,cur_tok))return nullptr;
						cur_tok=new_compiler_token(col_num, line_num);
					}
					break;
				case TOKEN_TYPE_KEYWORD:
					if(char_is_character(c) || char_is_number(c) ){
						append_char_to_compiler_token(cur_tok,c);
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}else{
						if(add_token_or_destroy_array(&out,num_out,cur_tok))return nullptr;
						cur_tok=new_compiler_token(col_num, line_num);
					}
					break;
				default:
					if(char_is_character(c)){
						cur_tok->token_type=TOKEN_TYPE_KEYWORD;
					}else if(char_is_number(c)){
						cur_tok->token_type=TOKEN_TYPE_NUMBER;
					}else if(char_is_symbol(c)){
						if(c=='\"'){
							cur_tok->token_type=TOKEN_TYPE_STRING;
							advance_position_from_char(c, c_prev, &col_num, &line_num);
							i++;
						}else{
							cur_tok->token_type=TOKEN_TYPE_OPERATOR;
						}
					}else{
						advance_position_from_char(c,c_prev,&col_num,&line_num);
						i++;
					}
			}
		}
		
	}
	if (cur_tok->num_chars) {
		if (add_token_or_destroy_array(&out, num_out, cur_tok))return nullptr;
	}

	//tokens extracted from string. now we need to run a processing step to get more useful data out of it.
	for(mercury_int i=0;i<(const mercury_int)*num_out;i++){
		compiler_token* t = out[i];
		switch(t->token_type){
			case TOKEN_TYPE_STRING:
				t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_STRING;
				break;
			case TOKEN_TYPE_NUMBER:
				t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_NUMBER;
				break;
			case TOKEN_TYPE_KEYWORD:
				if(t->num_chars==2){
					if(token_matches_chars(t,"if")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"do")){
						t->token_flags = TOKEN_KEYWORD;
					}
				}else if(t->num_chars==3){
					if(token_matches_chars(t,"nil")){
						t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_NIL;
					}else if(token_matches_chars(t,"end")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"inf")){
						t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_NUMBER;
					}
				}else if(t->num_chars==4){
					if(token_matches_chars(t,"then")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"true")){
						t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_BOOLEAN;
					}else if(token_matches_chars(t,"else")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"goto")){
						t->token_flags = TOKEN_KEYWORD;
					}
				}else if(t->num_chars==5){
					if(token_matches_chars(t,"false")){
						t->token_flags = TOKEN_VARIABLE | TOKEN_STATIC_BOOLEAN;
					}else if(token_matches_chars(t,"while")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"local")){
						t->token_flags = TOKEN_SCOPESPECIFIER;
					}else if (token_matches_chars(t, "break")) {
						t->token_flags = TOKEN_LOOP_MODIFIER;
					}
				}else if(t->num_chars==6){
					if(token_matches_chars(t,"global")){
						t->token_flags = TOKEN_SCOPESPECIFIER;
					}else if(token_matches_chars(t,"elseif")){
						t->token_flags = TOKEN_KEYWORD;
					}else if(token_matches_chars(t,"return")){
						t->token_flags = TOKEN_KEYWORD;
					}
				}else if(t->num_chars==8){
					if(token_matches_chars(t,"function")){
						t->token_flags = TOKEN_KEYWORD | TOKEN_VARIABLE;
					}else if(token_matches_chars(t,"continue")){
						t->token_flags = TOKEN_LOOP_MODIFIER;
					}
				}
				if(!t->token_flags)t->token_flags = TOKEN_VARIABLE | TOKEN_ENV_VAR;
				break;
			case TOKEN_TYPE_OPERATOR:
				if(t->num_chars==1){
					switch(t->chars[0]){
						case '=':
							t->token_flags = TOKEN_OPERATOR | TOKEN_MISC_OP;
							break;
						case '-':
							t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_UNARY_OP; //unary minus, too
							break;
						case '+':
						case '*':
						case '/':
						case '^':
						case '%':
						case '\\':
						case '&':
						case '|':
						case '~':
						case '>':
						case '<':
							t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_TAKES_STATIC_NUM;
							break;
						case '!':
							t->token_flags = TOKEN_OPERATOR | TOKEN_UNARY_OP | TOKEN_TAKES_STATIC_NUM;
							break;
						case '#':
							t->token_flags = TOKEN_OPERATOR | TOKEN_UNARY_OP;
							break;
						case '[':
						case '{':
							t->token_flags = TOKEN_OPERATOR | TOKEN_MISC_OP | TOKEN_VARIABLE;
							break;
						case '(':
						case ')':
						case '}':
						case ']':
						case ':':
						case '.':
						case ',':
							t->token_flags = TOKEN_OPERATOR | TOKEN_MISC_OP;
							break;
					}
				}else if(t->num_chars==2){
					switch(t->chars[0]){
						case '&':
						case '~':
						case '|':
							if(t->chars[0]==t->chars[1]){
								t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP;
								break;
							}
						case '+':
						case '-':
							if(t->chars[0]==t->chars[1]){
								t->token_flags = TOKEN_OPERATOR | TOKEN_TAKES_STATIC_NUM | TOKEN_UNARY_OP | TOKEN_SELFMODIFY_OP;
								break;
							}
						case '%':
						case '/':
						case '*':
						case '\\':
						case '^':
							if(t->chars[1]=='='){
								t->token_flags = TOKEN_OPERATOR | TOKEN_TAKES_STATIC_NUM | TOKEN_BINARY_OP | TOKEN_SELFMODIFY_OP;
							}
							break;
						case '!':
							if(t->chars[1]=='!'){
								t->token_flags = TOKEN_OPERATOR | TOKEN_UNARY_OP;
							}else if(t->chars[1]=='='){
								t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP;
							}
							break;
						case '>':
						case '<':
							if(t->chars[0]==t->chars[1] || t->chars[1]=='='){
								t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_TAKES_STATIC_NUM;
							}
							break;
						case '.':
							if(t->chars[1]=='.'){
								t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP;
							}
							break;
						case '=':
							if(t->chars[1]=='='){
								t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP;
							}
							break;
					}
				}else if(t->num_chars==3){
					if(token_matches_chars(t,"..=")){
						t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_SELFMODIFY_OP;
					} else if(token_matches_chars(t,"<<=")){
						t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_SELFMODIFY_OP;
					}else if(token_matches_chars(t,">>=")){
						t->token_flags = TOKEN_OPERATOR | TOKEN_BINARY_OP | TOKEN_SELFMODIFY_OP;
					}
				}
				break;
		}
	}
	return out;
}
//end of tokenization code, probably.

struct compiler_info {
	mercury_int additional_instructions = 0;

	//for while loops
	bool in_loop = false;
	mercury_int loop_jumppoint_start = 0; //used to track where continue and loop end will bring you to
	mercury_int loop_endpoint_ref_count = 0; //tracks the number of below refs
	mercury_int* loop_endpoint_ref_pos = nullptr; //where each break point is. filled in once a loop is compiled so that break will skip to after the loop ends.

	//for if thens
	bool in_conditional = false;
	mercury_int conditional_jumppoint_init = 0; // for if statments, stores the position JRNI distance of the last conditional check, so that it skips to the start of the next elseif/else block.
	mercury_int conditional_jumptoend_count = 0; //these are placed at the end of every elseif/if block to jump to the end so that other blocks are not called when we already executed one.
	mercury_int* conditional_jumptoend_points = nullptr;
	bool conditional_defaultcase = false;

	//constant strings
	mercury_int num_constants = 0;
	mercury_stringliteral* constants = nullptr;

	//goto compiling. this is made difficult because we can have situations where we write goto before the label is defined. because the compiler is single pass, this is made a bit tricky, but not impossible. eg, goto a a:
	mercury_int num_gotos = 0; //how many different goto labels are being used. eg, 2
	mercury_int** goto_positions = nullptr; //where the goto keywords are in instruction space. eg, [1], [55,43,32]
	mercury_int* goto_pos_count = nullptr; // how many gotos are present. eg, 1,3
	bool* goto_defined = nullptr; //if the label is defined yet. eg, 0,1 
	mercury_stringliteral* goto_alias = nullptr; //the string of the goto label. eg, "stuff","cancel"
	mercury_int* goto_labelpos = nullptr; //where the label is defined. eg, ?,50

};

inline compiler_info* new_compiler_info(){ //this exists to be used in m_compile_read_keyword_block and mercury_compile_tokens_to_bytecode, and whatnot to dictate keywords and control flow. doesn't need to be propagated to low-end functions like m_compile_read_variable or m_compile_read_statment.
//haha just kidding. that was the plan, but, well... programming. turns out constant statements need that propagation so... oh well. turns out a compiler with any amount of good features needs to do that, i guess. at least goto should be less compilated... right?
	compiler_info* out=(compiler_info*)malloc(sizeof(compiler_info));
	if(!out)return out;

	out->additional_instructions=0;
	
	out->in_loop=false;
	out->loop_jumppoint_start=0;
	out->loop_endpoint_ref_count=0;
	out->loop_endpoint_ref_pos=nullptr;

	out->in_conditional=false;
	out->conditional_jumppoint_init=0;
	out->conditional_jumptoend_count=0;
	out->conditional_jumptoend_points=nullptr;
	out->conditional_defaultcase=false;
	
	out->num_constants=0;
	out->constants=nullptr;
	
	out->num_gotos=0;
	
	out->goto_positions=nullptr;
	out->goto_pos_count=nullptr;
	out->goto_defined=nullptr;
	out->goto_alias=nullptr;
	out->goto_labelpos=nullptr;
	
	return out;
}

inline void delete_compiler_info(compiler_info* i){
	free(i->loop_endpoint_ref_pos);
	free(i->conditional_jumptoend_points);
	free(i->constants);
	for(mercury_int n=0;n<i->num_gotos;n++){
		free(i->goto_positions[n]);
	}
	free(i->goto_pos_count);
	free(i->goto_positions);
	free(i->goto_defined);
	free(i->goto_alias);
	free(i->goto_labelpos);	
	free(i);
}

inline void propogate_compiler_info(compiler_info* i_upstream, compiler_info* i_downstream) {
	i_upstream->constants = i_downstream->constants;
	i_upstream->num_constants = i_downstream->num_constants;
	i_upstream->num_gotos = i_downstream->num_gotos;
	i_upstream->goto_positions = i_downstream->goto_positions;
	i_upstream->goto_pos_count = i_downstream->goto_pos_count;
	i_upstream->goto_defined = i_downstream->goto_defined;
	i_upstream->goto_alias = i_downstream->goto_alias;
	i_upstream->goto_labelpos = i_downstream->goto_labelpos;
}

inline bool compiler_info_add_loop_endjump(compiler_info* i,mercury_int instruction_point){
	mercury_int* n=(mercury_int*)realloc(i->loop_endpoint_ref_pos,sizeof(mercury_int)*(i->loop_endpoint_ref_count+1) );
	if(!n)return false;
	n[i->loop_endpoint_ref_count] = instruction_point;
	i->loop_endpoint_ref_pos=n;
	i->loop_endpoint_ref_count++;
	return true;
}

inline bool compiler_info_add_conditional_endjump(compiler_info* i,mercury_int instruction_point){
	mercury_int* n=(mercury_int*)realloc(i->conditional_jumptoend_points,sizeof(mercury_int)*(i->conditional_jumptoend_count+1) );
	if(!n)return false;
	n[i->conditional_jumptoend_count] = instruction_point;
	i->conditional_jumptoend_points=n;
	i->conditional_jumptoend_count++;
	return true;
}

inline mercury_int compiler_info_add_constant_string_from_token(compiler_info* i,compiler_token* t){
	for(mercury_int n=0;n<i->num_constants;n++){
		mercury_stringliteral s=i->constants[n];
		if (s.size!=t->num_chars)continue;
		for(mercury_int c=0;c<s.size;c++){
			if(s.ptr[c]!=t->chars[c])goto exit;
		}
		return n;
		exit:;
	}
	
	mercury_stringliteral* nptr= (mercury_stringliteral*)realloc(i->constants,sizeof(mercury_stringliteral)*(i->num_constants+1) );
	if(!nptr)return -1; //surely nobody will use 4294967295 / 18446744073709551615 constants in their code, right?
	i->constants=nptr;
	mercury_stringliteral* nstr=i->constants+i->num_constants;
	nstr->size = t->num_chars;
	nstr->ptr = t->chars;
	i->num_constants++;
	return i->num_constants-1;
}

inline bool compiler_info_add_goto_jump(compiler_info* i,compiler_token* t,mercury_int instruction_point){
	void* nptr;
	
	for(mercury_int n=0;n<i->num_gotos;n++){
		mercury_stringliteral s=i->goto_alias[n];
		if(s.size!=t->num_chars)continue;
		for(mercury_int c=0;c<t->num_chars;c++){
			if(t->chars[c]!=s.ptr[c])goto next;
		}
		
		nptr=realloc(i->goto_positions[n],sizeof(mercury_int)*(i->goto_pos_count[n]+1) );
		if(!nptr)return false;
		i->goto_positions[n] = (mercury_int*)nptr;
		i->goto_positions[n][i->goto_pos_count[n]]=instruction_point;
		i->goto_pos_count[n]++;
		return true;
		
		next:;
	}
	
	nptr=realloc(i->goto_positions, sizeof(mercury_int*)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_positions=(mercury_int**)nptr;

	nptr=realloc(i->goto_pos_count, sizeof(mercury_int)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_pos_count=(mercury_int*)nptr;	
	
	nptr=realloc(i->goto_defined, sizeof(bool)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_defined=(bool*)nptr;	
	
	nptr=realloc(i->goto_alias, sizeof(mercury_stringliteral)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_alias=(mercury_stringliteral*)nptr;
	
	nptr=realloc(i->goto_labelpos, sizeof(mercury_int)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_labelpos=(mercury_int*)nptr;	
	
	nptr=malloc(sizeof(mercury_int));
	if(!nptr)return false;
	i->goto_positions[i->num_gotos]=(mercury_int*)nptr;
	i->goto_positions[i->num_gotos][0] = instruction_point;
	i->goto_pos_count[i->num_gotos]=1;
	i->goto_defined[i->num_gotos]=false;
	mercury_stringliteral s;
	s.ptr=t->chars;
	s.size=t->num_chars;
	i->goto_alias[i->num_gotos]=s;
	i->goto_labelpos[i->num_gotos]=0;
	
	i->num_gotos++;
	return true;
}


inline bool compiler_info_add_goto_label(compiler_info* i,compiler_token* t,mercury_int instruction_point){
	void* nptr;

	for(mercury_int n=0;n<i->num_gotos;n++){
		mercury_stringliteral s=i->goto_alias[n];
		if(s.size!=t->num_chars)continue;
		for(mercury_int c=0;c<t->num_chars;c++){
			if(t->chars[c]!=s.ptr[c])goto next;
		}

		i->goto_defined[n]=true;
		i->goto_labelpos[n]=instruction_point;
		return true;
		
		next:;
	}
	
	nptr=realloc(i->goto_positions, sizeof(mercury_int*)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_positions=(mercury_int**)nptr;

	nptr=realloc(i->goto_pos_count, sizeof(mercury_int)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_pos_count=(mercury_int*)nptr;	
	
	nptr=realloc(i->goto_defined, sizeof(bool)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_defined=(bool*)nptr;	
	
	nptr=realloc(i->goto_alias, sizeof(mercury_stringliteral)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_alias=(mercury_stringliteral*)nptr;
	
	nptr=realloc(i->goto_labelpos, sizeof(mercury_int)*(i->num_gotos+1));
	if(!nptr)return false;
	i->goto_labelpos=(mercury_int*)nptr;
	
	i->goto_positions[i->num_gotos]=nullptr;
	i->goto_pos_count[i->num_gotos]=0;
	i->goto_defined[i->num_gotos]=true;
	mercury_stringliteral s;
	s.ptr=t->chars;
	s.size=t->num_chars;
	i->goto_alias[i->num_gotos]=s;
	i->goto_labelpos[i->num_gotos]=instruction_point;
	
	i->num_gotos++;
	return true;
}


inline compiler_function* new_compiler_function(){
	compiler_function* out = (compiler_function*)malloc(sizeof(compiler_function));
	if(out){
		out->instructions=nullptr;
		out->number_instructions=0;
		out->instruction_tokens=nullptr;
		out->tokens_consumed = 0;
		out->token_error_num = 0;
		out->errorcode = 0;
	}
	return out;
}

inline void delete_compiler_function(compiler_function* f){
	if(f->instructions)free(f->instructions);
	if(f->instruction_tokens)free(f->instruction_tokens);
	free(f);
}




//appends the bytecode of fadd to fbase, and frees fadd.
inline bool merge_compiler_functions(compiler_function* fbase, compiler_function* fadd){
	mercury_int nsiz=fbase->number_instructions + fadd->number_instructions;
	
	void* p=realloc(fbase->instructions,sizeof(mercury_fullinstruction)*(nsiz) );
	if(!p)return false;
	fbase->instructions = (mercury_fullinstruction*)p;
	p=realloc(fbase->instruction_tokens,sizeof(mercury_int)*(nsiz) );
	if(!p)return false;
	fbase->instruction_tokens = (mercury_int*)p;
	
	memcpy(fbase->instructions+fbase->number_instructions,fadd->instructions,sizeof(mercury_fullinstruction)*fadd->number_instructions );
	memcpy(fbase->instruction_tokens+fbase->number_instructions,fadd->instruction_tokens,sizeof(mercury_int)*fadd->number_instructions );
	
	fbase->number_instructions = nsiz;
	if (fadd->errorcode && !fbase->errorcode) {
		fbase->errorcode = fadd->errorcode;
		fbase->token_error_num = fadd->token_error_num;
	}
	delete_compiler_function(fadd);
	return true;
}

inline bool add_instruction(compiler_function* f, mercury_opcode opcode, mercury_insflags flags,mercury_int token_num){
	void* p=realloc(f->instructions,sizeof(mercury_fullinstruction)*(f->number_instructions+1) );
	if(!p)return false;
	f->instructions = (mercury_fullinstruction*)p;
	p=realloc(f->instruction_tokens,sizeof(mercury_int)*(f->number_instructions+1) );
	if(!p)return false;
	f->instruction_tokens = (mercury_int*)p;
	
	mercury_fullinstruction d;
	d.flags = flags;
	d.opcode = opcode;
	f->instructions[f->number_instructions] = d;
	f->instruction_tokens[f->number_instructions] = token_num;
	
	f->number_instructions++;
	return true;
}

inline bool add_instruction_at_begining(compiler_function* f, mercury_opcode opcode, mercury_insflags flags,mercury_int token_num){
	void* p=realloc(f->instructions,sizeof(mercury_fullinstruction)*(f->number_instructions+1) );
	if(!p)return false;
	f->instructions = (mercury_fullinstruction*)p;
	p=realloc(f->instruction_tokens,sizeof(mercury_int)*(f->number_instructions+1) );
	if(!p)return false;
	f->instruction_tokens = (mercury_int*)p;
	
	memmove(f->instructions+1,f->instructions, sizeof(mercury_fullinstruction)*f->number_instructions );
	memmove(f->instruction_tokens+1,f->instruction_tokens, sizeof(mercury_int)*f->number_instructions );
	
	mercury_fullinstruction d;
	d.flags = flags;
	d.opcode = opcode;
	f->instructions[0] = d;
	f->instruction_tokens[0] = token_num;
	
	f->number_instructions++;
	return true;
}

inline bool add_rawdata(compiler_function* f, uint32_t data,mercury_int token_num){
	const int sizereq = sizeof(uint32_t) / sizeof(mercury_fullinstruction);
	void* p=realloc(f->instructions,sizeof(mercury_fullinstruction)*(f->number_instructions+ sizereq) );
	if(!p)return false;
	f->instructions = (mercury_fullinstruction*)p;
	p=realloc(f->instruction_tokens,sizeof(mercury_int)*(f->number_instructions+ sizereq) );
	if(!p)return false;
	f->instruction_tokens = (mercury_int*)p;
	
	*(uint32_t*)(f->instructions+f->number_instructions) = data;
	for (int n = 0; n < sizereq; n++) {
		f->instruction_tokens[f->number_instructions+n] = token_num;
	}
	
	f->number_instructions+= sizereq;
	return true;
}

#ifdef MERCURY_64BIT
inline bool add_rawdata_double(compiler_function* f, uint64_t data,mercury_int token_num){
	const int sizereq = sizeof(uint64_t) / sizeof(mercury_fullinstruction);
	void* p=realloc(f->instructions,sizeof(mercury_fullinstruction)*(f->number_instructions+ sizereq) );
	if(!p)return false;
	f->instructions = (mercury_fullinstruction*)p;
	p=realloc(f->instruction_tokens,sizeof(mercury_int)*(f->number_instructions+ sizereq) );
	if(!p)return false;
	f->instruction_tokens = (mercury_int*)p;
	
	*(uint64_t*)(f->instructions+f->number_instructions) = data;
	for (int n = 0; n < sizereq; n++) {
		f->instruction_tokens[f->number_instructions + n] = token_num;
	}
	
	f->number_instructions+= sizereq;
	return true;
}
#endif


#ifdef MERCURY_64BIT
typedef uint64_t binarydata;
inline bool add_rawdata_bitwidth_size(compiler_function* f, uint64_t data,mercury_int token_num){
	return add_rawdata_double(f, data, token_num);
}
#else
typedef uint32_t binarydata;
inline bool add_rawdata_bitwidth_size(compiler_function* f, uint32_t data,mercury_int token_num){
	return add_rawdata(f, data, token_num);
}
#endif

inline compiler_token* m_compile_get_next_token(compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset){
	compiler_token* out=nullptr;
	if( token_offset < num_tokens && token_offset>=0 ){ //OOB
		out=tokens[token_offset];
	}
	return out;
}

mercury_opcode m_compile_get_operator_opcode_from_token(compiler_token* t,int ttype=0){
	mercury_int nc = t->num_chars;
	if(nc==1){
		char c=t->chars[0];
		switch (c){
			case '+':
				return M_OPCODE_ADD;
			case '-':
				return ttype==1 ? M_OPCODE_UNM : M_OPCODE_SUB;
			case '*':
				return M_OPCODE_MUL;
			case '^':
				return M_OPCODE_POW;
			case '/':
				return M_OPCODE_DIV;
			case '\\':
				return M_OPCODE_IDIV;
			case '%':
				return M_OPCODE_MOD;
			case '!':
				return M_OPCODE_BNOT;
			case '&':
				return M_OPCODE_BAND;
			case '|':
				return M_OPCODE_BOR;
			case '~':
				return M_OPCODE_BXOR;
			case '#':
				return M_OPCODE_LEN;
			case '<':
				return M_OPCODE_LET;
			case '>':
				return M_OPCODE_GRT;
		}
	}else if(nc==2){
		if(t->chars[0]==t->chars[1]){
			switch(t->chars[0]){
				case '=':
					return M_OPCODE_EQL;
				case '.':
					return M_OPCODE_CNCT;	
				case '<':
					return M_OPCODE_BSHL;
				case '>':
					return M_OPCODE_BSHR;
				case '!':
					return M_OPCODE_LNOT;
				case '&':
					return M_OPCODE_LAND;
				case '|':
					return M_OPCODE_LOR;
				case '~':
					return M_OPCODE_LXOR;
				case '+':
					return M_OPCODE_INC;
				case '-':
					return M_OPCODE_DEC;
			}
		}else if(t->chars[1]=='='){
			switch(t->chars[0]){
				case '!':
					return M_OPCODE_NEQ;
				case '<':
					return M_OPCODE_LTE;
				case '>':
					return M_OPCODE_GTE;
				case '&':
					return M_OPCODE_BAND;
				case '|':
					return M_OPCODE_BOR;
				case '~':
					return M_OPCODE_BXOR;
				case '+':
					return M_OPCODE_ADD;
				case '-':
					return M_OPCODE_SUB;
				case '*':
					return M_OPCODE_MUL;
				case '^':
					return M_OPCODE_POW;
				case '\\':
					return M_OPCODE_IDIV;
				case '/':
					return M_OPCODE_DIV;
				case '%':
					return M_OPCODE_MOD;
			}
		}
	}else if(nc==3){
		if(token_matches_chars(t,"..=")){
			return M_OPCODE_CNCT;
		}else if(token_matches_chars(t,"<<=")){
			return M_OPCODE_BSHL;
		}else if(token_matches_chars(t,">>=")){
			return M_OPCODE_BSHR;
		}
	}
	return M_OPCODE_NOP;
}

inline void add_string_onto_stack(compiler_function* f,char* s,mercury_int len,mercury_int dbg_toknum){
	mercury_int required_space = (len + sizeof(mercury_fullinstruction)-1) / sizeof(mercury_fullinstruction);
	add_instruction(f, M_OPCODE_NSTR,0,dbg_toknum);
	add_rawdata_bitwidth_size(f, len,dbg_toknum);
	void* nptr=realloc(f->instructions, sizeof(mercury_fullinstruction)*(f->number_instructions+required_space) );
	if(!nptr){
		f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
		f->token_error_num=dbg_toknum;
		return;
	}
	f->instructions=(mercury_fullinstruction*)nptr;
	nptr=realloc(f->instruction_tokens, sizeof(mercury_int)*(f->number_instructions+required_space) );
	if(!nptr){
		f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
		f->token_error_num=dbg_toknum;
		return;
	}
	f->instruction_tokens=(mercury_int*)nptr;
	
	memcpy( f->instructions+f->number_instructions, s ,sizeof(char)*len );
	for(mercury_int i=0;i<required_space;i++){
		f->instruction_tokens[f->number_instructions+i]=dbg_toknum;
	}
	f->number_instructions+=required_space;
}




//forward refrences for other complex functions
mercury_int m_compile_read_var_statment(compiler_function* f, compiler_token** tokens, mercury_int num_tokens, mercury_int token_offset, compiler_info* i, compiler_function* postfix_function = nullptr);
compiler_function* mercury_compile_tokens_to_bytecode(compiler_token** tokens, mercury_int num_tokens, mercury_int token_offset = 0, mercury_int* tokens_used = nullptr, compiler_info* ci = nullptr);


mercury_int m_compile_read_variable(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* i){
	mercury_int initial_offset=token_offset;
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	//if(!(cur_tok &&(cur_tok->token_flags & (TOKEN_VARIABLE | TOKEN_SCOPESPECIFIER) ))) {
	if (!cur_tok){
		return 0;
	}
	if(cur_tok->token_flags & TOKEN_STATIC_NIL){
		add_instruction(f, M_OPCODE_NNIL,0,token_offset);
		token_offset++;
	}else if(cur_tok->token_flags & TOKEN_STATIC_BOOLEAN){
		if(cur_tok->chars[0]=='t'){
			add_instruction(f, M_OPCODE_NTRU,0,token_offset);
		}else{
			add_instruction(f, M_OPCODE_NFAL,0,token_offset);
		}
		token_offset++;
	}else if(cur_tok->token_flags & TOKEN_STATIC_STRING){
		mercury_int ci=compiler_info_add_constant_string_from_token(i,cur_tok);
		if (ci==-1){
			f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num=token_offset;
			return 0;
		}
		add_instruction(f,M_OPCODE_GCON,0, token_offset);
		add_rawdata_bitwidth_size(f,ci, token_offset);
		//add_string_onto_stack(f,cur_tok->chars,cur_tok->num_chars,token_offset);
		token_offset++;
	}else if(cur_tok->token_flags & TOKEN_STATIC_NUMBER){
		char* cstr=(char*)malloc(cur_tok->num_chars+1);
		if (!cstr) {
			f->errorcode = M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num = token_offset;
			return 0;
		}
		memcpy(cstr,cur_tok->chars,cur_tok->num_chars);
		cstr[cur_tok->num_chars]='\0';
		char* end;
		
		mercury_int i = strtoll(cstr, &end, 0);
		if (*end=='\0') {
			add_instruction(f, M_OPCODE_NINT,0,token_offset);
			add_rawdata_bitwidth_size(f, *((binarydata*)&i),token_offset);
		}else{
			mercury_float fa = strtod(cstr, &end);
			if (*end=='\0') {
				add_instruction(f, M_OPCODE_NFLO,0,token_offset);
				add_rawdata_bitwidth_size(f, *((binarydata*)&fa),token_offset);
			}else{
				f->errorcode=M_COMPERR_MALFORMED_NUMBER;
				f->token_error_num=token_offset;
				return 0;
			}
		}
		free(cstr);
		token_offset++;
	}else if( (cur_tok->token_flags & (TOKEN_OPERATOR | TOKEN_MISC_OP)) && cur_tok->num_chars==1 && (cur_tok->chars[0] == '[' || cur_tok->chars[0] == '{') ){
		
		bool is_array = cur_tok->chars[0]=='[';
		mercury_int implicit_position=0;
			
		add_instruction(f,is_array? M_OPCODE_NARR : M_OPCODE_NTAB,0,token_offset);
		token_offset++;

		while(true){ //the ticky thing about tables and arrays is how many syntax options we have. [1,2,3], [1=1,2=2,3=3]
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
				
				
			if(!cur_tok){
				break;
			}
				
			//exit at closing symbol.
			if( (cur_tok->token_flags & TOKEN_OPERATOR) && cur_tok->num_chars==1 && cur_tok->chars[0]== (is_array ? ']' : '}') ){
				token_offset++;
				break;
			}
				
			//read a variable. this could be implicit index value or an index.
			compiler_function* idxfunc=new_compiler_function();
			token_offset+= m_compile_read_var_statment(idxfunc,tokens,num_tokens,token_offset,i);
			if(idxfunc->errorcode){
				f->errorcode = idxfunc->errorcode;
				f->token_error_num = idxfunc->token_error_num;
				return 0;
			}
				
			//read the next token to find out.
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}
				
			if(cur_tok->token_flags & TOKEN_OPERATOR && cur_tok->num_chars==1){
				if(cur_tok->chars[0]=='='){ //explicit index
					add_instruction(f,M_OPCODE_CPYT,0,token_offset);
						
					merge_compiler_functions(f,idxfunc);
					idxfunc=nullptr;
					token_offset++;
					mercury_int o= m_compile_read_var_statment(f,tokens,num_tokens,token_offset,i);
						
					if(f->errorcode){
						return 0;
					}else if(!o){
						f->errorcode=M_COMPERR_NO_VARIABLE;
						f->token_error_num=token_offset;
						return 0;
					}
						
					add_instruction(f,M_OPCODE_SET,0,token_offset);
					token_offset += o;
					cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
						
					if(!cur_tok){
						f->errorcode=M_COMPERR_NO_MORE_TOKENS;
						f->token_error_num=token_offset;
						return 0;
					}
						
					if(!(cur_tok->token_flags & TOKEN_OPERATOR) && cur_tok->num_chars!=1 && !(cur_tok->chars[0]==',' || is_array ? ']' : '}')){
						f->errorcode=M_COMPERR_WRONG_SYMBOL;
						f->token_error_num=token_offset;
						return 0;
					}
					if(cur_tok->chars[0] == ',')token_offset++;
				}else if(cur_tok->chars[0]==',' || is_array ? ']' : '}'){ //implicit index
					add_instruction(f,M_OPCODE_CPYT,0,token_offset);
						
					add_instruction(f,M_OPCODE_NINT,0,token_offset);
					add_rawdata_double(f,*(binarydata*)&implicit_position,token_offset);
					implicit_position++;
						
					merge_compiler_functions(f,idxfunc);
					idxfunc=nullptr;
						
					add_instruction(f,M_OPCODE_SET,0,token_offset);
					if(cur_tok->chars[0] == ',')token_offset++;
				}else{
					f->errorcode=M_COMPERR_WRONG_SYMBOL;
					f->token_error_num=token_offset;
					return 0;
				}
			}else{
				f->errorcode=M_COMPERR_WRONG_SYMBOL;
				f->token_error_num=token_offset;
				return 0;
			}
				
		};
		
	}else if(cur_tok->token_flags & (TOKEN_ENV_VAR | TOKEN_SCOPESPECIFIER) ){
		mercury_opcode g_op= M_OPCODE_GENV;
		if(cur_tok->token_flags & TOKEN_SCOPESPECIFIER){
			if(token_matches_chars(cur_tok,"local")){
				g_op = M_OPCODE_GETL;
			}else if(token_matches_chars(cur_tok,"global")){
				g_op = M_OPCODE_GETG;
			}
			token_offset++;
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}else if(!(cur_tok->token_flags & TOKEN_ENV_VAR)){
				f->errorcode = M_COMPERR_NO_NAMED_VARIABLE;
				f->token_error_num=token_offset;
				return 0;
			}
			
			
		}
		
		mercury_int ci=compiler_info_add_constant_string_from_token(i,cur_tok);
		if (ci==-1){
			f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num=token_offset;
			return 0;
		}
		add_instruction(f,M_OPCODE_GCON,0, token_offset);
		add_rawdata_bitwidth_size(f,ci, token_offset);
		//add_string_onto_stack(f,cur_tok->chars,cur_tok->num_chars,token_offset);
		add_instruction(f, g_op,0,token_offset);
		token_offset++;
	}else if(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]=='('){ //parenthesis statement reading. we read (b+c) as a variable because it makes things easier on the compiler, and helps further segment the code to be more clean. after all, a+(b+c) is functionally equivalent to computing (b+c) then doing a+.
		token_offset++;

		token_offset+=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,i);
		if(f->errorcode){
			return 0;
		}
		
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode = M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if(!(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==')') ){
			f->errorcode = M_COMPERR_NESTEDSTATMENT_NOT_CLOSED;
			f->token_error_num=token_offset;
			return 0;
		}
		token_offset++;
		
	}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"function")){ //likewise, functions are a first class variable so there's no real reason to not include parsing it in the variable reading block.
		token_offset++;
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode=M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if( !(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]=='(') ){
			f->errorcode=M_COMPERR_WRONG_SYMBOL;
			f->token_error_num=token_offset;
			return 0;
		}
		
		add_instruction(f,M_OPCODE_NFUN,0,token_offset);
		
		//binarydata* sizeoffunction=(binarydata*)(f->instructions+f->number_instructions);
		
		add_rawdata_bitwidth_size(f,0,token_offset);

		mercury_int initial_bytecode_size=f->number_instructions;
		
		token_offset++;
		
		compiler_function* funcargreadf = new_compiler_function();

		while(true){ //reading args
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}else if(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==')'){
				break;
			}
			
			if(cur_tok->token_flags & TOKEN_ENV_VAR){

				add_string_onto_stack(funcargreadf,cur_tok->chars,cur_tok->num_chars,token_offset);
				//token_offset+=m_compile_read_variable(funcargreadf,tokens,num_tokens,token_offset, nullptr); //load the var name onto the stack
				add_instruction(funcargreadf,M_OPCODE_SWPT,0,token_offset); // we need the stack to be key, value. currently it's value 1, value 2, ..., key
				add_instruction(funcargreadf,M_OPCODE_SETL,0,token_offset);
				token_offset++;
			}else{
				f->errorcode=M_COMPERR_NO_NAMED_VARIABLE;
				f->token_error_num=token_offset;
				return 0;
			}
			
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}else if( !(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && (cur_tok->chars[0]==')' || cur_tok->chars[0]==',')  ) ){
				f->errorcode=M_COMPERR_WRONG_SYMBOL;
				f->token_error_num=token_offset;
				return 0;
			}
			if(cur_tok->chars[0] == ',')token_offset++;

		}
		
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode=M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if( !(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==')') ){
			f->errorcode=M_COMPERR_WRONG_SYMBOL;
			f->token_error_num=token_offset;
			return 0;
		}
		token_offset++;
		
		mercury_int toks_consumed=0;
		compiler_function* ff = mercury_compile_tokens_to_bytecode(tokens, num_tokens, token_offset,&toks_consumed);
		if (!ff) {
			f->errorcode = M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num = token_offset;
			return 0;
		}
		if(ff->errorcode){
			f->errorcode=ff->errorcode;
			f->token_error_num=ff->token_error_num;
			return 0;
		}
		token_offset += toks_consumed;
		merge_compiler_functions(f, funcargreadf);
		merge_compiler_functions(f,ff);
		add_instruction(f, M_OPCODE_EXIT, 0, token_offset);
		//*sizeoffunction = f->number_instructions-initial_bytecode_size;
		*(binarydata*)(f->instructions + initial_bytecode_size - MERCURY_INSTRUCTIONS_PER_VARIABLE_SIZE) = f->number_instructions - initial_bytecode_size;
		token_offset++; //advance 1 token to step over the ending end keyword
	}

	return token_offset- initial_offset;
}



mercury_int m_compile_read_unary_op(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset){
	mercury_int initial_offset=token_offset;
	while(true){
		compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			break;
		}
		if(cur_tok->token_flags & TOKEN_UNARY_OP && !(cur_tok->token_flags & TOKEN_SELFMODIFY_OP) ){
			add_instruction_at_begining(f, m_compile_get_operator_opcode_from_token(cur_tok,1) ,0,token_offset); // use _at_begining so that !#a is interpreted as not length a, instead of length not a.
			token_offset++;
		}else{
			break;
		}
	}
	return token_offset-initial_offset;
}

mercury_int m_compile_read_binary_op(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset){
	mercury_int initial_offset=token_offset;
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok){
		return 0;
	}else if(cur_tok->token_flags & TOKEN_BINARY_OP && !(cur_tok->token_flags & TOKEN_SELFMODIFY_OP) ){
		add_instruction(f,m_compile_get_operator_opcode_from_token(cur_tok),0,token_offset);
		token_offset++;
	}else{
		return 0;
	}
	
	return token_offset-initial_offset;
}


mercury_int m_compile_read_var_indexing(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* i){
	mercury_int initial_offset=token_offset;
	
	while(true){
		compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok || !(cur_tok->token_flags & TOKEN_MISC_OP) || cur_tok->num_chars!=1){ //soft break, since reading indexes is optional.
			break;
		}
		
		if(cur_tok->chars[0]=='.'){ //namespaced indexing
			token_offset++;
			compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}else if(!(cur_tok->token_flags & TOKEN_ENV_VAR)){
				f->errorcode=M_COMPERR_NO_NAMED_VARIABLE;
				f->token_error_num=token_offset;
				return 0;
			}else{
				token_offset+=m_compile_read_variable(f,tokens,num_tokens,token_offset,i);
				f->number_instructions--; //truncate the GENV instruction that will be added.
				add_instruction(f,M_OPCODE_GET,0,token_offset);
			}
		}else if(cur_tok->chars[0]=='['){ //specific indexing
			token_offset++;
			mercury_int o=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,i);
			if(f->errorcode){
				return 0;
			}else if(!o){
				f->errorcode = M_COMPERR_NO_VARIABLE;
				f->token_error_num=token_offset;
				return 0;
			}
			token_offset+=o;
			
			compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			if(!cur_tok){
				f->errorcode=M_COMPERR_NO_MORE_TOKENS;
				f->token_error_num=token_offset;
				return 0;
			}else if(!(cur_tok->token_flags&TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==']')){
				f->errorcode=M_COMPERR_WRONG_SYMBOL;
				f->token_error_num=token_offset;
				return 0;
			}
			
			add_instruction(f,M_OPCODE_GET,0,token_offset);
			token_offset++;
		}else{
			break;
		}
		
	}
	
	return token_offset-initial_offset;
}



mercury_int m_compile_read_call_func(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset, mercury_int* args_in,compiler_info* i){
	mercury_int initial_offset=token_offset;

	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok)return 0;
	if( !(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]=='(') ){
		return 0;
	}
	token_offset++;


	while(true){
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode=M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if( (cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==')') ){
			token_offset++;
			break;
		}
		
		mercury_int o=m_compile_read_var_statment(f,tokens,num_tokens, token_offset,i);
		if(f->errorcode){
			return 0;
		}else if(!o){
			f->errorcode=M_COMPERR_NO_VARIABLE;
			f->token_error_num=token_offset;
			return 0;
		}
		(*args_in)++;
		token_offset+=o;
		
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode=M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if( (cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && (cur_tok->chars[0]==')' || cur_tok->chars[0]==',') ) ){
			if(cur_tok->chars[0]==',')token_offset++;
		}else{
			f->errorcode=M_COMPERR_WRONG_SYMBOL;
			f->token_error_num=token_offset;
			return 0;
		}
		
	}


	return token_offset-initial_offset;
}

//a var statment is something like 5+5
mercury_int m_compile_read_var_statment(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* i,compiler_function* postfix_function){
	mercury_int initial_offset=token_offset;
	/*bytecode op order:
	1. get variable from environment
	2. index variable
	3. call function
	4. unary operation
	5. binary operation
	
	written op order:
	1. unary operation
	2. get variable from environment
	3. index variable
	4. call function
	5. binary operation*/
	
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	//token_offset++;
	if(!cur_tok){
		//f->errorcode=M_COMPERR_NO_MORE_TOKENS;
		//f->token_error_num=token_offset;
		return 0;
	}
	compiler_function* func_var = new_compiler_function();
	compiler_function* func_index = new_compiler_function();
	compiler_function* func_call = new_compiler_function();
	compiler_function* func_unary = new_compiler_function();
	compiler_function* func_binary = new_compiler_function();
	
	token_offset+=m_compile_read_unary_op(func_unary,tokens,num_tokens,token_offset);
	if(func_unary->errorcode){
		delete_compiler_function(func_var); delete_compiler_function(func_index); delete_compiler_function(func_call); delete_compiler_function(func_unary); delete_compiler_function(func_binary);
		f->errorcode=func_unary->errorcode;
		f->token_error_num=func_unary->token_error_num;
		return 0;
	}
	
	token_offset+=m_compile_read_variable(func_var,tokens,num_tokens,token_offset,i);
	if(func_var->errorcode){
		f->errorcode = func_var->errorcode;
		f->token_error_num = func_var->token_error_num;
		delete_compiler_function(func_var); delete_compiler_function(func_index); delete_compiler_function(func_call); delete_compiler_function(func_unary); delete_compiler_function(func_binary);
		return 0;
	}else if(!func_var->number_instructions){
		delete_compiler_function(func_var); delete_compiler_function(func_index); delete_compiler_function(func_call); delete_compiler_function(func_unary); delete_compiler_function(func_binary);
		f->errorcode=M_COMPERR_NO_VARIABLE;
		f->token_error_num=token_offset;
		return 0;
	}
	
	token_offset+= m_compile_read_var_indexing(func_index,tokens,num_tokens,token_offset,i);
	if(func_index->errorcode){
		delete_compiler_function(func_var); delete_compiler_function(func_index); delete_compiler_function(func_call); delete_compiler_function(func_unary); delete_compiler_function(func_binary);
		f->errorcode=func_index->errorcode;
		f->token_error_num=func_index->token_error_num;
		return 0;
	}
	
	mercury_int f_args_in=0;
	mercury_int fctokc=m_compile_read_call_func(func_call,tokens,num_tokens,token_offset,&f_args_in,i);
	token_offset+=fctokc;
	if(func_call->errorcode){
		delete_compiler_function(func_var); delete_compiler_function(func_index); delete_compiler_function(func_call); delete_compiler_function(func_unary); delete_compiler_function(func_binary);
		f->errorcode=func_call->errorcode;
		f->token_error_num=func_call->token_error_num;
		return 0;
	}

	
	compiler_function* func_complete = new_compiler_function();
	
	
	if(fctokc){
		merge_compiler_functions(func_complete,func_call);

		merge_compiler_functions(func_complete, func_var);
		merge_compiler_functions(func_complete, func_index);

		add_instruction(func_complete,M_OPCODE_CALL,0,token_offset-1);
		add_rawdata_bitwidth_size(func_complete, f_args_in,token_offset-1);
		add_rawdata_bitwidth_size(func_complete,1,token_offset-1); //args out
	}else{
		delete_compiler_function(func_call);

		merge_compiler_functions(func_complete, func_var);
		merge_compiler_functions(func_complete, func_index);
	}
	merge_compiler_functions(func_complete,func_unary);
	
	if(postfix_function)merge_compiler_functions(func_complete,postfix_function);
	
	
	token_offset+=m_compile_read_binary_op(func_binary,tokens,num_tokens,token_offset);
	if(func_binary->errorcode){
		delete_compiler_function(func_complete); delete_compiler_function(func_binary);
		f->errorcode=func_binary->errorcode;
		f->token_error_num=func_binary->token_error_num;
		return 0;
	}
	if(func_binary->number_instructions){ //if we registered a binary operator, then we need to read ahead to get the other statement, which means we get to do this whole thing again.
		token_offset+=m_compile_read_var_statment(func_complete,tokens,num_tokens,token_offset,i,func_binary);
	}else{
		delete_compiler_function(func_binary);
	}

	merge_compiler_functions(f, func_complete);
	
	return token_offset-initial_offset;
}


//a statement is something like a=5, a[1]=5, a(5), or a[1](5)
mercury_int m_compile_read_statment(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* i){
	mercury_int initial_offset=token_offset;
	mercury_opcode get_instruction=M_OPCODE_GENV;
	mercury_opcode set_instruction=M_OPCODE_SENV;
	
	//find local/global keyword first
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok){
		return 0;
	}else if(cur_tok->token_flags & TOKEN_SCOPESPECIFIER){
		
		if(token_matches_chars(cur_tok,"local")){
			get_instruction = M_OPCODE_GETL;
			set_instruction = M_OPCODE_SETL;
		}else{
			get_instruction = M_OPCODE_GETG;
			set_instruction = M_OPCODE_SETG;
		}
		
		token_offset++;
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->token_error_num=token_offset;
			f->errorcode = M_COMPERR_NO_MORE_TOKENS;
			return 0;
		}
	}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"return") ){
		//this is tricky, since we have to discern return a=5 and return a. one of these is a statement, while the other is a variable. why even compile code after return? well, goto exists, for one, and the cursed code that this will inevitably cause will be more than worth it. but why even allow that, you may ask. well, the compiler's job is to make code run, not pass judgments about how shitty your code is, so pass judgments we will not.
		//well actually, having said that, i've used jump after return in this code so uh... you go get that 1% speed improvment, king!
		token_offset++;
		while(true){
			compiler_function* n=new_compiler_function();
			mercury_int o=m_compile_read_var_statment(n,tokens,num_tokens,token_offset,i);
			cur_tok = m_compile_get_next_token(tokens, num_tokens, token_offset + o);
			if(!cur_tok || (cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[1]=='=')){
				delete_compiler_function(n);
				break;
			}
			if(n->errorcode){
				//f->errorcode=n->errorcode;
				//f->token_error_num=n->token_error_num;
				delete_compiler_function(n);
				break; //we want to break instead, because reading an arg is optional, and m_compile_read_var_statment will error if there's no arg.
			}
			merge_compiler_functions(f,n);
			token_offset+=o;
			if( !(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[1]==',')){
				break;
			}
		}
		add_instruction(f,M_OPCODE_EXIT,0,token_offset);
		return token_offset-initial_offset;
	}else if(cur_tok->token_flags & (TOKEN_KEYWORD | TOKEN_LOOP_MODIFIER)){
		return 0;
	}
	
	//then find the variable we'll be interfacing with
	if(!(cur_tok->token_flags & TOKEN_ENV_VAR)){
		f->token_error_num=token_offset;
		f->errorcode = M_COMPERR_NO_NAMED_VARIABLE;
		return 0;
	}
	
	compiler_token* nt=m_compile_get_next_token(tokens,num_tokens,token_offset+1); //goto labels
	if(get_instruction==M_OPCODE_GENV && nt && nt->token_flags & TOKEN_MISC_OP && nt->num_chars==1 && nt->chars[0]==':'){
		if(!compiler_info_add_goto_label(i,cur_tok,f->number_instructions+i->additional_instructions)){
			f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num=token_offset;
			return 0;
		}
		return 2;
	}
	
	compiler_function* getvarfunc=new_compiler_function();
	if (!getvarfunc) {
		f->errorcode = M_COMPERR_MEMORY_ALLOCATION;
		f->token_error_num = token_offset;
		return 0;
	}

	//token_offset+=m_compile_read_variable(f,tokens,num_tokens,token_offset,i);
	//*
	mercury_int ci = compiler_info_add_constant_string_from_token(i, cur_tok);
	if (ci == -1) {
		f->errorcode = M_COMPERR_MEMORY_ALLOCATION;
		f->token_error_num = token_offset;
		return 0;
	}
	add_instruction(getvarfunc, M_OPCODE_GCON, 0, token_offset);
	add_rawdata_bitwidth_size(getvarfunc, ci, token_offset);
	token_offset++;
	//*/

	//check if we're indexing something about it.
	while(true){
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->token_error_num=token_offset;
			f->errorcode = M_COMPERR_NO_MORE_TOKENS;
			delete_compiler_function(getvarfunc);
			return 0;
		}else if(cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && (cur_tok->chars[0]=='[' || cur_tok->chars[0]=='.') ){
			
			add_instruction(getvarfunc,get_instruction,0,token_offset);
			
			get_instruction=M_OPCODE_GET;
			set_instruction=M_OPCODE_SET;
			token_offset++;
			if(cur_tok->chars[0]=='['){ //explicit index
				token_offset+=m_compile_read_var_statment(getvarfunc,tokens,num_tokens,token_offset,i);
				cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
				if(!(cur_tok && cur_tok->token_flags & TOKEN_MISC_OP && cur_tok->num_chars==1 && cur_tok->chars[0]==']' ) ){
					f->token_error_num=token_offset;
					f->errorcode = M_COMPERR_WRONG_SYMBOL;
					delete_compiler_function(getvarfunc);
					return 0;
				}
				token_offset++;
			}else{ //implicit index
				cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
				if( !(cur_tok) ){
					f->token_error_num=token_offset;
					f->errorcode = M_COMPERR_NO_MORE_TOKENS;
					delete_compiler_function(getvarfunc);
					return 0;
				}else if( !(cur_tok->token_flags & TOKEN_ENV_VAR) ){
					f->token_error_num=token_offset;
					f->errorcode = M_COMPERR_NO_NAMED_VARIABLE;
					delete_compiler_function(getvarfunc);
					return 0;
				}
				token_offset+=m_compile_read_variable(getvarfunc,tokens,num_tokens,token_offset,i);
				getvarfunc->number_instructions--; //remove the last instruction, which would be GENV. hacky, i know.
			}
			
		}else{
			break;
		}
		
		
	}
	//then check what we're doing with this. this is either a set operation, void function call, or a self-modifying set operation
	
 	cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok){
		f->token_error_num=token_offset;
		f->errorcode = M_COMPERR_NO_MORE_TOKENS;
		delete_compiler_function(getvarfunc);
		return 0;
	}else if( !(cur_tok->token_flags & (TOKEN_MISC_OP | TOKEN_SELFMODIFY_OP)) ){
		f->token_error_num=token_offset;
		f->errorcode = M_COMPERR_WRONG_SYMBOL;
		delete_compiler_function(getvarfunc);
		return 0;
	}
	
	mercury_int ito = token_offset;
	if(token_matches_chars(cur_tok,"(")){ //function call
		mercury_int ain=0;
		add_instruction(getvarfunc,get_instruction,0,token_offset);
		token_offset+=m_compile_read_call_func(f,tokens,num_tokens,token_offset,&ain,i);
		merge_compiler_functions(f, getvarfunc);
		if(f->errorcode){
			return 0;
		}
		add_instruction(f,M_OPCODE_CALL,0, ito); //offset back by 1 to account for the closing symbol being used.
		add_rawdata_bitwidth_size(f,ain, ito);
		add_rawdata_bitwidth_size(f,0, ito);
	}else if(token_matches_chars(cur_tok,"=")){ //assignment
		token_offset++;
		merge_compiler_functions(f, getvarfunc);
		token_offset+=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,i);
		if(f->errorcode){
			return 0;
		}
		add_instruction(f,set_instruction,0, ito);
	}else if(cur_tok->token_flags & TOKEN_SELFMODIFY_OP){ //assignment & modification
		merge_compiler_functions(f, getvarfunc);
		// stack: table, key
		// we of course need to do both get and set in one line, so we have to get a bit tricky about the stack variables, so...
		if (get_instruction == M_OPCODE_GET) {
			add_instruction(f, M_OPCODE_CPYX, 0, ito);
			add_rawdata_bitwidth_size(f, 2, ito); 
		}
		else { //for enviromental vars, we only have the key on the stack, since the table is implicit
			add_instruction(f, M_OPCODE_CPYT, 0, ito);
		}	
		//stack: table, key, table, key
		add_instruction(f,get_instruction,0, ito);
		//stack: table, key, value
		//horray, we did it. well, actually no, we still need to read the rest of the statment.
		mercury_opcode insop=m_compile_get_operator_opcode_from_token(cur_tok);
		token_offset++;
		
		if(cur_tok->token_flags & TOKEN_BINARY_OP){
			token_offset+=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,i);
			if(f->errorcode){
				return 0;
			}
		}
		
		add_instruction(f,insop,0, ito);
		add_instruction(f, set_instruction, 0, ito);
	}else{
		delete_compiler_function(getvarfunc);
		f->token_error_num=token_offset;
		f->errorcode = M_COMPERR_NO_OPERATION_FOUND;
		return 0;
	}
	
	return token_offset-initial_offset;
}



mercury_int m_compile_parse_while_loop(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* ci){
	mercury_int initial_offset=token_offset;
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	
	compiler_info oci=*ci;
	
	
	if( !(cur_tok&& cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"while")) ){
		return 0;
	}
	token_offset++;
	
	
	ci->in_loop=true;
	ci->loop_jumppoint_start=f->number_instructions;
	ci->loop_endpoint_ref_count=0;
	ci->loop_endpoint_ref_pos=nullptr;
	
	token_offset+=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,ci);
	if(f->errorcode)return 0;
	
	add_instruction(f,M_OPCODE_JRNI,0,token_offset);
	compiler_info_add_loop_endjump(ci,f->number_instructions);
	add_rawdata_bitwidth_size(f,0,token_offset);
	
	cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok){
		f->errorcode = M_COMPERR_NO_MORE_TOKENS;
		f->token_error_num = token_offset;
		return 0;
	} else if( !(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"do")) ){
		f->errorcode = M_COMPERR_WHILE_NEEDS_DO;
		f->token_error_num = token_offset;
		return 0;
	}
	token_offset++;
	
	while(true){
		mercury_int consumed=0;
		ci->additional_instructions=oci.additional_instructions+f->number_instructions;
		compiler_function* bf=mercury_compile_tokens_to_bytecode(tokens,num_tokens,token_offset , &consumed,ci);
		if(bf->errorcode){
			f->errorcode=bf->errorcode;
			f->token_error_num=bf->token_error_num;
			return 0;
		}
		merge_compiler_functions(f,bf);
		token_offset+=consumed;
		
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode = M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num = token_offset;
			return 0;
		}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"end") ){
			token_offset++;
			break;
		}else{
			f->errorcode = M_COMPERR_WRONG_SYMBOL;
			f->token_error_num=token_offset;
			return 0;
		}
		
	}
	
	add_instruction(f, M_OPCODE_JMPR, 0, token_offset-1);
	add_rawdata_double(f, ci->loop_jumppoint_start-f->number_instructions,token_offset-1);

	for(mercury_int i=0;i<ci->loop_endpoint_ref_count;i++){
		*(mercury_int*)(f->instructions+ci->loop_endpoint_ref_pos[i]) =f->number_instructions-ci->loop_endpoint_ref_pos[i];
	}
	free(ci->loop_endpoint_ref_pos);
	

	oci.conditional_defaultcase = ci->conditional_defaultcase;
	oci.conditional_jumppoint_init = ci->conditional_jumppoint_init;
	oci.conditional_jumptoend_count = ci->conditional_jumptoend_count;
	oci.conditional_jumptoend_points= ci->conditional_jumptoend_points;
	propogate_compiler_info(&oci, ci);
	*ci=oci;
	
	return token_offset-initial_offset;
}

mercury_int m_compile_parse_if_statement(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* ci){
	mercury_int initial_offset=token_offset;
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	
	compiler_info oci=*ci;
	
	
	if( !(cur_tok&& cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"if")) ){
		return 0;
	}
	token_offset++;
	
	ci->in_conditional=true;
	ci->conditional_jumppoint_init=f->number_instructions;
	ci->conditional_jumptoend_count=0;
	ci->conditional_jumptoend_points=nullptr;
	ci->conditional_defaultcase=false;
	
	
	mercury_int o=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,ci);
	if(f->errorcode){
		return 0;
	}else if(!o){
		f->errorcode=M_COMPERR_NO_VARIABLE;
		f->token_error_num=token_offset;
		return 0;
	}
	token_offset+=o;
	
	cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if( !(cur_tok&& cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"then")) ){
		f->errorcode=M_COMPERR_IF_NEEDS_THEN;
		f->token_error_num=token_offset;
		return 0;
	}
	
	
	add_instruction(f,M_OPCODE_JRNI,0,token_offset);
	ci->conditional_jumppoint_init=f->number_instructions;
	add_rawdata_bitwidth_size(f,0,token_offset);
	token_offset++;


	while(true){
		mercury_int consumed=0;
		ci->additional_instructions=oci.additional_instructions+f->number_instructions;
		compiler_function* bf=mercury_compile_tokens_to_bytecode(tokens,num_tokens,token_offset , &consumed,ci);
		if(bf->errorcode){
			f->errorcode=bf->errorcode;
			f->token_error_num=bf->token_error_num;
			return 0;
		}
		merge_compiler_functions(f,bf);
		token_offset+=consumed;
		
		cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode = M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num = token_offset;
			return 0;
		}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"elseif") ){
			add_instruction(f,M_OPCODE_JMPR,0,token_offset);
			compiler_info_add_conditional_endjump(ci,f->number_instructions);
			add_rawdata_bitwidth_size(f,0,token_offset);
			token_offset++;

			*(mercury_int*)(f->instructions + ci->conditional_jumppoint_init) = f->number_instructions - ci->conditional_jumppoint_init;
			
			o=m_compile_read_var_statment(f,tokens,num_tokens,token_offset,ci);
			if(f->errorcode){
				return 0;
			}else if(!o){
				f->errorcode=M_COMPERR_NO_VARIABLE;
				f->token_error_num=token_offset;
				return 0;
			}
			token_offset+=o;
			
			add_instruction(f,M_OPCODE_JRNI,0,token_offset);
			ci->conditional_jumppoint_init=f->number_instructions;
			add_rawdata_bitwidth_size(f,0,token_offset);
			
			cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
			if( !(cur_tok&& cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"then")) ){
				f->errorcode=M_COMPERR_IF_NEEDS_THEN;
				f->token_error_num=token_offset;
				return 0;
			}
			token_offset++;
			
		}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"else") ){
			add_instruction(f, M_OPCODE_JMPR, 0, token_offset);
			compiler_info_add_conditional_endjump(ci, f->number_instructions);
			add_rawdata_bitwidth_size(f, 0, token_offset);

			token_offset++;
			*(mercury_int*)(f->instructions + ci->conditional_jumppoint_init) = f->number_instructions - ci->conditional_jumppoint_init;
			ci->conditional_defaultcase = true;
		}else if(cur_tok->token_flags & TOKEN_KEYWORD && token_matches_chars(cur_tok,"end") ){
			token_offset++;
			
			if(!ci->conditional_defaultcase){ //if we didn't fall through to else, we have to set the jump point of the last if check to here.
				*(mercury_int*)(f->instructions+ci->conditional_jumppoint_init) = f->number_instructions-ci->conditional_jumppoint_init;
			}
			
			for(mercury_int i=0;i<ci->conditional_jumptoend_count;i++){
				*(mercury_int*)(f->instructions+ci->conditional_jumptoend_points[i]) = f->number_instructions - ci->conditional_jumptoend_points[i];
			}
			
			break;
		}else{
			f->errorcode = M_COMPERR_WRONG_SYMBOL;
			f->token_error_num=token_offset;
			return 0;
		}
		
	}
	
	free(ci->conditional_jumptoend_points);

	//some data we need to propogate back upwards.
	propogate_compiler_info(&oci,ci);
	oci.loop_jumppoint_start = ci->loop_jumppoint_start;
	oci.loop_endpoint_ref_count = ci->loop_endpoint_ref_count;
	oci.loop_endpoint_ref_pos = ci->loop_endpoint_ref_pos;
	*ci=oci;

	return token_offset-initial_offset;
}


mercury_int m_compile_read_keyword_block(compiler_function* f, compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset,compiler_info* ci){
	mercury_int initial_offset=token_offset;
	
	compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
	if(!cur_tok){
		return 0;
	}else if(!(cur_tok->token_flags & TOKEN_KEYWORD) ){
		return 0;
	}
	
	if(token_matches_chars(cur_tok,"while")){
		token_offset+=m_compile_parse_while_loop(f,tokens,num_tokens,token_offset,ci);
	}else if(token_matches_chars(cur_tok,"if")){
		token_offset+=m_compile_parse_if_statement(f,tokens,num_tokens,token_offset,ci);
	}else if(token_matches_chars(cur_tok,"goto")){
		token_offset++;
		compiler_token* cur_tok=m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(!cur_tok){
			f->errorcode=M_COMPERR_NO_MORE_TOKENS;
			f->token_error_num=token_offset;
			return 0;
		}else if(!(cur_tok->token_flags & TOKEN_ENV_VAR) ){
			f->errorcode=M_COMPERR_NO_NAMED_VARIABLE;
			f->token_error_num=token_offset;
			return 0;
		}
		add_instruction(f,M_OPCODE_JMPR,0, token_offset);
		if(!compiler_info_add_goto_jump(ci,cur_tok,f->number_instructions+ci->additional_instructions)){
			f->errorcode=M_COMPERR_MEMORY_ALLOCATION;
			f->token_error_num=token_offset;
			return 0;
		}
		add_rawdata_bitwidth_size(f,0, token_offset);
		
		token_offset++;
	}else{
		return 0;
	}
	
	return token_offset-initial_offset;
}




compiler_function* mercury_compile_tokens_to_bytecode(compiler_token** tokens,mercury_int num_tokens,mercury_int token_offset, mercury_int* tokens_used, compiler_info* ci){
	compiler_function* out = new_compiler_function();
	if(!out)return nullptr;
	mercury_int initial_offset=token_offset;
	bool ci_was_defined=false;
	if(!ci){
		ci_was_defined=true;
		ci=new_compiler_info();
	}
	
	while(true){
		mercury_int o1=m_compile_read_statment(out,tokens,num_tokens,token_offset,ci);
		if(out->errorcode){
			break;
		}
		token_offset+=o1;
		mercury_int o2=m_compile_read_keyword_block(out,tokens,num_tokens,token_offset,ci);
		if(out->errorcode){
			break;
		}
		token_offset+=o2;
		
		mercury_int o3=0;
		compiler_token* cur_tok = m_compile_get_next_token(tokens,num_tokens,token_offset);
		if(cur_tok && cur_tok->token_flags & TOKEN_LOOP_MODIFIER){
			if(ci->in_loop){
				if(token_matches_chars(cur_tok,"continue")){
					add_instruction(out,M_OPCODE_JMPR,0,token_offset);
					add_rawdata_bitwidth_size(out, ci->loop_jumppoint_start-(ci->additional_instructions+out->number_instructions)  ,token_offset);
				}else{ //break
					add_instruction(out,M_OPCODE_JMPR,0,token_offset);
					if(!compiler_info_add_loop_endjump(ci,out->number_instructions+ci->additional_instructions)){
						out->errorcode=M_COMPERR_MEMORY_ALLOCATION;
						out->token_error_num=token_offset;
						break;
					}
					add_rawdata_bitwidth_size(out,0,token_offset);
				}
				token_offset++;
				o3=1;
			}else{
				out->errorcode=M_COMPERR_LOOPKEYWORD_WRONG_SPOT;
				out->token_error_num=token_offset;
				break;
			}
		}
		
		if(!o1 && !o2 && !o3){ //exit if no code was generated
			break;
		}
		
		
	}
	
	if(tokens_used)*tokens_used=token_offset-initial_offset;
	if(ci_was_defined){
		for(mercury_int i=0;i<ci->num_gotos;i++){
			if(ci->goto_defined[i]){
				for(mercury_int n=0;n<ci->goto_pos_count[i];n++){
					mercury_int p = ci->goto_positions[i][n];
					mercury_int* j = (mercury_int*)(out->instructions + p);
					*j=ci->goto_labelpos[i]-p;
				}
			}
		}
		compiler_function* con_app = new_compiler_function();
		for (mercury_int i = 0; i < ci->num_constants; i++) {
			mercury_stringliteral s = ci->constants[i];
			add_string_onto_stack(con_app, s.ptr, s.size, 0);
			add_instruction(con_app, M_OPCODE_SCON, 0, 0);
			add_rawdata_bitwidth_size(con_app, i, 0);
		}
		merge_compiler_functions(con_app, out);
		out = con_app;
		delete_compiler_info(ci);
	}
	return out;
}


mercury_variable* mercury_compile_mstring(mercury_stringliteral* str, bool remove_debug_info){
	mercury_variable* out=(mercury_variable*)malloc(sizeof(mercury_variable));
	if(!out)return nullptr;
	out->type = M_TYPE_NIL;
	out->data.i = 0;
	out->constant = 0;
	
	mercury_int num_tokens;
	compiler_token** tokens = mercury_compile_tokenize_mstring(str,&num_tokens);
	compiler_function* func = mercury_compile_tokens_to_bytecode(tokens,num_tokens,0);

	if (func->errorcode) {
		int req_chars;
		char* buffer=nullptr;

		char pbuf[512];

		if (func->token_error_num >= num_tokens) {
#ifdef MERCURY_DEBUG
			printf("DEBUG/error|compiler: error registered at token %i, but we only have %i avalible.\n", func->token_error_num, num_tokens);
#endif
			func->token_error_num = num_tokens - 1;
		}
		else if (func->token_error_num < 0) {
#ifdef MERCURY_DEBUG
			printf("DEBUG/error|compiler: error registered at token %i when the minimum is 0\n", func->token_error_num);
#endif
			func->token_error_num = 0;
		}

		compiler_token* etok = tokens[func->token_error_num];
		char* ts=(char*)malloc(etok->num_chars+1);
		if (!ts)return nullptr;
		memcpy(ts, etok->chars, etok->num_chars);
		ts[etok->num_chars]='\0';
#ifdef MERCURY_64BIT
		snprintf(pbuf, 510, "line %lli col %lli at \"%s\": ",etok->line_num,etok->line_col,ts);
#else
		snprintf(pbuf, 510, "line %i col %i at \"%s\": ", etok->line_num, etok->line_col,ts);
#endif
		const char* reason;

		switch (func->errorcode) {	
			case M_COMPERR_MEMORY_ALLOCATION:
				reason = "failed to allocate memory";
				break;
			case M_COMPERR_NO_MORE_TOKENS:
				reason = "no more tokens";
				break;
			case M_COMPERR_MALFORMED_NUMBER:
				reason = "malformed number";
				break;
			case M_COMPERR_WRONG_SYMBOL:
				reason = "wrong symbol";
				break;
			case M_COMPERR_NO_OPERATION_FOUND:
				reason = "no operation performed";
				break;
			case M_COMPERR_WHILE_NEEDS_DO:
				reason = "while requires 'do' keyword";
				break;
			case M_COMPERR_IF_NEEDS_THEN:
				reason = "if requires 'then' keyword";
				break;
			case M_COMPERR_LOOPKEYWORD_WRONG_SPOT:
				reason = "invalid location, must be in a loop";
				break;
			case M_COMPERR_ARRAY_NOT_CLOSED:
				reason = "array was not closed, expected ']'";
				break;
			case M_COMPERR_TABLE_NOT_CLOSED:
				reason = "table was not closed, expected '}'";
				break;
			case M_COMPERR_CALL_NOT_CLOSED:
				reason = "function was not closed, expected ')'";
				break;
			case M_COMPERR_NESTEDSTATMENT_NOT_CLOSED:
				reason = "parenthesis was not closed, expected ')'";
				break;
			case M_COMPERR_NO_VARIABLE:
				reason = "unable to find variable statment";
				break;
			case M_COMPERR_NO_NAMED_VARIABLE:
				reason = "unable to find named variable";
				break;
			default:
				reason = "unspecified error.";
		}

		req_chars = snprintf(nullptr, 0, "%s: %s.\n", pbuf, reason) + 1;
		buffer = (char*)malloc(req_chars);
		snprintf(buffer, req_chars, "%s%s", pbuf, reason);

		if (!buffer)return nullptr;

		out->type = M_TYPE_STRING;
		out->data.p = mercury_cstring_to_mstring(buffer,strlen(buffer));
	}
	else {
		mercury_function* nmf = (mercury_function*)malloc(sizeof(mercury_function));
		if (!nmf)return nullptr;


		mercury_debug_token* dts=nullptr;

		if (!remove_debug_info) {
			dts = (mercury_debug_token*)malloc(sizeof(mercury_debug_token) * func->number_instructions);
			if (!dts)return nullptr;

			for (mercury_int i = 0; i < func->number_instructions; i++) {
				mercury_int toknum = func->instruction_tokens[i];

				if (toknum >= num_tokens) {
#ifdef MERCURY_DEBUG
					printf("DEBUG/error|compiler: instruction #%i (%i/%x) was assigned by token %i, but we only have %i avalible.\n",i,func->instructions[i], func->instructions[i],toknum,num_tokens);
#endif
					toknum = num_tokens - 1;
				}

				compiler_token* t = tokens[toknum];
				mercury_debug_token d;
				d.col = t->line_col;
				d.line = t->line_num;
				d.chars = t->chars;
				d.num_chars = t->num_chars;
				*(dts + toknum) = d;
			}

			for (mercury_int i = 0; i < num_tokens; i++) {
				compiler_token* t = tokens[i];
				free(t);
			}
		}
		else {
			for (mercury_int i = 0; i < num_tokens; i++) {
				compiler_token* t = tokens[i];
				free(t->chars);
				free(t);
			}
		}
		
		free(tokens);

		nmf->enviromental = false;
		nmf->refrences = 1;
		nmf->instructions = func->instructions;
		nmf->numberofinstructions = func->number_instructions;
		nmf->debug_info = dts;
		
		free(func);

		out->data.p = nmf;
		out->type = M_TYPE_FUNCTION;
	}
	
	return out;
}
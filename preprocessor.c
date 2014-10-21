#include <stdlib.h>
#include <stdio.h>

#include "preprocessor.h"
#include "hash_table.h"
#include "errors.h"

int tokenizer_initialized = 0;
hash_table macro_symbol_table;
token *token_stream;
unsigned long num_tokens = 0;
unsigned long token_stream_size = 0;

void initialize() {
	macro_symbol_table = hash_table_create(MACRO_HASH_TABLE_SIZE);
	token_stream_size = INITIAL_TOKEN_STREAM_SIZE;
	token_stream = (token *)malloc(token_stream_size * sizeof(token));
	num_tokens = 0;
	tokenizer_initialized = 1;
}

void append_token(token tok) {
	token_stream[num_tokens++] = tok;
	if (num_tokens == token_stream_size) {
		token_stream_size *= 2;
		token_stream = realloc(token_stream, token_stream_size * sizeof(token));
	}
}

void tokenize(FILE *file) {
	int is_new_line = 1;
	token tok;
	if (!tokenizer_initialized) initialize();
	do {
		is_new_line = consume_whitespace(file) || is_new_line;
		tok = scan_token(file);
		while (is_new_line && strcmp(tok.text, "#") == 0) {
			parse_directive(file);
			consume_whitespace(file);
			tok = scan_token(file);
		}
		is_new_line = 0;
		if (tok.type == identifier) {
			macro *mac = hash_table_remove(macro_symbol_table, tok.text);
			if (mac) {
				expand_macro(file, *mac);
				hash_table_insert(macro_symbol_table, tok.text, mac);
			}
		} else if (tok.type != comment) {
			append_token(tok);
		}
	} while (tok.type != eof);
}

void parse_directive(FILE *file) {
	/* GCC seems perfectly content with empty preprocessor directives
	 * TODO : figure out if this is the correct treatment */
	token tok;
	if (consume_whitespace(file)) return;
	tok = scan_token(file);
	if (strcmp(tok.text, "include") == 0) {
		char *filepath;
		if (consume_whitespace(file)) {
			issue_error("Expected filename after include directive", tok.line, tok.col);
			return;
		}
		tok = scan_preprocessor_string(file);
		if (tok.text[0] == '\"') {
			filepath = (char *)malloc((strlen(QUOTE_PATH) + strlen(BRACKET_PATH) + 2) * sizeof(char));
			strcpy(filepath, QUOTE_PATH);
			strcat(filepath, ":");
			strcat(filepath, BRACKET_PATH);
		} else if (tok.text[0] == '<') {
			
		}
	}
}

int main(int argc, char *argv[]) {
	int i;
	tokenize(stdin);
	for (i = 0; i < num_tokens; ++i) {
		printf("%s\n", token_stream[i].text);
	}
}

/*
int main(int argc, char *argv[]) {
	
	token tok;
	while ((tok = next_token(stdin)).type != eof) {
		char *typeString;
		switch (tok.type) {
			case identifier:
				typeString = "Identifier";
				break;
			case number:
				typeString = "Number";
				break;
			case char_constant:
				typeString = "Character Constant";
				break;
			case string_literal:
				typeString = "String Literal";
				break;
			case symbol:
				typeString = "Symbol";
				break;
			case other:
				typeString = "Other";
				break;
			case eof:
				typeString = "End of File";
				break;
			default:
				typeString = "Unknown Token";
				break;
		}
		printf("%s : %s\n@%d:%d\n", tok.text, typeString, tok.line, tok.col);
	}
}


token *tokenize(FILE *file, int *count) {
	token tok;
	int buffer_size = 1024;
	int i = 0;
	token *token_buffer = (token *)malloc(buffer_size *  sizeof(token));
	do {
		tok = tok = next_token(file);
		if (i == buffer_size) {
			buffer_size *= 2;
			token_buffer = (token *)realloc(token_buffer, buffer_size * sizeof(token));
		}
		token_buffer[i++] = tok;
	} while (tok.type != eof);
	*count = i;
	return token_buffer;
}
*/




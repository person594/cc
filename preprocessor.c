#include <stdlib.h>

#include <tokenizer.h>


token next_token(FILE *file) {
	static int is_first_line = 1;
	int is_new_line;
	token tok;
	is_new_line = consume_line_whitespace(file) || is_first_line;
	is_first_line = 0;
	tok = next_token;
	
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




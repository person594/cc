#include <stdio.h> 
#include <string.h>

#include "errors.h"
#include "tokenizer.h"

int line = 1;
int col = 0;

int is_identifier_start(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_digit(char c) {
	return c >= '0' && c <= '9';
}

int is_identifier_char(char c) {
	return is_identifier_start(c) || is_digit(c);
}

int is_symbol_start(char c) {
	return strchr("{}[]().-+&|*~!/%<>=^?:,#;", c) ? 1 : 0; 
}


char next_ch(FILE *file) {
	int c; 
	char ch;
	/*
	 after reading a trigraph, we need to increment our column number
	 for the next char
	*/
	static int col_increment = 0;
	col += col_increment;
	col_increment = 0;
	
	c = getc(file);
	if (c < 0) return '\0';
	ch = (char) c;
	
	if (ch == '\n' || ch == '\v') {
		line++, col = 0;
	} else {
		col++;
	}
	
	if (ch == '\\') {
		c = getc(file);
		if (c == '\n') {
			line++, col = 0;
			return next_ch(file);
		} else {
			ungetc(c, file);
		}
	} else if (ch == '?') {
		c = getc(file);
		if (c == '?') {
			c = getc(file);
			col_increment = 2;
			switch (c) {
				case '=':
					return '#';
				case '/':
					return '\\';
				case '\'':
					return '^';
				case '(':
					return '[';
				case ')':
					return ']';
				case '!':
					return '|';
				case '<':
					return '{';
				case '>':
					return '}';
				case '-':
					return '~';
				default:
					ungetc(c, file);
					ungetc('?', file);
					col_increment = 0;
			}
		} else {
			ungetc(c, file);
		}
	}
	return ch;
}

char peek_ch(FILE *file) {
	int c; 
	char ch;
	
	c = getc(file);
	ch = (char) c;
	if (c < 0) {
		ch = '\0';
	} else if (c == '\\') {
		int c2;
		c2 = getc(file);
		if (c2 == '\n') {
			ch = peek_ch(file);
		}
		ungetc(c2, file);
	} else if (c == '?') {
		int c2;
		c2 = getc(file);
		if (c2 == '?') {
			int c3;
			c3 = getc(file);
			switch (c3) {
				case '=':
					ch = '#';
					break;
				case '/':
					ch = '\\';
					break;
				case '\'':
					ch = '^';
					break;
				case '(':
					ch = '[';
					break;
				case ')':
					ch = ']';
					break;
				case '!':
					ch = '|';
					break;
				case '<':
					ch = '{';
					break;
				case '>':
					ch = '}';
					break;
				case '-':
					ch = '~';
					break;
			}
			ungetc(c3, file);
		}
		ungetc(c2, file);
	}
	ungetc(c, file);
	return ch;
}

token scan_identifier(FILE *file) {
	int buffer_len = 32;
	int i = 0;
	token tok;
	tok.type = identifier;
	tok.text = (char *)malloc(buffer_len * sizeof(char));
	tok.text[i++] = next_ch(file);
	tok.line = line;
	tok.col = col;
	
	while (is_identifier_char(peek_ch(file))) {
		tok.text[i++] = next_ch(file);
		if (i == buffer_len) {
			buffer_len *= 2;
			tok.text = realloc(tok.text, buffer_len * sizeof(char));
		}
	}
	tok.text = realloc(tok.text, (i+1) * sizeof(char));
	tok.text[i] = '\0';
	return tok;
}

token scan_number(FILE *file) {
	int buffer_len = 32;
	int i = 0;
	token tok;
	int hexadecimal = 0;
	tok.type = number;
	tok.text = (char *)malloc(buffer_len * sizeof(char));
	tok.text[i++] = next_ch(file);
	tok.line = line;
	tok.col = col;
	while (is_identifier_char(peek_ch(file)) || peek_ch(file) == '.') {
		char ch;
		ch = next_ch(file);
		tok.text[i++] = ch;
		if (i == 2 && tok.text[0] == '0' && (tok.text[1] == 'X' || tok.text[1] == 'x')) {
			hexadecimal = 1;
		}
		
		if (i == buffer_len) {
			buffer_len *= 2;
			tok.text = realloc(tok.text, buffer_len * sizeof(char));
		}
		
		if ((!hexadecimal && (ch == 'e' || ch == 'E')) || (hexadecimal && (ch == 'P' || ch == 'p'))) {
			ch = peek_ch(file);
			if (ch == '+' || ch == '-') {
				tok.text[i++] = next_ch(file);
			}
		}
	}
	tok.text = realloc(tok.text, (i+1) * sizeof(char));
	tok.text[i] = '\0';
	return tok;
}

token scan_string(FILE *file) {
	int buffer_len = 32;
	int i = 0;
	token tok;
	tok.text = (char *)malloc(buffer_len * sizeof(char));
	tok.text[i++] = next_ch(file);
	tok.type = (tok.text[0] == '\"') ? string_literal : char_constant;
	tok.line = line;
	tok.col = col;
	
	while (peek_ch(file) != tok.text[0]) {
		char ch;
		ch = next_ch(file);
		tok.text[i++] = ch;
		if (ch == '\n' || ch == '\v') {
			if (tok.type = string_literal) {
				issue_error("Illegal newline character in string literal", line, col);
			} else {
				issue_error("Illegal newline character in character constant", line, col);
			}
		}
		if (i == buffer_len) {
			buffer_len *= 2;
			tok.text = realloc(tok.text, buffer_len * sizeof(char));
		}
		if (ch == '\\') {
			tok.text[i++] = next_ch(file);
		}
		if (i == buffer_len) {
			buffer_len *= 2;
			tok.text = realloc(tok.text, buffer_len * sizeof(char));
		}
	}
	tok.text[i++] = next_ch(file);
	tok.text = realloc(tok.text, (i+1) * sizeof(char));
	tok.text[i] = '\0';
	return tok;
}

token scan_symbol(FILE *file) {
	char ch = next_ch(file);
	char ch2;
	token tok;
	tok.line = line;
	tok.col = col;
	tok.type = symbol;
	switch (ch) {
		//everything that is always one character long
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
		case '~':
		case ':':
		case ';':
		case ',':
		case '?':
		case '.':
			tok.text = (char *)calloc(2, sizeof(char));
			tok.text[0] = ch;
			break;
		//everything that can appear either alone or followed by a '='
		case '!':
		case '%':
		case '^':
		case '*':
		case '=':
			if (peek_ch(file) == '=') {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
				tok.text[1] = next_ch(file);
			} else {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
			}
			break;
		/* everything that can appear either alone, reduplicated, or followed by a '=' */
		case '&':
		case '-':
		case '+':
		case '|':
			ch2 = peek_ch(file);
			if (ch2 == ch || ch2 == '=') {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
				tok.text[1] = next_ch(file);
			} else {
				tok.text = (char *)calloc(2, sizeof(char));
				tok.text[0] = ch;
			}
			break;
		case '<':
		case '>':
			ch2 = peek_ch(file);
			if (ch2 == '=') {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
				tok.text[1] = next_ch(file);
			} else if (ch2 == ch) {
				next_ch(file);
				if (peek_ch(file) == '=') {
					tok.text = (char *)calloc(4, sizeof(char));
					tok.text[0] = ch;
					tok.text[1] = ch2;
					tok.text[3] = next_ch(file);
				} else {
					tok.text = (char *)calloc(3, sizeof(char));
					tok.text[0] = ch;
					tok.text[1] = ch2;
				}
			} else {
				tok.text = (char *)calloc(2, sizeof(char));
				tok.text[0] = ch;
			}
			break;
		case '#':
			if (peek_ch(file) == '#') {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
				tok.text[1] = next_ch(file);
			} else {
				tok.text = (char *)calloc(2, sizeof(char));
				tok.text[0] = ch;
			}
			break;
		case '/':
			ch2 = peek_ch(file);
			if (ch2 == '=') {
				tok.text = (char *)calloc(3, sizeof(char));
				tok.text[0] = ch;
				tok.text[1] = next_ch(file);
			} else if (ch2 == '*') {
				/* comments!  these are fun */
				int asterisk = 0;
				next_ch(file);
				do {
					asterisk = (ch == '*');
					ch = next_ch(file);
				} while (ch && !(asterisk && ch == '/'));
				return next_token(file);
			} else {
				tok.text = (char *)calloc(2, sizeof(char));
				tok.text[0] = ch;
			}
			break;
	}
	return tok;
}

token next_token(FILE *file) {
	char ch;
	/* consume whitespace */
	while (isspace(ch = peek_ch(file))) {
		next_ch(file);
		/* we need to explicitly remember newlines in our token stream for 
		   the sake of preprocessor directives.  Note this doesn't pick up
		   line continuations; those are caught in next_ch */
		if (ch == '\n' || ch == '\v') {
			token tok;
			tok.type = newline;
			tok.text = (char *)calloc(2, sizeof(char));
			tok.text[0] = ch;
			tok.line = line;
			tok.col = col;
			return tok;
		}
		
	}
	if (ch == '\0') {
		token tok;
		tok.line = line;
		tok.col = col;
		tok.type = eof;
		tok.text = (char *)calloc(1, sizeof(char));
		return tok;
	}
	else if (is_identifier_start(ch)) {
		return scan_identifier(file);
	} else if (is_digit(ch)) {
		return scan_number(file);
	} else if (ch == '"' || ch == '\'') {
		return scan_string(file);
	} else if (is_symbol_start(ch)) {
		return scan_symbol(file);
	} else {
		token tok;
		tok.text = (char *)calloc(2, sizeof(char));
		tok.text[0] = next_ch(file);
		tok.line = line;
		tok.col = col;
		tok.type = other;
		return tok;
	}
}

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
			case newline:
				typeString = "New Line";
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


#include <stdio.h> 
#include <string.h>

#include "errors.h"
#include "tokenizer.h"

int line = 1;
int col = 0;

int is_digit(char c) {
	return c >= '0' && c <= '9';
}

int is_identifier_start(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_identifier_char(char c) {
	return is_identifier_start(c) || is_digit(c);
}

int is_symbol_start(char c) {
	return strchr("{}[]().-+&|*~!/%<>=^?:,#;", c) ? 1 : 0; 
}

int prev_newline = 1;
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
	if (c < 0) {
		if (prev_newline) {
			return '\0';
		} else {
			prev_newline = 1;
			return '\n';
		}
	}
	ch = (char) c;
	
	if (ch == '\n' || ch == '\v') {
		line++, col = 0;
		prev_newline = 1;
	} else {
		col++;
		prev_newline = 0;
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
		c = 0;
		if (prev_newline)
			ch = '\0';
		else
			ch = '\n';
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

void skip_line(FILE *file) {
	char ch;
	while (ch = next_ch(file)) {
		if (ch == '\n' || ch == '\v') return;
	}
}

int consume_whitespace(FILE *file) {
	int n = 0;
	char ch;
	while (ch = peek_ch(file)) {
		if (!isspace(ch) || ch == '\n' || ch == '\v') {
			return n;
		} else {
			next_ch(file);
			n++;
		}
	}
	return n;
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
	char ch;
	tok.text = (char *)malloc(buffer_len * sizeof(char));
	tok.text[i++] = next_ch(file);
	tok.type = (tok.text[0] == '\"') ? string_literal : char_constant;
	tok.line = line;
	tok.col = col;
	
	while ((ch = peek_ch(file)) != tok.text[0]) {
		if (ch == '\n' || ch == '\v') {
			tok.text = realloc(tok.text, (i+1) * sizeof(char));
			tok.text[i] = '\0';
			if (tok.type == string_literal) {
				issue_error("Unclosed string literal", tok);
			} else {
				issue_error("Unclosed character constant", tok);
			}
			return tok;
		}
		tok.text[i++] = next_ch(file);
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

/*
 * Similar to scanning a normal string, but with no special treatment of
 * backslash characters
 */
token scan_preprocessor_string(FILE *file) {
	int buffer_len = 32;
	int i = 0;
	token tok;
	char ch, end;
	tok.text = (char *)malloc(buffer_len * sizeof(char));
	tok.text[i++] = next_ch(file);
	tok.type = string_literal;
	tok.line = line;
	tok.col = col;
	if (tok.text[0] == '<') {
		end = '>';
	} else {
		end = '\"';
	}
	while ((ch = peek_ch(file)) != end) {
		if (ch == '\n' || ch == '\v') {
			/* For normal strings, we wanted to make as complete of a string token
			 * as we could.  here, we return an empty string, so we don't end up
			 * including anything. */
			tok.text = realloc(tok.text, sizeof(char));
			tok.text[0] = '\0';
			issue_error("Unclosed string literal", tok);
			return tok;
		}
		tok.text[i++] = next_ch(file);
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
		/* everything that is always one character long */
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
		/* everything that can appear either alone or followed by a '=' */
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
		
		case '-':
			ch2 = peek_ch(file);
			if (ch2 == ch || ch2 == '=' || ch2 == '>') {
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
				tok.type = comment;
				tok.text = "";
			} else {
				tok.text = (char *)calloc(2, sizeof(char));
				tok.text[0] = ch;
			}
			break;
	}
	return tok;
}

token scan_token(FILE *file, int is_include) {
	char ch;
	token tok;
	int whitespace;
	whitespace = consume_whitespace(file);
	ch = peek_ch(file);
	if (ch == '\0') {
		tok.line = line;
		tok.col = col;
		tok.type = eof;
		tok.text = (char *)calloc(1, sizeof(char));
	} else if (ch == '\n' || ch == '\v') {
		tok.text = (char *)calloc(2, sizeof(char));
		tok.text[0] = next_ch(file);
		tok.line = line;
		tok.col = col;
		tok.type = newline;
	} else if (is_identifier_start(ch)) {
		tok = scan_identifier(file);
	} else if (is_digit(ch)) {
		tok = scan_number(file);
	} else if (is_include && (ch == '\"' || ch == '<')) {
		tok = scan_preprocessor_string(file);
	} else if (ch == '"' || ch == '\'') {
		tok = scan_string(file);
	} else if (is_symbol_start(ch)) {
		tok = scan_symbol(file);
		if (tok.type == comment) {
			tok = scan_token(file, is_include);
			whitespace += tok.preceding_whitespace + 1;
		}
	} else {
		tok.text = (char *)calloc(2, sizeof(char));
		tok.text[0] = next_ch(file);
		tok.line = line;
		tok.col = col;
		tok.type = other;
	}
	tok.preceding_whitespace = whitespace;
	return tok;
}


token_stream tokenize_line(FILE *file) {
	token_stream stream;
	token tok;
	int is_include = 0;
	stream = stream_create();
	tok = scan_token(file, is_include);
	stream_append(&stream, tok);
	if (strcmp(tok.text, "#") == 0) {
		tok = scan_token(file, is_include);
		stream_append(&stream, tok);
		is_include = (tok.type == identifier && strcmp(tok.text, "include") == 0);
	}
	while (tok.type != newline && tok.type != eof) {
		tok = scan_token(file, is_include);
		stream_append(&stream, tok);
	}
	return stream;
}


token_stream tokenize_file(FILE *file) {
	token_stream stream, line;
	token tok;
	stream = stream_create();
	do {
		line = tokenize_line(file);
		tok = line.tokens[0];
		stream_cat(&stream, line);
		free(line.tokens);
	} while (tok.type != eof);
	return stream;
}

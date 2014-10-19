#include <stdlib.h> 

typedef enum {
	identifier,
	number,
	char_constant,
	string_literal,
	symbol,
	other,
	newline,
	eof,
} tokenType;

typedef struct {
	char *text;
	int line, col;
	tokenType type;
} token;

int is_identifier_start(char c);
int is_digit(char c);
int is_identifier_char(char c);
int is_symbol_start(char c);
char next_ch(FILE *file);
char peek_ch(FILE *file);
token scan_identifier(FILE *file);
token scan_number(FILE *file);
token scan_string(FILE *file);
token scan_symbol(FILE *file);
token next_token(FILE *file);
token *tokenize(FILE *file, int *count);

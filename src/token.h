#ifndef CC_TOKEN_H
#define CC_TOKEN_H

typedef enum {
	identifier,
	number,
	char_constant,
	string_literal,
	symbol,
	comment,
	other,
	eof
} tokenType;

typedef struct {
	char *text;
	int line, col;
	tokenType type;
} token;


#endif

#ifndef CC_TOKEN_H
#define CC_TOKEN_H

typedef enum {
	identifier,
	number,
	char_constant,
	string_literal,
	symbol,
	comment, /*used internally, not in final token stream*/
	other,
	newline,
	eof,
	/* this is a special token type used to
	 * indicate the token should be replaced
	 * with an argument of a function-like macro 
	 */
	replacement
} tokenType;

typedef struct {
	char *text;
	int line, col;
	tokenType type;
	int replacement_num;
	int preceding_whitespace;
} token;


#endif

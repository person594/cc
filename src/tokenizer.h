#ifndef CC_TOKENIZER_H
#define CC_TOKENIZER_H

#include "token_stream.h"

#include <stdlib.h> 

int is_digit(char c);
int is_identifier_start(char c);
int is_identifier_char(char c);
int is_symbol_start(char c);


/* these functions handle trigraphs and line continuations
 */
char next_ch(FILE *file);
char peek_ch(FILE *file);

void skip_line(FILE *file);
int consume_whitespace(FILE *file);

token scan_identifier(FILE *file);
token scan_number(FILE *file);
token scan_string(FILE *file);
token scan_preprocessor_string(FILE *file);
token scan_symbol(FILE *file);
token scan_token(FILE *file);

/* tokenizes one logical line of c code, taking into account
 * line continuations.  If called at the end of file, returns
 * a token stream consisting of a single EOF token */
token_stream tokenize_line(FILE *file);

#endif

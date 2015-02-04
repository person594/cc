/* macros are kept in a hash table, with linked listed collision
   handling.  This is the size of our table.  Adjust for time/space
   tradeoffs. 
*/
#include "tokenizer.h"
#include "token_stream.h"

#define MACRO_HASH_TABLE_SIZE 1024


/* This is commented out because lots of stuff in there uses
 * conditional directives I can't parse yet, leading to infinite loops */
#define BRACKET_PATH ""/*"/usr/local/include:libdir/gcc/target/version/include:/usr/target/include:/usr/include"*/
#define QUOTE_PATH "."

typedef struct {
	/* num_args is -1 for object-like macros */
	int num_params;
	int variadic;
	char **parameters;
	token_stream replacement;
} macro;
#if 0
void initialize();

void append_token(token tok);

void expand_and_append(token tok);

void tokenize(FILE *file);

void parse_directive(FILE *file);

/* In the case of an error returns a macro with a null body pointer */
void parse_macro_definition(FILE *file);

void expand_function_like_macro(macro *mac, token **args);

/* preprocesses a token stream, returning the resulting stream.
 * does not treat the stream as the start of a line, and thus  will not
 * parse directives */
token_stream preprocess_stream(token_stream stream);
#endif
token_stream parse_directive(token_stream line);
token_stream preprocess(FILE *file);

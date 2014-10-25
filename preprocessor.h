/* macros are kept in a hash table, with linked listed collision
   handling.  This is the size of our table.  Adjust for time/space
   tradeoffs. 
*/
#include "tokenizer.h"

#define MACRO_HASH_TABLE_SIZE 1024
#define INITIAL_TOKEN_STREAM_SIZE 4096

#define INIT_MACRO_PARAMS_SIZE 4
#define INIT_MACRO_BODY_SIZE 16

/* This is commented out because lots of stuff in there uses
 * conditional directives I can't parse yet, leading to infinite loops */
#define BRACKET_PATH ""/*"/usr/local/include:libdir/gcc/target/version/include:/usr/target/include:/usr/include"*/
#define QUOTE_PATH "."

typedef struct {
	/* num_params is -1 for object-like macros */
	int num_params, body_len;
	char **params;
	token *body;
} macro;

void initialize();

void append_token(token tok);

void expand_and_append(token tok);

void tokenize(FILE *file);

void parse_directive(FILE *file);

/* In the case of an error returns a macro with a null body pointer */
void parse_macro_definition(FILE *file);

void expand_function_like_macro(macro *mac, token **args);

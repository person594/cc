/* macros are kept in a hash table, with linked listed collision
   handling.  This is the size of our table.  Adjust for time/space
   tradeoffs. 
*/
#include "tokenizer.h"

#define MACRO_HASH_TABLE_SIZE 1024
#define INITIAL_TOKEN_STREAM_SIZE 4096

#define BRACKET_PATH "/usr/local/include:libdir/gcc/target/version/include:/usr/target/include:/usr/include"
#define QUOTE_PATH "."

typedef struct {
	int num_params, body_len;
	char **params;
	token *body;
} macro;

void initialize();

void append_token(token tok);

void tokenize(FILE *file);

void parse_directive(FILE *file);

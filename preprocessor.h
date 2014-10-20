/* macros are kept in a hash table, with linked listed collision
   handling.  This is the size of our table.  Adjust for time/space
   tradeoffs. 
*/
#define MACRO_HASH_TABLE_SIZE 1024

typedef struct s_macro{
	char *name;
	int n_parameters;
	char **parameters;
	int body_len;
	token *body;
	/* This struct is a linked list node, and here is our next pointer.
	   Used for hash collisions */
	struct s_macro *next;
} macro;

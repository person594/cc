typedef struct s_hash_table_entry {
	char *key;
	void *value;
	struct s_hash_table_entry *next;
} hash_table_entry;


typedef struct {
	unsigned long size;
	hash_table_entry *entries;
} hash_table;

hash_table hash_table_create(unsigned long size);

/* Frees a hash table, freeing the memory for each entry,
 * and optionally freeing the memory for the values stored within.
 * Returns the number of entries in the table
 */
int hash_table_free(hash_table table, int free_values);

/* Inserts a new value into the hash table
 * If a value already existed at the given key, returns it.
 */
void *hash_table_insert(hash_table table, char *key, void *value);

void *hash_table_remove(hash_table table, char *key);

void *hash_table_retrieve(hash_table table, char *key);

unsigned long hash_string(char *key);

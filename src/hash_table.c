#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hash_table.h"

/* The djb2 hash function for strings, graciously stolen from
 * http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long hash_string(char *key) {
	unsigned long hash = 5381;
	int c;
	while ((c = *key++)) {
		hash += (hash<<5) + c;
	}
	return hash;
}

hash_table hash_table_create(unsigned long size) {
	hash_table table;
	table.size = size;
	table.entries = (hash_table_entry *)calloc(size, sizeof(hash_table_entry));
	return table;
}

int hash_table_free(hash_table table, int free_values) {
	unsigned long i;
	unsigned long count = 0;
	for (i = 0; i < table.size; ++i) {
		hash_table_entry *entry;
		entry = &table.entries[i];
		if (entry->key) {
			hash_table_entry *prev;
			int top_level = 1;
			do {
				count++;
				free(entry->key);
				if (free_values) {
					free(entry->value);
				}
				prev = entry;
				entry = entry->next;
				if (!top_level) {

					free(prev);
				}
				top_level = 0;
			} while (entry);
		}
	}
	free(table.entries);
	return count;
}

void *hash_table_insert(hash_table table, char *key, void *value) {
	unsigned long index;
	hash_table_entry new_entry;
	void *old_value = NULL;
	index = hash_string(key) % table.size;
	new_entry.key = malloc((1+strlen(key)) * sizeof(char));
	strcpy(new_entry.key, key);
	new_entry.value = value;
	new_entry.next = NULL;
	if (!table.entries[index].key) {
		table.entries[index] = new_entry;
	} else if (strcmp(table.entries[index].key, key) == 0) {
		old_value = table.entries[index].value;
		table.entries[index] = new_entry;
	} else {
		hash_table_entry *current_entry;
		current_entry = &table.entries[index];
		while(current_entry->next) {
			current_entry = current_entry->next;
			if (strcmp(key, current_entry->key) == 0) {
				old_value = current_entry->value;
				new_entry.next = current_entry->next;
				*current_entry = new_entry;
				return old_value;
			}
		}
		current_entry->next = malloc(sizeof(hash_table_entry));
		*(current_entry->next) = new_entry;
	}
	return old_value;
}

void *hash_table_retrieve(hash_table table, char *key) {
	unsigned long index;
	hash_table_entry *current_entry;
	index = hash_string(key) % table.size;
	current_entry = &table.entries[index];
	if (!current_entry->key) return NULL;
	while (strcmp(key, current_entry->key)) {
		current_entry = current_entry->next;
		if (!current_entry) return NULL;
	}
	return current_entry->value;
}

void *hash_table_remove(hash_table table, char *key) {
	unsigned long index;
	hash_table_entry *current_entry, *previous_entry = NULL;
	void *value;
	index = hash_string(key) % table.size;
	current_entry = &table.entries[index];
	if (!current_entry->key) return NULL;
	while (strcmp(key, current_entry->key)) {
		previous_entry = current_entry;
		current_entry = current_entry->next;
		if (!current_entry) return NULL;
	}
	free(current_entry->key);
	value = current_entry->value;
	if (previous_entry) {
		current_entry->key = NULL;
		previous_entry->next = current_entry->next;
		free(current_entry);
	} else if (current_entry->next) {
		hash_table_entry *next = current_entry->next;
		*current_entry = *current_entry->next;
		free(next);
	}
	return value;
}

/*
int main(int argc, char *argv[]) {
	hash_table table = hash_table_create(1);
	hash_table_insert(table, "hello", "world");
	printf("%s\n", hash_table_retrieve(table, "hello"));
	printf("%s\n", hash_table_remove(table, "hello"));
}
*/

all:
	gcc -std=c89 -pedantic -o cc tokenizer.c errors.c hash_table.c preprocessor.c

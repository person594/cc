#ifndef CC_MACRO_H
#define CC_MACRO_H

typedef struct {
	/* num_args is -1 for object-like macros */
	int num_params;
	char **parameters;
	token_stream replacement;
} macro;

#endif

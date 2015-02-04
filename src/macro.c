#include <string.h>


#include "token_stream.h"
#include "macro.h"
#include <stdlib.h>


macro object_macro_create (token_stream replacement) {
	macro mac;
	mac.num_params = -1;
	/* A macro should keep its own copy of the token stream. */
	mac.replacement = stream_create();
	stream_cat(&mac.replacement, replacement);
	return mac;
}

macro function_macro_create(token_stream parameters, token_stream replacement) {
	macro mac;
	int i;
	
	if (parameters.length == 1) {
		mac.num_params = 1;
	} else {
		mac.num_params = (1+parameters.length)/2;
	}
	mac.parameters = (char **)malloc(mac.num_params * sizeof(char *));

	for (i = 0; i < mac.num_params; ++i) {
		token tok;
		tok = parameters.tokens[2*i];
		if (tok.type != identifier) {
			issue_error("Illegal macro parameter: expected an identifier.", tok);
			free(mac.parameters);
			return object_macro_create(replacement);
		}
		
	}
	
}


int macro_identical(macro a, macro b) {
	int i;
	if (a.num_params != b.num_params) return 0;
	if (! stream_identical(a.replacement, b.replacement)) return 0;
	for (i = 0; i < a.num_params; ++i) {
		if (strcmp(a.parameters[i], b.parameters[i])) return 0;
	}
	return 1;
	
}

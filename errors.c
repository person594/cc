#include <stdio.h>

#include "errors.h"

int error_encountered = 0;

void issue_warning(char *text, token tok) {
	printf("warning int token %s:\n%s\n", tok.text, text);
}

void issue_error(char *text, token tok) {
	error_encountered = 1;
	printf("error in token %s:\n%s\n", tok.text, text);
}

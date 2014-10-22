#include <stdio.h>

#include "errors.h"

int error_encountered = 0;

void issue_warning(char *text, token tok) {
	printf("warn: %s\n", text);
}

void issue_error(char *text, token tok) {
	error_encountered = 1;
	printf("err: %s\n", text);
}

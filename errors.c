#include <stdio.h>

#include "errors.h"

int error_encountered = 0;

void issue_warning(char *text, int line, int col) {
	printf("warn: %s\n", text);
}

void issue_error(char *text, int line, int col) {
	error_encountered = 1;
	printf("err: %s\n", text);
}

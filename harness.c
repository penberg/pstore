#include "pstore/test/harness.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void assert_is_null(const void *p)
{
	if (p) {
		printf("Expected: NULL, but was %p\n", p);
		exit(EXIT_FAILURE);
	}
}

void assert_str_equals(const char *s1, const char *s2)
{
	if (!s2) {
		printf("Expected: '%s', but was: NULL\n", s1);
		exit(EXIT_FAILURE);
	}

	if (strcmp(s1, s2)) {
		printf("Expected: '%s', but was: '%s'\n", s1, s2);
		exit(EXIT_FAILURE);
	}
}

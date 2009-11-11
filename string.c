#include "pstore/string.h"

#include <string.h>
#include <ctype.h>

char *strstrip(char *s)
{
	int i;

	i = strlen(s) - 1;
	while (isspace(s[i])) {
		s[i] = '\0';
		i--;
	}
	while (isspace(*s))
		s++;

	return s;
}

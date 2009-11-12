#include "pstore/string.h"
#include "pstore/compat.h"
#include "pstore/csv.h"

#define _GNU_SOURCE	/* for strndup() */
#include <string.h>

char *csv_field_value(char *s, unsigned long field_idx)
{
	char *start, *end, *ret;
	unsigned long pos;

	pos	= 0;
	start	= s;
	end	= s + strlen(s);

	for (;;) {
		end = strchr(start, ',');
		if (!end) {
			end = s + strlen(s);
			break;
		}
		if (pos++ == field_idx)
			break;

		start = end + 1;
	}
	if (pos < field_idx)
		return NULL;

	ret = strndup(start, end - start);

	return strstrip(ret);
}

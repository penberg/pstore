#include "pstore/string.h"
#include "pstore/compat.h"
#include "pstore/csv.h"
#include "pstore/die.h"

#include <string.h>
#include <ctype.h>

bool csv_field_value(char *s, unsigned long field_idx, struct pstore_value *value)
{
	char *start, *end;
	unsigned long pos;

	pos	= 0;
	start	= s;

	for (;;) {
		end = strchr(start, ',');
		if (!end) {
			end = strchr(start, '\n');
			if (!end)
				die("premature end of line");
			break;
		}
		if (pos++ == field_idx)
			break;

		start = end + 1;
	}
	if (pos < field_idx)
		return false;

        while (isspace(*start))
		start++;

        while (isspace(*end))
		end--;

	value->s	= start;
	value->len	= end - start;

	return true;
}

#include "pstore/string.h"
#include "pstore/compat.h"
#include "pstore/csv.h"
#include "pstore/die.h"

#include <string.h>
#include <ctype.h>

static inline bool is_space_or_tab(char c)
{
	return c == ' ' || c == '\t';
}

bool csv_field_value(char *s, unsigned long field_ndx, struct pstore_value *value)
{
	unsigned long ndx;
	char *start, *end;

	start = s;
	for (ndx = 0; ndx < field_ndx; ndx++) {
		char *tmp;

		tmp = strchr(start, ',');
		if (!tmp)
			return false;

		start = tmp + 1;
	}

        while (isspace(*start))
		start++;

	end = start;
	for (;;) {
		if (*end == '\0' || *end == '\n' || *end == ',')
			break;
		end++;
	}

        while (is_space_or_tab(*end))
		end--;

	value->s	= start;
	value->len	= end - start;

	return true;
}

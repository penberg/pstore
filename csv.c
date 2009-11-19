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

bool csv_field_value(char *s, unsigned long field_idx, struct pstore_value *value)
{
	unsigned long current_field = 0;
	char *start, *end;

	start = s;
	while (current_field < field_idx) {
		if (*start == '\0')
			return false;

		if (*start == ',')
			current_field++;
		start++;
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

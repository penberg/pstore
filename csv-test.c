#include "pstore/test/harness.h"
#include "pstore/csv.h"
#include "test-suite.h"

#include <stdlib.h>
#include <string.h>

static char *line;

void test_csv(void)
{
	struct pstore_value value;

	line = strdup("hello\t,world\n");

	csv_field_value(line, 1, &value);
	assert_int_equals(5, value.len);
	assert_str_equals("world", value.s, value.len);

	free(line);
}

void test_csv_out_of_bounds(void)
{
	struct pstore_value value;

	line = strdup("hello\t,world\n");

	assert_false(csv_field_value(line, 2, &value));

	free(line);
}

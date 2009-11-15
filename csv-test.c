#include "pstore/test/harness.h"
#include "pstore/csv.h"

#include <stdlib.h>
#include <string.h>

static char *line;

static void setup(void)
{
	line = strdup("hello\t,world\n");
}

static void teardown(void)
{
	free(line);
}

void test_csv(void)
{
	struct pstore_value value;

	setup();

	csv_field_value(line, 1, &value);
	assert_str_equals("world", value.s, value.len);

	teardown();
}

void test_csv_out_of_bounds(void)
{
	struct pstore_value value;

	setup();

	assert_false(csv_field_value(line, 2, &value));

	teardown();
}

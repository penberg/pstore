#include "pstore/test/harness.h"
#include "pstore/csv.h"

#include <stdlib.h>
#include <string.h>

static char *line;

static void setup(void)
{
	line = strdup("hello,world\n");
}

static void teardown(void)
{
	free(line);
}

void test_csv(void)
{
	char *s;

	setup();

	s = csv_field_value(line, 1);
	assert_str_equals("world", s);
	free(s);

	teardown();
}

void test_csv_out_of_bounds(void)
{
	setup();

	assert_is_null(csv_field_value(line, 2));

	teardown();
}

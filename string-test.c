#include "pstore/test/harness.h"
#include "pstore/string.h"

#include <stdlib.h>
#include <string.h>

void test_string(void)
{
	char *s = strdup("  hello  \n");

	assert_str_equals("hello", strstrip(s));

	free(s);
}

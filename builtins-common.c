#include "pstore/builtins-common.h"
#include "pstore/core.h"
#include "pstore/disk-format.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

unsigned long parse_storage_arg(char *arg)
{
	char *endptr;
	unsigned long val;

	val = strtol(arg, &endptr, 10);

	if ((endptr[0] == 'k') || (endptr[0] == 'K'))
		return KiB(val);

	return MiB(val);
}

uint8_t parse_comp_arg(char *arg)
{
	if (strcmp(arg, "none") == 0)
		return PSTORE_COMP_NONE;
	else if (strcmp(arg, "fastlz") == 0)
		return PSTORE_COMP_FASTLZ;
#ifdef CONFIG_HAVE_SNAPPY
	else if (strcmp(arg, "snappy") == 0)
		return PSTORE_COMP_SNAPPY;
#endif

	return NR_PSTORE_COMP;
}

void comp_arg_usage(void)
{
	printf("\n The supported compression schemes are:\n");
	printf("   none\n");
	printf("   fastlz\n");
#ifdef CONFIG_HAVE_SNAPPY
	printf("   snappy\n");
#endif
}

unsigned long parse_int_arg(char *arg)
{
	return strtol(arg, NULL, 10);
}

bool is_int_arg(char *arg)
{
	char *endptr;
	long val;

	if (arg[0] == '\0')
		return false;

	val = strtol(arg, &endptr, 10);

	if (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		return false;

	if (errno != 0 && val == 0)
		return false;

	if (endptr[0] == '\0')
		return true;

	return false;
}

bool id_or_name_matches(uint64_t id, const char *name, char *ref)
{
        if (is_int_arg(ref)) {
		if (parse_int_arg(ref) == id)
			return true;
	}

	return strncmp(ref, name, strlen(ref)) == 0;
}

#include "pstore/builtins-common.h"
#include "pstore/core.h"
#include "pstore/disk-format.h"

#include <stdlib.h>
#include <string.h>

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
        if (strcmp(arg, "fastlz") == 0)
		return PSTORE_COMP_FASTLZ;
	else if (strcmp(arg, "none") == 0)
		return PSTORE_COMP_NONE;

	return NR_PSTORE_COMP;
}

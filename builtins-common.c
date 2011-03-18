#include "pstore/builtins-common.h"
#include "pstore/disk-format.h"

#include <stdlib.h>
#include <string.h>

unsigned long parse_int_arg(char *arg)
{
        return strtol(arg, NULL, 10);
}

uint8_t parse_comp_arg(char *arg)
{
        if (strcmp(arg, "fastlz") == 0)
		return PSTORE_COMP_FASTLZ;
	else if (strcmp(arg, "none") == 0)
		return PSTORE_COMP_NONE;

	return NR_PSTORE_COMP;
}

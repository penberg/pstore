#include "pstore/die.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void do_die(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}


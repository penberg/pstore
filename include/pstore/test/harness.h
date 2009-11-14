#ifndef PSTORE_HARNESS_H
#define PSTORE_HARNESS_H

#include <stdbool.h>
#include <stddef.h>

void assert_is_null(const void *p);
void assert_str_equals(const char *s1, const char *s2, size_t len);
void assert_false(bool value);

#endif /* PSTORE_HARNESS_H */

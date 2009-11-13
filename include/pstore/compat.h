#ifndef PSTORE_COMPAT_H
#define PSTORE_COMPAT_H

#include <stddef.h>

#ifdef CONFIG_NEED_STRNDUP
char *strndup(const char *s, size_t n);
#endif

#endif /* PSTORE_COMPAT_H */

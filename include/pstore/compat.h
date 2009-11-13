#ifndef PSTORE_COMPAT_H
#define PSTORE_COMPAT_H

#ifdef CONFIG_NEED_STRNDUP
#include <stddef.h>

char *strndup(const char *s, size_t n);
#endif

#ifdef CONFIG_NEED_LARGE_FILE_COMPAT
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

typedef off_t off64_t;

static inline off64_t lseek64(int fd, off64_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

static inline FILE *fopen64(const char *filename, const char *type)
{
	return fopen(filename, type);
}
#endif /* CONFIG_NEED_LARGE_FILE_COMPAT */

#endif /* PSTORE_COMPAT_H */

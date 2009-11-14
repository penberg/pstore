#ifndef PSTORE_COMPAT_H
#define PSTORE_COMPAT_H

#ifdef CONFIG_NEED_STRNDUP
#include <stddef.h>

char *strndup(const char *s, size_t n);
#endif

#ifdef CONFIG_NEED_LARGE_FILE_COMPAT
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define O_LARGEFILE 0

typedef off_t off64_t;

static inline off64_t lseek64(int fd, off64_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

#define stat64 stat

static inline int fstat64(int fd, struct stat64 *st)
{
	return fstat(fd, st);
}

static inline FILE *fopen64(const char *filename, const char *type)
{
	return fopen(filename, type);
}
#endif /* CONFIG_NEED_LARGE_FILE_COMPAT */

#endif /* PSTORE_COMPAT_H */

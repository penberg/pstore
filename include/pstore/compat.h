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

#ifdef CONFIG_NEED_FSTAT64

#define stat64 stat

static inline int fstat64(int fd, struct stat64 *st)
{
	return fstat(fd, st);
}
#endif /* CONFIG_NEED_FSTAT64 */

static inline FILE *fopen64(const char *filename, const char *type)
{
	return fopen(filename, type);
}
#endif /* CONFIG_NEED_LARGE_FILE_COMPAT */

#ifdef CONFIG_NEED_POSIX_FALLOCATE
#include <errno.h>

static inline int posix_fallocate64(int fd, off_t offset, off_t len)
{
	return 0;
}
#endif

#ifdef CONFIG_NEED_POSIX_FADVISE
enum {
	POSIX_FADV_NORMAL,
	POSIX_FADV_SEQUENTIAL,
	POSIX_FADV_RANDOM,
	POSIX_FADV_NOREUSE,
	POSIX_FADV_WILLNEED,
	POSIX_FADV_DONTNEED,
};

static inline int posix_fadvise64(int fd, off_t offset, off_t len, int advice)
{
	return 0;
}
#endif

#endif /* PSTORE_COMPAT_H */

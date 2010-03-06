#ifndef PSTORE_COMPAT_H
#define PSTORE_COMPAT_H

#ifdef CONFIG_NEED_STRNDUP
#include <stddef.h>

char *strndup(const char *s, size_t n);
#endif

#ifdef CONFIG_NEED_POSIX_FALLOCATE
#include <errno.h>

static inline int posix_fallocate(int fd, off_t offset, off_t len)
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

static inline int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
	return 0;
}
#endif

#endif /* PSTORE_COMPAT_H */

#define _LARGEFILE64_SOURCE

#include "pstore/read-write.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* Same as read(2) except that this function never returns EAGAIN or EINTR. */
ssize_t xread(int fd, void *buf, size_t count)
{
	ssize_t nr;

restart:
	nr = read(fd, buf, count);
	if ((nr < 0) && ((errno == EAGAIN) || (errno == EINTR)))
		goto restart;

	return nr;
}

/* Same as write(2) except that this function never returns EAGAIN or EINTR. */
ssize_t xwrite(int fd, const void *buf, size_t count)
{
	ssize_t nr;

restart:
	nr = write(fd, buf, count);
	if ((nr < 0) && ((errno == EAGAIN) || (errno == EINTR)))
		goto restart;

	return nr;
}

ssize_t read_in_full(int fd, void *buf, size_t count)
{
	ssize_t total = 0;
	char *p = buf;

	while (count > 0) {
		ssize_t nr;

		nr = xread(fd, p, count);
		if (nr <= 0) {
			if (total > 0)
				return total;

			return -1;
		}

		count -= nr;
		total += nr;
		p += nr;
	}

	return total;
}

ssize_t write_in_full(int fd, const void *buf, size_t count)
{
	const char *p = buf;
	ssize_t total = 0;

	while (count > 0) {
		ssize_t nr;

		nr = xwrite(fd, p, count);
		if (nr < 0)
			return -1;
		if (nr == 0) {
			errno = ENOSPC;
			return -1;
		}
		count -= nr;
		total += nr;
		p += nr;
	}

	return total;
}

void read_or_die(int fd, void *buf, size_t count)
{
	if (read_in_full(fd, buf, count) != count)
		die("read_or_die: %s", strerror(errno));
}

void write_or_die(int fd, const void *buf, size_t count)
{
	if (write_in_full(fd, buf, count) != count)
		die("write_or_die: %s", strerror(errno));
}

off64_t seek_or_die(int fd, off64_t offset, int whence)
{
	off64_t ret;

	ret = lseek64(fd, offset, whence);
	if (ret < 0)
		die("lseek64: %s", strerror(errno));

	return ret;
}

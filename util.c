#include "pstore/read-write.h"

#include "pstore/die.h"

#include <string.h>
#include <errno.h>

void write_or_die(int fd, const void *buf, size_t count)
{
	if (write_in_full(fd, buf, count) != count)
		die("write_or_die: %s", strerror(errno));
}

off_t seek_or_die(int fd, off_t offset, int whence)
{
	off_t ret;

	ret = lseek(fd, offset, whence);
	if (ret < 0)
		die("lseek: %s", strerror(errno));

	return ret;
}

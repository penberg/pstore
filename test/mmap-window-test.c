#include "pstore/test/harness.h"
#include "pstore/mmap-window.h"
#include "test-suite.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE		sysconf(_SC_PAGE_SIZE)


#define WINDOW_SIZE		(2 * PAGE_SIZE)
#define FD_OFFSET		(4ULL * 1024ULL * 1024ULL) /* 4 MB */
#define FD_SIZE			(1ULL * 1024ULL * 1024ULL) /* 1 MB */

static struct mmap_window	*mmap;
static int			fd;

static void setup(void)
{
	fd	= open("/dev/zero", O_RDONLY);

	mmap	= mmap_window__map(WINDOW_SIZE, fd, FD_OFFSET, FD_SIZE);
}

static void teardown(void)
{
	mmap_window__unmap(mmap);
	close(fd);
}

void test_mmap_window_in_window(void)
{
	void *start;

	setup();

	start = mmap_window__start(mmap);

	assert_true(mmap_window__in_window(mmap, start + WINDOW_SIZE - 1));
	assert_false(mmap_window__in_window(mmap, start + WINDOW_SIZE));

	teardown();
}

void test_mmap_window_in_region(void)
{
	void *start;

	setup();

	start = mmap_window__start(mmap);

	assert_true(mmap_window__in_region(mmap, start + WINDOW_SIZE - 1));
	assert_true(mmap_window__in_region(mmap, start + WINDOW_SIZE));
	assert_true(mmap_window__in_region(mmap, start + FD_SIZE - 1));
	assert_false(mmap_window__in_region(mmap, start + FD_SIZE));

	teardown();
}

void test_mmap_window_slide(void)
{
	void *start;

	setup();

	start = mmap_window__start(mmap);
	assert_true(mmap_window__in_region(mmap, start + WINDOW_SIZE));

	start = mmap_window__slide(mmap, start + WINDOW_SIZE);

	assert_true(mmap_window__in_region(mmap, start + FD_SIZE - WINDOW_SIZE - 1));

	teardown();
}

#include "pstore/test/harness.h"
#include "pstore/mmap-window.h"
#include "test-suite.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE        sysconf(_SC_PAGE_SIZE)

#define WINDOW_SIZE        (2 * PAGE_SIZE)
#define FD_OFFSET        (4ULL * 1024ULL * 1024ULL) /* 4 MB */
#define MMAP_LENGTH        (1ULL * 1024ULL * 1024ULL) /* 1 MB */

static struct mmap_window    *mmap;
static int            fd;

static void setup(void)
{
    fd    = open("/dev/zero", O_RDONLY);

    mmap    = mmap_window_map(WINDOW_SIZE, fd, FD_OFFSET, MMAP_LENGTH);
}

static void teardown(void)
{
    mmap_window_unmap(mmap);
    close(fd);
}

void test_mmap_window_in_window(void)
{
    void *start;

    setup();

    start = mmap_window_start(mmap);

    assert_true(mmap_window_in_window(mmap, start));
    assert_true(mmap_window_in_window(mmap, start + WINDOW_SIZE - 1));
    assert_false(mmap_window_in_window(mmap, start + WINDOW_SIZE));

    teardown();
}

void test_mmap_window_in_region(void)
{
    void *start;

    setup();

    start = mmap_window_start(mmap);

    assert_true(mmap_window_in_region(mmap, start + WINDOW_SIZE - 1));
    assert_true(mmap_window_in_region(mmap, start + WINDOW_SIZE));
    assert_true(mmap_window_in_region(mmap, start + MMAP_LENGTH - 1));
    assert_false(mmap_window_in_region(mmap, start + MMAP_LENGTH));

    teardown();
}

void test_mmap_window_slide(void)
{
    void *start;

    setup();

    start = mmap_window_start(mmap);
    assert_true(mmap_window_in_region(mmap, start + WINDOW_SIZE));

    start = mmap_window_slide(mmap, start + WINDOW_SIZE);

    assert_true(mmap_window_in_region(mmap, start + MMAP_LENGTH - WINDOW_SIZE - 1));

    teardown();
}

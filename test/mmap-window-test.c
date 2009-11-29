#include "pstore/test/harness.h"
#include "pstore/mmap-window.h"
#include "test-suite.h"

#include <stdlib.h>
#include <string.h>

#define MMAP_START	((void *) 0x1000UL)
#define MMAP_POS	0x100UL
#define MMAP_END	((void *) 0x2000UL)
#define MMAP_WINDOW_LEN	(MMAP_END - MMAP_START - MMAP_POS)

#define REGION_LEN	0x4000UL

static struct mmap_window mmap;

static void setup(void)
{
	mmap = (struct mmap_window) {
		.mmap		= MMAP_START,
		.mmap_pos	= MMAP_POS,
		.mmap_end	= MMAP_END,

		.length		= REGION_LEN,
	};
}

static void teardown(void)
{
}

void test_mmap_window_in_window(void)
{
	setup();

	assert_true(mmap_window__in_window(&mmap, mmap_window__start(&mmap) + MMAP_WINDOW_LEN - 1));

	teardown();
}

void test_mmap_window_not_in_window(void)
{
	setup();

	assert_false(mmap_window__in_window(&mmap, mmap_window__start(&mmap) + MMAP_WINDOW_LEN));

	teardown();
}

void test_mmap_window_in_region(void)
{
	setup();

	assert_true(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + MMAP_WINDOW_LEN));
	assert_true(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + REGION_LEN - 1));

	teardown();
}

void test_mmap_window_not_in_region(void)
{
	setup();

	assert_false(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + REGION_LEN));

	teardown();
}

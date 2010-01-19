#include "pstore/test/harness.h"
#include "pstore/mmap-window.h"
#include "test-suite.h"

#include <stdlib.h>
#include <string.h>

#define MMAP_START	((void *) 0x1000ULL)
#define MMAP_POS	0x100ULL
#define MMAP_END	((void *) 0x2000ULL)
#define MMAP_WINDOW_LEN	(MMAP_END - MMAP_START - MMAP_POS)

#define REGION_OFF	0x1000ULL
#define SLIDED_OFF	0x2000ULL
#define REGION_LEN	0x4000ULL

static struct mmap_window mmap;
static struct mmap_window s_mmap;	/* slided mmap */

static void setup(void)
{
	mmap = (struct mmap_window) {
		.mmap		= MMAP_START,
		.mmap_pos	= MMAP_POS,
		.mmap_end	= MMAP_END,

		.offset		= REGION_OFF,
		.length		= REGION_LEN,

		.start_off	= REGION_OFF + MMAP_POS,
	};

	s_mmap = (struct mmap_window) {
		.mmap		= MMAP_START,
		.mmap_pos	= MMAP_POS,
		.mmap_end	= MMAP_END,

		.offset		= SLIDED_OFF,
		.length		= REGION_LEN,

		.start_off	= REGION_OFF + MMAP_POS,
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

	assert_true(mmap_window__in_region(&mmap, mmap_window__start(&mmap)));
	assert_true(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + MMAP_WINDOW_LEN));
	assert_true(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + REGION_LEN - 1));
	assert_true(mmap_window__in_region(&s_mmap, mmap_window__start(&s_mmap) + REGION_LEN - (SLIDED_OFF - REGION_OFF) - 1));

	teardown();
}

void test_mmap_window_not_in_region(void)
{
	setup();

	assert_false(mmap_window__in_region(&mmap, mmap_window__start(&mmap) + REGION_LEN));
	assert_false(mmap_window__in_region(&s_mmap, mmap_window__start(&s_mmap) + REGION_LEN - 1));

	teardown();
}

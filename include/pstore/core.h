#ifndef PSTORE_CORE_H
#define PSTORE_CORE_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define KiB(x) (x * 1024ULL)
#define MiB(x) (x * KiB(1024ULL))

#define PAGE_SIZE		getpagesize()
#define PAGE_MASK		(~(PAGE_SIZE-1))

#endif /* PSTORE_CORE_H */

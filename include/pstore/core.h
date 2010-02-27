#ifndef PSTORE_CORE_H
#define PSTORE_CORE_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define KiB(x) (x * 1024ULL)
#define MiB(x) (x * KiB(1024ULL))

#endif /* PSTORE_CORE_H */

#ifndef PSTORE_TYPES_H
#define PSTORE_TYPES_H

#include <stdint.h>

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef uint8_t			u8;
typedef uint16_t __bitwise	le16;
typedef uint16_t __bitwise	be16;
typedef uint32_t __bitwise	le32;
typedef uint32_t __bitwise	be32;
typedef uint64_t __bitwise	le64;
typedef uint64_t __bitwise	be64;

#endif /* PSTORE_TYPES_H */

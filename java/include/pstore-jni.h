#ifndef PSTOREJNI_H
#define PSTOREJNI_H

#include <inttypes.h>

#ifdef __i386__
  #define LONG_TO_PTR(ptr) (void *) (uint32_t) ptr
#elif __X86_64__
  #define LONG_TO_PTR(ptr) (void *) ptr
#endif

#define PTR_TO_LONG(ptr) (long) ptr

#endif

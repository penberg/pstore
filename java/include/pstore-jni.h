#ifndef PSTOREJNI_H
#define PSTOREJNI_H

#include <jni.h>
#include <inttypes.h>

#ifdef __i386__
#define LONG_TO_PTR(ptr) (void *) (uint32_t) ptr
#elif __x86_64__
#define LONG_TO_PTR(ptr) (void *) ptr
#endif

#define PTR_TO_LONG(ptr) (long) ptr

void throw_exception(JNIEnv *env, const char *exception_class_name, const char *message);
void throw_io_exception(JNIEnv * env, const char *message);
void throw_out_of_memory_error(JNIEnv * env, const char *message);

#endif

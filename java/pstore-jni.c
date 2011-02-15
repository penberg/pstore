#include "pstore-jni.h"

void throw_exception(JNIEnv *env, const char *exception_class_name, const char *message)
{
        jclass exception_class = (*env)->FindClass(env, exception_class_name);
        if (!exception_class)
                (*env)->FatalError(env, "Exception class not found");
        (*env)->ThrowNew(env, exception_class, message);
        (*env)->DeleteLocalRef(env, exception_class);
}

void throw_io_exception(JNIEnv * env, const char *message)
{
	throw_exception(env, "java/io/IOException", message);
}

void throw_out_of_memory_error(JNIEnv * env, const char *message)
{
	throw_exception(env, "java/lang/OutOfMemoryError", message);
}

#include "pstore_IteratorState.h"
#include "pstore/column.h"
#include "pstore-jni.h"

JNIEXPORT jlong JNICALL Java_pstore_IteratorState_create(JNIEnv *env, jobject obj)
{
	return PTR_TO_LONG((*env)->NewGlobalRef(env, obj));
}

JNIEXPORT void JNICALL Java_pstore_IteratorState_destroy(JNIEnv *env, jobject obj, jlong iter_state_ptr)
{
	(*env)->DeleteGlobalRef(env, LONG_TO_PTR(iter_state_ptr));
}

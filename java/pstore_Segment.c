#include "pstore_Segment.h"
#include "pstore/segment.h"
#include "pstore-jni.h"

JNIEXPORT void JNICALL Java_pstore_Segment_destroy(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_segment *segment = LONG_TO_PTR(ptr);
  pstore_segment__delete(segment);
}

JNIEXPORT jlong JNICALL Java_pstore_Segment_read(JNIEnv *env, jclass clazz, jlong ptr, jint fd)
{
  return PTR_TO_LONG(pstore_segment__read(LONG_TO_PTR(ptr), fd));
}

JNIEXPORT jstring JNICALL Java_pstore_Segment_next(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_segment *segment;
  void *value;

  segment = LONG_TO_PTR(ptr);
  value = pstore_segment__next_value(segment);

  if (value == NULL)
    return NULL;

  return (*env)->NewStringUTF(env, value);
}

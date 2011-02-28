#include "pstore_Column.h"

#include "pstore/column.h"
#include "pstore-jni.h"

JNIEXPORT jlong JNICALL Java_pstore_Column_create(JNIEnv *env, jobject obj, jstring name, jlong id, jint type)
{
	const char *name0;
	void *ptr;

	name0 = (*env)->GetStringUTFChars(env, name, NULL);
	if (!name0)
		return PTR_TO_LONG(NULL);

	ptr = pstore_column__new(name0, id, type);

	(*env)->ReleaseStringUTFChars(env, name, name0);

	return PTR_TO_LONG(ptr);
}

JNIEXPORT void JNICALL Java_pstore_Column_destroy(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_column *column = LONG_TO_PTR(ptr);
  pstore_column__delete(column);
}

JNIEXPORT jstring JNICALL Java_pstore_Column_name(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_column *column = LONG_TO_PTR(ptr);
  return (*env)->NewStringUTF(env, column->name);
}

JNIEXPORT jlong JNICALL Java_pstore_Column_id(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_column *column = LONG_TO_PTR(ptr);
  return column->column_id;
}

JNIEXPORT int JNICALL Java_pstore_Column_type(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_column *column = LONG_TO_PTR(ptr);
  return column->type;
}

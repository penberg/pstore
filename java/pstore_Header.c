#include "pstore_Header.h"
#include "pstore/header.h"
#include "pstore-jni.h"

JNIEXPORT void JNICALL Java_pstore_Header_write(JNIEnv *env, jclass clazz, jlong ptr, jint fd)
{
	pstore_header_write(LONG_TO_PTR(ptr), fd);
}

JNIEXPORT jlong JNICALL Java_pstore_Header_create(JNIEnv *env, jclass clazz)
{
	return PTR_TO_LONG(pstore_header_new());
}

JNIEXPORT void JNICALL Java_pstore_Header_destroy(JNIEnv *env, jclass clazz, jlong ptr)
{
	pstore_header_delete(LONG_TO_PTR(ptr));
}

JNIEXPORT void JNICALL Java_pstore_Header_insertTable(JNIEnv *env, jclass clazz, jlong header_ptr, jlong table_ptr)
{
	pstore_header_insert_table(LONG_TO_PTR(header_ptr), LONG_TO_PTR(table_ptr));
}

JNIEXPORT jlong JNICALL Java_pstore_Header_read(JNIEnv *env, jclass clazz, jint fd)
{
	return PTR_TO_LONG(pstore_header_read(fd));
}

JNIEXPORT jint JNICALL Java_pstore_Header_nrTables(JNIEnv *env, jclass clazz, jlong ptr)
{
	struct pstore_header *header = LONG_TO_PTR(ptr);

	return header->nr_tables;
}

JNIEXPORT jlong JNICALL Java_pstore_Header_tables(JNIEnv *env, jclass clazz, jlong ptr, jint ndx)
{
	struct pstore_header *header = LONG_TO_PTR(ptr);

	return PTR_TO_LONG(header->tables[ndx]);
}

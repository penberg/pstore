JAVAC	?= javac
JAVAH	?= javah
JAVA	?= java
CC	?= gcc

LIBRARY := libpstore-java.so
LD_LIBRARY_PATH := .
JAVA_CLASSPATH := .

JAVA_FILES += $(wildcard pstore/*.java)
JAVA_FILES += $(wildcard pstore/examples/*.java)
JAVA_CLASSES := $(JAVA_FILES:%.java=%.class)

JAVA_NATIVE_CLASSES := \
	pstore.Column \
	pstore.Header \
	pstore.IteratorState \
	pstore.PStoreFile \
	pstore.Row \
	pstore.Segment \
	pstore.Table

OBJS := $(patsubst %,%.o,$(subst .,_,$(JAVA_NATIVE_CLASSES)))
JAVA_HEADER_FILES := $(patsubst %.o,%.h,$(OBJS))

CFLAGS := -Wall -g -O6 -std=gnu99
DEFINES := -D_FILE_OFFSET_BITS=64

INCLUDE := \
	-Iinclude/ \
	-I../include \
	-I$(JAVA_HOME)/include \
	-I$(JAVA_HOME)/include/linux
LIBS := -L../ -lpstore

all: test
.DEFAULT: all
.PHONY: all

test: $(JAVA_CLASSES) $(LIBRARY)
	@LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) java -classpath $(JAVA_CLASSPATH) pstore.examples.PStore
.PHONY: test

clean:
	@rm -rf $(JAVA_CLASSES) $(JAVA_HEADER_FILES) $(LIBRARY) $(OBJS)
.PHONY: clean

$(LIBRARY): $(JAVA_HEADER_FILES) $(OBJS)
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -shared -o $(LIBRARY) $(OBJS) $(LIBS)

$(JAVA_HEADER_FILES):
	@$(JAVAH) -classpath $(JAVA_CLASSPATH) -d include -jni $(JAVA_NATIVE_CLASSES)

%.class: %.java
	@echo "  JAVAC   " $@
	@$(JAVAC) -classpath $(JAVA_CLASSPATH) $<

%.o: %.c
	@echo "  CC      " $@
	@$(CC) $(CFLAGS) $(DEFINES) -c $(INCLUDE) $< -o $@

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

JAVAC	?= javac
JAVAH	?= javah
JAVA	?= java
CC	?= gcc

ifeq ($(uname_S),Darwin)
  EXT := jnilib
endif
ifeq ($(uname_S),Linux)
  EXT := so
endif
LIBRARY := libpstore-java.$(EXT)

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
  -I../include
ifeq ($(uname_S),Darwin)
  INCLUDE += -I$(JAVA_HOME)/../Headers
endif
ifeq ($(uname_S),Linux)
  INCLUDE += \
    -I$(JAVA_HOME)/include/linux \
    -I$(JAVA_HOME)/include
endif
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

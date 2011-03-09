uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

JAVAC	?= javac
JAVAH	?= javah
JAVA	?= java
JAR	?= jar
CC	?= gcc

# Make the build silent by default
V =
ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q

VERSION = 0.0.2

ifeq ($(uname_S),Darwin)
  EXT := jnilib
endif
ifeq ($(uname_S),Linux)
  EXT := so
endif
LIBRARY_BASE := libpstore-java.$(EXT)
LIBRARY := $(LIBRARY_BASE).$(VERSION)

LIBRARY_JAR := pstore-$(VERSION).jar
SOURCE_JAR := pstore-$(VERSION)-sources.jar

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
JAVA_HEADER_FILES := $(patsubst %.o,include/%.h,$(OBJS))

# pstore-jni.h is not part of $(JAVA_HEADER_FILES).
OBJS += pstore-jni.o

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

all: test dist
.DEFAULT: all
.PHONY: all

test: $(JAVA_CLASSES) $(LIBRARY_BASE)
	$(E) "  TEST      "
	$(Q) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) java -classpath $(JAVA_CLASSPATH) pstore.examples.PStore
.PHONY: test

check: $(JAVA_CLASSES) $(LIBRARY)
	$(E) "  CUCUMBER  "
	$(Q) cucumber --format progress
.PHONY: check

clean:
	$(E) "  CLEAN     "
	$(Q) rm -rf $(JAVA_CLASSES) $(JAVA_HEADER_FILES) $(LIBRARY) $(OBJS) \
		$(LIBRARY_BASE) $(LIBRARY_JAR) $(SOURCE_JAR)
.PHONY: clean

dist: $(LIBRARY_JAR) $(SOURCE_JAR)
.PHONY: dist

$(LIBRARY_JAR): $(JAVA_CLASSES)
	$(E) "  JAR       " $@
	$(Q) $(JAR) cf $(LIBRARY_JAR) $(JAVA_CLASSES)

$(SOURCE_JAR): $(JAVA_FILES)
	$(E) "  JAR       " $@
	$(Q) $(JAR) cf $(SOURCE_JAR) $(JAVA_FILES)

$(LIBRARY): $(JAVA_HEADER_FILES) $(OBJS)
	$(E) "  LINK      " $@
	$(Q) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -shared -o $(LIBRARY) $(OBJS) \
		$(LIBS)

$(LIBRARY_BASE): $(LIBRARY)
	$(E) "  SYMLINK   " $@
	$(Q) ln -s $(LIBRARY) $(LIBRARY_BASE)

$(JAVA_HEADER_FILES):
	$(E) "  JAVAH     "
	$(Q) $(JAVAH) -classpath $(JAVA_CLASSPATH) -d include -jni \
		$(JAVA_NATIVE_CLASSES)

%.class: %.java
	$(E) "  JAVAC     " $@
	$(Q) $(JAVAC) -classpath $(JAVA_CLASSPATH) $<

%.o: %.c
	$(E) "  CC        " $@
	$(Q) $(CC) $(CFLAGS) $(DEFINES) -c $(INCLUDE) $< -o $@

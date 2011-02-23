uname_S	:= $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_R	:= $(shell sh -c 'uname -r 2>/dev/null || echo not')

# External programs
CC	:= gcc
AR	:= ar

# Set up source directory for GNU Make
srcdir		:= $(CURDIR)
VPATH		:= $(srcdir)

EXTRA_WARNINGS := -Wcast-align
EXTRA_WARNINGS += -Wformat
EXTRA_WARNINGS += -Wformat-security
EXTRA_WARNINGS += -Wformat-y2k
EXTRA_WARNINGS += -Wshadow
EXTRA_WARNINGS += -Winit-self
EXTRA_WARNINGS += -Wpacked
EXTRA_WARNINGS += -Wredundant-decls
EXTRA_WARNINGS += -Wstrict-aliasing=3
#EXTRA_WARNINGS += -Wswitch-default
EXTRA_WARNINGS += -Wswitch-enum
EXTRA_WARNINGS += -Wno-system-headers
#EXTRA_WARNINGS += -Wundef		# This is disabled because of MiniLZO brain-damage
EXTRA_WARNINGS += -Wwrite-strings
EXTRA_WARNINGS += -Wbad-function-cast
EXTRA_WARNINGS += -Wmissing-declarations
EXTRA_WARNINGS += -Wmissing-prototypes
EXTRA_WARNINGS += -Wnested-externs
EXTRA_WARNINGS += -Wold-style-definition
EXTRA_WARNINGS += -Wstrict-prototypes
EXTRA_WARNINGS += -Wdeclaration-after-statement

# Compile flags
CFLAGS	:= -I$(srcdir)/include -Wall $(EXTRA_WARNINGS) -g -O3 -std=gnu99

# Output to current directory by default
O =

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

# Project files
PROGRAM := pstore

DEFINES =
CONFIG_OPTS =
COMPAT_OBJS =

ifeq ($(uname_S),Darwin)
	CONFIG_OPTS += -DCONFIG_NEED_STRNDUP=1
	COMPAT_OBJS += compat/strndup.o

	CONFIG_OPTS += -DCONFIG_NEED_POSIX_FALLOCATE=1
	CONFIG_OPTS += -DCONFIG_NEED_POSIX_FADVISE=1
endif
ifeq ($(uname_S),Linux)
	DEFINES += -D_FILE_OFFSET_BITS=64
	DEFINES += -D_GNU_SOURCE
endif
ifeq ($(uname_S),SunOS)
	DEFINES += -D_FILE_OFFSET_BITS=64

	CONFIG_OPTS += -DCONFIG_NEED_STRNDUP=1
	COMPAT_OBJS += compat/strndup.o
endif

OBJS += builtin-cat.o
OBJS += builtin-import.o
OBJS += builtin-repack.o
OBJS += builtin-stat.o
OBJS += pstore.o

OBJS += $(COMPAT_OBJS)

LIBS := -L. -lpstore

CFLAGS += $(DEFINES)
CFLAGS += $(CONFIG_OPTS)

DEPS		:= $(patsubst %.o,%.d,$(OBJS))

LIB_FILE := libpstore.a

LIB_OBJS += buffer.o
LIB_OBJS += column.o
LIB_OBJS += compress.o
LIB_OBJS += csv.o
LIB_OBJS += die.o
LIB_OBJS += extent.o
LIB_OBJS += fastlz/fastlz.o
LIB_OBJS += header.o
LIB_OBJS += mmap-window.o
LIB_OBJS += read-write.o
LIB_OBJS += segment.o
LIB_OBJS += table.o

LIB_DEPS	:= $(patsubst %.o,%.d,$(LIB_OBJS))

TEST_PROGRAM	:= test-pstore
TEST_SUITE_H	:= test/test-suite.h
TEST_RUNNER_C	:= test/test-runner.c
TEST_RUNNER_OBJ := test/test-runner.o

TEST_OBJS += harness.o
TEST_OBJS += test/csv-test.o
ifneq ($(uname_S),Darwin)
TEST_OBJS += test/mmap-window-test.o
endif

TEST_SRC	:= $(patsubst %.o,%.c,$(TEST_OBJS))
TEST_DEPS	:= $(patsubst %.o,%.d,$(TEST_OBJS))

TEST_LIBS := $(LIBS)

# Targets
all: sub-make
.DEFAULT: all
.PHONY: all

ifneq ($(O),)
sub-make: $(O) $(FORCE)
	$(Q) $(MAKE) --no-print-directory -C $(O) -f ../Makefile srcdir=$(CURDIR) _all
else
sub-make: _all
endif

_all: $(PROGRAM) $(LIB_FILE)
.PHONY: _all

$(O):
ifneq ($(O),)
	$(E) "  MKDIR   " $@
	$(Q) mkdir -p $(O)
endif

%.d: %.c
	$(Q) $(CC) -M -MT $(patsubst %.d,%.o,$@) $(CFLAGS) $< -o $@

%.o: %.c
	$(E) "  CC      " $@
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

$(PROGRAM): $(DEPS) $(LIB_FILE) $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(OBJS) $(LIBS) -o $(PROGRAM)

$(LIB_FILE): $(LIB_DEPS) $(LIB_OBJS)
	$(E) "  AR      " $@
	$(Q) rm -f $@ && $(AR) rcs $@ $(LIB_OBJS)

test: $(TEST_PROGRAM)
	$(E) "  CHECK"
	$(Q) ./$(TEST_PROGRAM)
.PHONY: test

$(TEST_RUNNER_C): $(FORCE)
	$(E) "  GEN     " $@
	$(Q) sh scripts/gen-test-runner "$(TEST_SRC)" > $@

$(TEST_SUITE_H): $(FORCE)
	$(E) "  GEN     " $@
	$(Q) sh scripts/gen-test-proto "$(TEST_SRC)" > $@

$(TEST_PROGRAM): $(TEST_SUITE_H) $(TEST_DEPS) $(TEST_OBJS) $(TEST_RUNNER_OBJ) $(LIB_FILE)
	$(E) "  LINK    " $@
	$(E) "  LINK    " $<
	$(Q) $(CC) $(TEST_OBJS) $(TEST_RUNNER_OBJ) $(TEST_LIBS) -o $(TEST_PROGRAM)

clean:
	$(E) "  CLEAN"
	$(Q) $(MAKE) -C java clean
	$(Q) rm -f $(LIB_FILE) $(LIB_OBJS) $(LIB_DEPS)
	$(Q) rm -f $(PROGRAM) $(OBJS) $(DEPS) $(TEST_PROGRAM) $(TEST_SUITE_H) $(TEST_OBJS) $(TEST_DEPS) $(TEST_RUNNER_OBJ)
.PHONY: clean

regress: $(PROGRAM)
	$(E) "  CUCUMBER"
	$(Q) cucumber --format progress
ifneq ($(JAVA_HOME),)
	$(Q) $(MAKE) -C java check
endif
.PHONY: regress

check: test regress
.PHONY: check

tags: FORCE
	$(E) "  TAGS"
	$(Q) rm -f tags
	$(Q) ctags-exuberant -a *.c
	$(Q) ctags-exuberant -a -R include

PHONY += FORCE

FORCE:

# Deps
-include $(DEPS)

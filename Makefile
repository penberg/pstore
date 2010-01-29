uname_S	:= $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_R	:= $(shell sh -c 'uname -r 2>/dev/null || echo not')

# External programs
CC	:= gcc

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
EXTRA_WARNINGS += -Wswitch-default
EXTRA_WARNINGS += -Wswitch-enum
EXTRA_WARNINGS += -Wno-system-headers
EXTRA_WARNINGS += -Wundef
EXTRA_WARNINGS += -Wwrite-strings
EXTRA_WARNINGS += -Wbad-function-cast
EXTRA_WARNINGS += -Wmissing-declarations
EXTRA_WARNINGS += -Wmissing-prototypes
EXTRA_WARNINGS += -Wnested-externs
EXTRA_WARNINGS += -Wold-style-definition
EXTRA_WARNINGS += -Wstrict-prototypes
EXTRA_WARNINGS += -Wdeclaration-after-statement

# Compile flags
CFLAGS	:= -I$(srcdir)/include -Wall $(EXTRA_WARNINGS) -g -O6 -std=gnu99

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
	CONFIG_OPTS += -DCONFIG_NEED_POSIX_FALLOCATE=1
	COMPAT_OBJS += compat/strndup.o

	CONFIG_OPTS += -DCONFIG_NEED_LARGE_FILE_COMPAT=1

	ifeq ($(shell expr "$(uname_R)" : '8\.'),2)
		CONFIG_OPTS += -DCONFIG_NEED_FSTAT64=1
	endif
endif
ifeq ($(uname_S),Linux)
	DEFINES += -D_LARGEFILE64_SOURCE=1
	DEFINES += -D_GNU_SOURCE=1
endif
ifeq ($(uname_S),SunOS)
	DEFINES += -D_LARGEFILE64_SOURCE=1
	CONFIG_OPTS += -DCONFIG_NEED_STRNDUP=1
	COMPAT_OBJS += compat/strndup.o
endif

OBJS += buffer.o
OBJS += builtin-cat.o
OBJS += builtin-import.o
OBJS += column.o
OBJS += csv.o
OBJS += die.o
OBJS += extent.o
OBJS += header.o
OBJS += mmap-window.o
OBJS += pstore.o
OBJS += read-write.o
OBJS += table.o

OBJS += $(COMPAT_OBJS)

CFLAGS += $(DEFINES)
CFLAGS += $(CONFIG_OPTS)

DEPS		:= $(patsubst %.o,%.d,$(OBJS))

TEST_PROGRAM	:= test-pstore
TEST_SUITE_H	:= test/test-suite.h
TEST_RUNNER_C	:= test/test-runner.c

TEST_OBJS := csv.o
TEST_OBJS += die.o
TEST_OBJS += harness.o
TEST_OBJS += mmap-window.o
TEST_OBJS += test/csv-test.o
TEST_OBJS += test/mmap-window-test.o
TEST_OBJS += test/test-runner.o

TEST_DEPS	:= $(patsubst %.o,%.d,$(TEST_OBJS))

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

_all: $(PROGRAM)
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

$(PROGRAM): $(DEPS) $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(OBJS) -o $(PROGRAM)

test: $(TEST_PROGRAM)
	$(E) "  CHECK"
	$(Q) ./$(TEST_PROGRAM)
.PHONY: test

$(TEST_RUNNER_C): $(FORCE)
	$(E) "  GEN     " $@
	$(Q) sh scripts/gen-test-runner "test/*-test.c" > $@

$(TEST_SUITE_H): $(FORCE)
	$(E) "  GEN     " $@
	$(Q) sh scripts/gen-test-proto "test/*-test.c" > $@

$(TEST_PROGRAM): $(TEST_SUITE_H) $(TEST_DEPS) $(TEST_OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(TEST_OBJS) -o $(TEST_PROGRAM)

clean:
	$(E) "  CLEAN"
	$(Q) rm -f $(PROGRAM) $(OBJS) $(DEPS) $(TEST_PROGRAM) $(TEST_SUITE_H) $(TEST_OBJS) $(TEST_DEPS) $(TEST_RUNNER_C)
.PHONY: clean

regress: $(PROGRAM)
	$(E) "  REGRESS"
	$(Q) $(MAKE) -s -C regress
.PHONY: regress

check: test regress
.PHONY: check

PHONY += FORCE

FORCE:

# Deps
-include $(DEPS)

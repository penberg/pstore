uname_S	:= $(shell sh -c 'uname -s 2>/dev/null || echo not')

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
EXTRA_WARNINGS += -Wstack-protector
EXTRA_WARNINGS += -Wstrict-aliasing=3
EXTRA_WARNINGS += -Wswitch-default
EXTRA_WARNINGS += -Wswitch-enum
EXTRA_WARNINGS += -Wno-system-headers
EXTRA_WARNINGS += -Wundef
EXTRA_WARNINGS += -Wvolatile-register-var
EXTRA_WARNINGS += -Wwrite-strings
EXTRA_WARNINGS += -Wbad-function-cast
EXTRA_WARNINGS += -Wmissing-declarations
EXTRA_WARNINGS += -Wmissing-prototypes
EXTRA_WARNINGS += -Wnested-externs
EXTRA_WARNINGS += -Wold-style-definition
EXTRA_WARNINGS += -Wstrict-prototypes
EXTRA_WARNINGS += -Wdeclaration-after-statement

# Compile flags
CFLAGS	:= -I$(srcdir)/include -Wall $(EXTRA_WARNINGS) -g -Os

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

COMPAT_OBJS =

ifeq ($(uname_S),Darwin)
	COMPAT_OBJS += compat/strndup.o
endif
ifeq ($(uname_S),SunOS)
	COMPAT_OBJS += compat/strndup.o
endif

OBJS := block.o 
OBJS += builtin-cat.o
OBJS += builtin-import.o
OBJS += column.o
OBJS += csv.o
OBJS += die.o
OBJS += header.o
OBJS += pstore.o
OBJS += read-write.o
OBJS += string.o
OBJS += table.o

OBJS += $(COMPAT_OBJS)

DEPS		:= $(patsubst %.o,%.d,$(OBJS))

TEST_PROGRAM	:= test-pstore
TEST_OBJS	:= test-runner.c harness.o string-test.o string.o csv-test.o csv.o
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

check: $(TEST_PROGRAM)
	$(E) "  CHECK"
	$(Q) ./$(TEST_PROGRAM)
.PHONY: check

test-runner.c: $(FORCE)
	$(E) "  GEN     " $@
	$(Q) sh scripts/gen-test-runner > $@

$(TEST_PROGRAM): $(TEST_DEPS) $(TEST_OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(TEST_OBJS) -o $(TEST_PROGRAM)

clean:
	$(E) "  CLEAN"
	$(Q) rm -f $(PROGRAM) $(OBJS) $(DEPS) $(TEST_PROGRAM) $(TEST_OBJS) $(TEST_DEPS)
.PHONY: clean

PHONY += FORCE

FORCE:

# Deps
-include $(DEPS)

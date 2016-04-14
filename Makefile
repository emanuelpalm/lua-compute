# Base build settings.
AR              = ar
CC              = clang
CFLAGS          = -std=c99 -Wall -Wpedantic
LDFLAGS         =
RM				= rm -f

# Debug build settings.
DEBUG_AR        = ${AR}
DEBUG_CC        = ${CC}
DEBUG_CFLAGS    = ${CFLAGS} -O0 -g
DEBUG_LDFLAGS   = ${LDFLAGS}

# Release build settings.
RELEASE_AR      = ${AR}
RELEASE_CC      = ${CC}
RELEASE_CFLAGS  = ${CFLAGS} -O3
RELEASE_LDFLAGS = ${LDFLAGS} -Os

# Test build settings.
TEST_CC         = ${DEBUG_CC}
TEST_CFLAGS     = ${DEBUG_CFLAGS}
TEST_LDFLAGS    = ${DEBUG_LDFLAGS}

# Source and object files.
CFILES          = $(wildcard src/main/c/*.c)
OFILES          = $(CFILES:%.c=%.o)

# Test files.
TEST_BIN        = lmr-tests.out
TEST_CFILES     = $(wildcard src/test/c/*.c)
TEST_OFILES     = $(TEST_CFILES:%.c=%.o)

default: test

all: release test

debug:

release:

test:
	${MAKE} ${TEST_BIN} CC="${TEST_CC}" CFLAGS="${TEST_CFLAGS}" \
		LDFLAGS="${TEST_LDFLAGS}"

clean:
	$(foreach F,${OFILES},${RM} $F)
	$(foreach F,${TEST_BIN},${RM} $F)
	$(foreach F,${TEST_OFILES},${RM} $F)

%.o:
	${CC} ${CFLAGS} -c $(@:%.o=%.c) -o $@

${TEST_BIN}: ${TEST_OFILES}
	${CC} ${LDFLAGS} -o $@ $^

# Dependency map.
src/main/c/lmr.o: src/main/c/lmr.c src/main/c/lmr.h src/main/c/lmrconf.h \
	src/main/c/lmrlua.h
src/test/c/unit.o: src/test/c/unit.c src/test/c/unit.h
src/test/c/main.o: src/test/c/main.c src/test/c/lmr.unit.o src/test/c/unit.o
src/test/c/lmr.unit.o: src/test/c/lmr.unit.c src/test/c/unit.o

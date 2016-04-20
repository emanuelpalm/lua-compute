# Platform settings.
PLATFORM_OS     = $(shell uname -s)
PLATFORM_ARCH   = $(shell uname -m)

ifeq (${PLATFORM_OS},Darwin)
	PLATFORM_CFLAGS = -I/usr/local/include/luajit-2.0
	PLATFORM_LDFLAGS = -L/usr/local/lib
	PLATFORM_TEST_LIBS = -lluajit
	PLATFORM_SOEXT = dylib
	ifneq ($(filter %64,${PLATFORM_ARCH}),)
		PLATFORM_TEST_LDFLAGS += -pagezero_size 10000 -image_base 100000000
	endif
else
	PLATFORM_SOEXT = so
endif

# Build settings.
AR              = ar -rcu
CC              = clang
CFLAGS          = -std=c99 -Wall -Wpedantic ${PLATFORM_CFLAGS}
LDFLAGS         = ${PLATFORM_LDFLAGS}
LIBS            =
RM              = rm -f

DEBUG_AR        = ${AR}
DEBUG_CC        = ${CC}
DEBUG_CFLAGS    = ${CFLAGS} -O0 -g
DEBUG_LDFLAGS   = ${LDFLAGS}
DEBUG_LIBS      = ${LIBS}

RELEASE_AR      = ${AR}
RELEASE_CC      = ${CC}
RELEASE_CFLAGS  = ${CFLAGS} -O3
RELEASE_LDFLAGS = ${LDFLAGS} -Os
RELEASE_LIBS    = ${LIBS}

TEST_CC         = ${DEBUG_CC}
TEST_CFLAGS     = ${DEBUG_CFLAGS}
TEST_LDFLAGS    = ${DEBUG_LDFLAGS} ${PLATFORM_TEST_LDFLAGS}
TEST_LIBS       = ${DEBUG_LIBS} ${PLATFORM_TEST_LIBS}

OEXT            = o
SOEXT           = ${PLATFORM_SOEXT}

# Source and object files.
CFILES          = $(wildcard src/main/c/*.c)
OFILES          = $(CFILES:%.c=%.${OEXT})
LIBA            = libluamare.a
LIBSO           = libluamare.${SOEXT}

DEBUG_LIBA      = $(LIBA:%.a=%.debug.a)
DEBUG_LIBSO     = $(LIBSO:%.${SOEXT}=%.debug.${SOEXT})

RELEASE_LIBA    = ${LIBA}
RELEASE_LIBSO   = ${LIBSO}

TEST_BIN        = lmr-tests.out
TEST_CFILES     = ${CFILES} $(wildcard src/test/c/*.c)
TEST_OFILES     = $(TEST_CFILES:%.c=%.${OEXT})

default: test

all: debug release test

debug:
	@${MAKE} ${DEBUG_LIBA} LIBA="${DEBUG_LIBA}" CC="${DEBUG_CC}" \
		CFLAGS="${DEBUG_CFLAGS}" LDFLAGS="${DEBUG_LDFLAGS}" \
		LIBS="${DEBUG_LIBS}" OEXT="debug.o" --no-print-directory
	@${MAKE} ${DEBUG_LIBSO} LIBSO="${DEBUG_LIBSO}" CC="${DEBUG_CC}" \
		CFLAGS="${DEBUG_CFLAGS} -fPIC" LDFLAGS="${DEBUG_LDFLAGS}" \
		LIBS="${DEBUG_LIBS}" OEXT="debug.pic.o" --no-print-directory

release:
	@${MAKE} ${RELEASE_LIBA} LIBA="${RELEASE_LIBA}" CC="${RELEASE_CC}" \
		CFLAGS="${RELEASE_CFLAGS}" LDFLAGS="${RELEASE_LDFLAGS}" \
		LIBS="${RELEASE_LIBS}" --no-print-directory
	@${MAKE} ${RELEASE_LIBSO} LIBSO="${RELEASE_LIBSO}" CC="${RELEASE_CC}" \
		CFLAGS="${RELEASE_CFLAGS} -fPIC" LDFLAGS="${RELEASE_LDFLAGS}" \
		LIBS="${RELEASE_LIBS}" OEXT="pic.o" --no-print-directory

test:
	@${MAKE} ${TEST_BIN} CC="${TEST_CC}" CFLAGS="${TEST_CFLAGS}" \
		LDFLAGS="${TEST_LDFLAGS}" LIBS="${TEST_LIBS}" OEXT="debug.o" \
		--no-print-directory

clean:
	$(foreach F,$(wildcard src/main/c/*.[do]),${RM} $F;)
	$(foreach F,$(wildcard *.a),${RM} $F;)
	$(foreach F,$(wildcard *.${SOEXT}),${RM} $F;)
	$(foreach F,$(wildcard ${TEST_BIN}),${RM} $F;)
	$(foreach F,$(wildcard src/test/c/*.[do]),${RM} $F;)

.PHONY: default all debug release test clean

%.${OEXT}:
	${CC} ${CFLAGS} -c $*.c -o $@

${LIBA}: ${OFILES}
	${AR} $@ $^

${LIBSO}: ${OFILES}
	${CC} -shared ${LDFLAGS} ${LIBS} -o $@ $^

${TEST_BIN}: ${TEST_OFILES}
	${CC} ${LDFLAGS} ${LIBS} -o $@ $^

# Dependency map.
src/main/c/lmr.${OEXT}: src/main/c/lmr.c src/main/c/lmr.h \
	src/main/c/lmrconf.h src/main/c/lmrlua.h
src/test/c/lmr.unit.${OEXT}: src/test/c/lmr.unit.c src/main/c/lmr.h \
	src/main/c/lmrconf.h src/test/c/unit.h
src/test/c/main.${OEXT}: src/test/c/main.c src/test/c/unit.h
src/test/c/unit.${OEXT}: src/test/c/unit.c src/test/c/unit.h

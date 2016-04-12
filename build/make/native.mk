# Makefile for native (current platform) build targets.

NATIVE_OUT  := ${PROJECT_OUT}/native
LIB_LUAMARE := ${NATIVE_OUT}/libs/luamare/libluamare.so
LIB_LUAJIT2 := ${NATIVE_OUT}/libs/luajit2/lib/libluajit-5.1.a

default-native: ${LIB_LUAMARE}

all-native: ${LIB_LUAMARE}

${LIB_LUAMARE}: ${LIB_LUAJIT2}

${LIB_LUAJIT2}:
	${PROJECT_BUILD}/bash/make_luajit2.sh ${PROJECT_BUILD}/env.conf native

clean-native:
	${RM} -R ${NATIVE_OUT}

.PHONY: default-native all-native clean-native

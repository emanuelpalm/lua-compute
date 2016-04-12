# Makefile for native (current platform) build targets.

NATIVE_OUT  := ${PROJECT_OUT}/native
LIB_LUAMARE := ${NATIVE_OUT}/libs/luamare/libluamare.so
LIB_LUAJIT2 := ${NATIVE_OUT}/libs/luajit2/lib/libluajit-5.1.so

# User actions.

default-native: libs-native

all-native: libs-native

libs-native:
	@${MAKE} -f ${PROJECT_MAKE}/main.mk libs-native-inner --no-print-directory

clean-native:
	${RM} -R ${NATIVE_OUT}

.PHONY: default-native all-native libs-native clean-native

# Inner actions. Do not use these directly.

libs-native-inner: ${LIB_LUAJIT2}

${LIB_LUAJIT2}:
	${PROJECT_BUILD}/bash/make_luajit2.sh ${PROJECT_BUILD}/env.conf native

.PHONY: libs-native-inner

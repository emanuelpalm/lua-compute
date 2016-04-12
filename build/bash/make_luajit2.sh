#!/usr/bin/env bash

# Builds luaJIT 2 for a specified set of platforms. Places build output in
# out/<platform>/libs/luajit2/.

if [ "$#" -lt 1 ];
then
    echo >&2 "[!] All required parameters not provided."
    echo >&2 "    Usage: $0 <path/to/env.conf> <platform>"
    exit 1
fi

# Path to file containing required environment variables. Required.
ENV=$1
if [ -e "$ENV" ];
then
    . "$ENV"
else
    echo >&2 "[!] Invalid environment file referenced: $ENV."
    exit 2
fi

# Platform to build for. Optional.
PLATFORM=${2:-native}

LUA_PATH="$PROJECT_LIBS/luajit2"
MAKE=${MAKE:-make}

# Builds for Android.
android() {
    local filename=$(basename "$1")
    local name=${filename:5}
    echo >&2 "Building LuaJIT2 for Android ($name) ..."

    # If name contains 64, we assume it names a 64-bit architecture.
    if [[ $name == *64* ]];
    then
        local cpu="64"
    else
        local cpu="32"
    fi

    local toolchains=$(find $NDK_PATH/toolchains -maxdepth 1 -ipath *$name*)
    for i in $toolchains;
    do
        # 64-bit architectures must have 64 in their toolchain names, and
        # 32-bit architectures must not have 64 in their names.
        if [ "$cpu" == "32" ];
        then
            [[ $i == *64* ]] && { continue; }
        else
            [[ $i != *64* ]] && { continue; }
        fi
        local toolchain=$i
        break
    done
    if [ "$toolchain" == "" ];
    then
        echo >&2 "[!] No suitable NDK toolchain available for arch ($name)."
    else
        local gcc=$(find $toolchain -ipath */bin/*-gcc)
        local bin_prefix=${gcc%???}
        local lua_prefix="$PROJECT_OUT/android-$name/libs/luajit2"
        local uname=`uname`
        mkdir -p "$lua_prefix"
        cd "$LUA_PATH" \
            && $MAKE clean \
            && $MAKE CC=gcc HOST_CC="gcc -m$cpu" HOST_SYS=$uname \
            CROSS=$bin_prefix TARGET_FLAGS="--sysroot=$1" TARGET_SYS=Linux \
            && $MAKE install PREFIX=\""$lua_prefix"\"
    fi
}

# Builds for the host platform.
native() {
    echo >&2 "Building LuaJIT2 ..."
    local lua_prefix="$PROJECT_OUT/native/libs/luajit2"
    mkdir -p "$lua_prefix"
    cd "$LUA_PATH" \
        && $MAKE clean \
        && $MAKE \
        && $MAKE install PREFIX=\""$lua_prefix"\"
}

case $PLATFORM in
    android)
        archs=${NDK_TARGET_ARCHS//:/ }
        for arch in $archs;
        do
            android $arch
        done
        unset archs
        ;;
    native)
        native
        ;;
    *)
        echo >&2 "[!] Unsupported target platform: $PLATFORM."
        exit 3
        ;;
esac

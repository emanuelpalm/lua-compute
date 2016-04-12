#!/usr/bin/env bash

# Builds luaJIT 2 for a specified platform.

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

case $PLATFORM in
    android)
        echo "Android: $NDK_PATH $LUA_PATH"
        ;;
    native)
        LUA_PREFIX="$PROJECT_OUT/native/libs/luajit2"
        mkdir -p "$LUA_PREFIX"
        cd "$LUA_PATH" \
            && make \
            && make install PREFIX=\""$LUA_PREFIX"\" \
            && make clean
        ;;
    *)
        echo >&2 "[!] Unsupported target platform: $PLATFORM."
        exit 3
        ;;
esac

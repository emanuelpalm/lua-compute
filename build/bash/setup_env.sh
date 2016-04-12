# Ensures required environment variables are defined.

# Local file containing shell definitions.
PROJECT_ENV="$PROJECT_BUILD/env.conf"

# Attempts to find named directory, returning its path if successful.
findpath() {
    echo >&2 -n "   - Finding $1 ..."
    local result=""
    for path in ~ /opt;
    do
        for i in "$(find $path -ipath $1 2>/dev/null)";
        do
            [[ "$i" == "" ]] && { break; }
            echo >&2 " FOUND AT $i"
            result="$i"
            break 2
        done
    done
    [[ "$result" == "" ]] && {
        echo >&2 " NOT FOUND"
    }
    echo "$result"
}

MISSING=0

[[ -f "$PROJECT_ENV" ]] && {
    . "$PROJECT_ENV"
}

# Sets reference to make utility.
[[ -z "$MAKE" ]] && {
    MAKE="make"
}

# Resolve Android NDK path.
[[ -z "$NDK_PATH" ]] && {
    NDK_PATH=$(findpath "*/ndk-bundle/ndk-build")
    if [ "$NDK_PATH" != "" ];
    then
        NDK_PATH=$(dirname "$NDK_PATH")
    else
        let "MISSING += 1"
        echo >&2 "     Android NDK not found. Cannot set NDK_PATH."
    fi
}

# Resolve Android NDK ABI level and platform.
([[ -z "$NDK_PLATFORM" ]] || [[ -z "$NDK_PLATFORM_ABI" ]]) \
    && [[ "$NDK_PATH" != "" ]] && {
    for level in $NDK_PLATFORM_ABI 16 9 21 23 24 25 26 27 28 29 30 3 4 5 8;
    do
        NDK_PLATFORM=$(findpath "$NDK_PATH/platform*-$level")
        [[ "$NDK_PLATFORM" != "" ]] && {
            NDK_PLATFORM_ABI="$level"
            break
        }
    done
    [[ "$NDK_PLATFORM" == "" ]] && {
        let "MISSING += 1"
        echo >&2 "     No NDK platform found. Cannot set NDK_PLATFORM."
        echo >&2 "     No NDK platform found. Cannot set NDK_PLATFORM_ABI."
    }
}

# Resolve target Android NDK CPU architectures.
[[ -z "$NDK_TARGET_ARCHS" ]] && [[ "$NDK_PLATFORM" != "" ]] && {
    echo >&2 -n "   - Finding platform architecutes ..."
    platforms="$(find $NDK_PLATFORM -iname arch-* -type d 2>/dev/null)"
    for i in $platforms;
    do
        name=$(basename "$i")
        echo >&2 -n " ${name:5}"
        NDK_TARGET_ARCHS="$i:$NDK_TARGET_ARCHS"
    done
    echo >&2

    unset name
    unset platforms

    [[ "$NDK_TARGET_ARCHS" == "" ]] && {
        let "MISSING += 1"
        echo >&2 "     No NDK target archs found. Cannot set NDK_TARGET_ARCHS."
    }

    NDK_TARGET_ARCHS="${NDK_TARGET_ARCHS%?}"
}

# Set library root folder.
[[ -z "$PROJECT_LIBS" ]] && {
    PROJECT_LIBS="$PROJECT_ROOT/libs"
}

# Set build output folder.
[[ -z "$PROJECT_OUT" ]] && {
    PROJECT_OUT="$PROJECT_ROOT/out"
}

# Sets current platform name.
[[ -z "$UNAME" ]] && {
    UNAME=`uname`
}

if [ "$MISSING" == "0" ];
then
    touch "$PROJECT_ENV"
    echo "MAKE=$MAKE" >> "$PROJECT_ENV"
    echo "NDK_PATH=$NDK_PATH" >> "$PROJECT_ENV"
    echo "NDK_PLATFORM=$NDK_PLATFORM" >> "$PROJECT_ENV"
    echo "NDK_PLATFORM_ABI=$NDK_PLATFORM_ABI" >> "$PROJECT_ENV"
    echo "NDK_TARGET_ARCHS=$NDK_TARGET_ARCHS" >> "$PROJECT_ENV"
    echo "PROJECT_BUILD=$PROJECT_BUILD" >> "$PROJECT_ENV"
    echo "PROJECT_LIBS=$PROJECT_LIBS" >> "$PROJECT_ENV"
    echo "PROJECT_OUT=$PROJECT_OUT" >> "$PROJECT_ENV"
    echo "PROJECT_ROOT=$PROJECT_ROOT" >> "$PROJECT_ENV"
    echo "UNAME=$UNAME" >> "$PROJECT_ENV"
else
    echo >&2
    echo >&2 "  [!] Some significant paths could not be resolved. Please"
    echo >&2 "  review the above list and make sure all desired utilities"
    echo >&2 "  are installed and available via your home folder. If you"
    echo >&2 "  intalled the listed utilities outside your home folder,"
    echo >&2 "  you have the option of specifying those paths manually in"
    echo >&2 "  the following file:"
    echo >&2
    echo >&2 "  $PROJECT_ENV"
    echo >&2
fi

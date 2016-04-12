# Ensures required environment variables are defined.

# Local file containing shell definitions.
PROJECT_ENV="$PROJECT_BUILD/.env"

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

POPULATE_ENV=0
MISSING=0

if [ -f "$PROJECT_ENV" ];
then
    . "$PROJECT_ENV"
else
    POPULATE_ENV=1
fi

# Resolve Android NDK path.
[[ -z "$PATH_NDK_BUNDLE" ]] && {
    PATH_NDK_BUNDLE=$(findpath "*/ndk-bundle/ndk-build")
    if [ "$PATH_NDK_BUNDLE" != "" ];
    then
        PATH_NDK_BUNDLE=$(dirname "$PATH_NDK_BUNDLE")
        echo "PATH_NDK_BUNDLE=$PATH_NDK_BUNDLE" >> "$PROJECT_ENV"
    else
        let "MISSING += 1"
        echo >&2 "     Android NDK not found. Cannot define PATH_NDK_BUNDLE."
    fi
}

if [ "$MISSING" != "0" ];
then
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

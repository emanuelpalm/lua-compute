#!/usr/bin/env bash

### A collection of routines for setting up your local repository for building
### and development. Prints nothing if no action is performed.

## Standard functions.
abspath() {
    if [ -d "$(dirname "$1")" ];
    then
        echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
    fi
}
absdirname() {
    echo "$(dirname $(abspath "$1"))"
}

## Standard variables.
PROJECT_ROOT=$(dirname "$(dirname "$(absdirname "$0")")")
PROJECT_BUILD="$PROJECT_ROOT/build"
PROJECT_BASH="$PROJECT_BUILD/bash"

## Modules.
. "$PROJECT_BASH/setup_tools.sh"
. "$PROJECT_BASH/setup_env.sh"

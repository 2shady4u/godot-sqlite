#!/bin/sh
# Generate amalgamation by replacing '#include "..."' lines with file contents.
# Usage: ./script/amalgamate.sh <sqleet.c >amalgamation.c
while IFS='' read -r ln; do
    if echo "$ln" | grep -q '^#include "[^"]\+"$'; then
        cat "$(printf "%s\n" "$ln" | sed 's/^#include "\([^"]\+\)"/\1/')"
    else
        printf "%s\n" "$ln"
    fi
done

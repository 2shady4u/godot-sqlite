#!/bin/sh
# Prepare release archives.

set -e

die() {
    [ -n "$RELEASE" ] && rm -rf "$RELEASE_DIR" "$RELEASE.zip" "$RELEASE.tar.gz"
    [ "$#" -ne 0 ] && echo "[-] Error:" "$@" >&2
    exit 1
}

cd "$(git rev-parse --show-toplevel)"
[ "$(git symbolic-ref --short -q HEAD)" = "master" ] || die "checkout master first"
git status --short | grep -vq '^[!?]' && die "dirty working tree (commit or stash your changes)"
VERSION="$(sed -n 's/^#define SQLITE_VERSION[^"]*"\([0-9]\+\.[0-9]\+\.[0-9]\+\)"$/\1/p' sqlite3.h)"
[ -z "$VERSION" ] && die "cannot find SQLite3 version"
SQLEET_VERSION="$(echo "$VERSION" | sed 's/^3/0/')"
RELEASE="sqleet-v$SQLEET_VERSION"
RELEASE_DIR="$RELEASE"

echo "[+] SQLite version $VERSION" >&2

echo "[+] Checking rekeyvacuum.c" >&2
./script/rekeyvacuum.sh sqlite3.c | cmp -s - rekeyvacuum.c || die "outdated rekeyvacuum.c"

echo "[+] Creating release directory $RELEASE_DIR" >&2
mkdir -p "$RELEASE_DIR"
cp README.md "$RELEASE_DIR"

echo "[+] Generating sqleet.c amalgamation" >&2
./script/amalgamate.sh <sqleet.c >"$RELEASE_DIR/sqleet.c" || die "sqleet.c amalgamation failed"

echo "[+] Generating sqleet.h amalgamation" >&2
./script/amalgamate.sh <sqleet.h >"$RELEASE_DIR/sqleet.h" || die "sqleet.h amalgamation failed"

echo '[+] Updating shell.c #include "sqlite3.h" -> "sqleet.h"' >&2
sed 's/^#include "sqlite3.h"$/#include "sqleet.h"/' shell.c >"$RELEASE_DIR/shell.c"
grep -Fq '#include "sqleet.h"' "$RELEASE_DIR/shell.c" || die "error updating shell.c include"

echo "[+] Copying sqlite3ext.h to release directory" >&2
cp sqlite3ext.h "$RELEASE_DIR/sqlite3ext.h" || die "error copying sqlite3ext.h"

echo "[+] Creating release archives" >&2
zip -qr "$RELEASE-amalgamation.zip" "$RELEASE_DIR" || die "error creating $RELEASE-amalgamation.zip"
tar -czf "$RELEASE-amalgamation.tar.gz" --owner=0 --group=0 "$RELEASE_DIR" || die "error creating $RELEASE-amalgamation.tar.gz"

sync
echo "[+] Success!" >&2
echo "git tag v$SQLEET_VERSION && git push origin v$SQLEET_VERSION"

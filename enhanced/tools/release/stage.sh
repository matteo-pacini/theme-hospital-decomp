#!/bin/sh
# Fills DEST (a folder to be created) with the files every release carries
# next to the program: README.txt, LICENSE, COPYING, CHANGELOG.md and
# THIRD_PARTY_NOTICES.txt.
#
#   stage.sh DEST DEPS_DIR
#
# DEPS_DIR is the build's `_deps` folder (see notices.sh).
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 2 ] || die "usage: stage.sh DEST DEPS_DIR"
dest=$1
deps=$2
version=$("$root/enhanced/tools/release/version.sh")

mkdir -p "$dest"
sed "s/@VERSION@/$version/g" "$root/enhanced/tools/release/README.release.txt" >"$dest/README.txt"
cp "$root/LICENSE" "$root/COPYING" "$root/enhanced/CHANGELOG.md" "$dest/"
"$root/enhanced/tools/release/notices.sh" "$deps" >"$dest/THIRD_PARTY_NOTICES.txt"

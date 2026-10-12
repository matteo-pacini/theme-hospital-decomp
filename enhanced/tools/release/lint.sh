#!/bin/sh
# clang-tidy and clang-format over enhanced/, as the gate (check.sh) does.
# Run from anywhere inside the `enhanced` dev shell
# (`nix develop path:./nix#enhanced -c enhanced/tools/release/lint.sh`).
# Builds in BUILD_DIR (default build/lint) because tidy needs the compile
# commands and the generated headers.
#
#   lint.sh [BUILD_DIR]
set -eu
. "$(dirname "$0")/lib.sh"

build=${1:-build/lint}
cd "$root"

cmake -S enhanced -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DTH_DEV_DATA_DIR=
cmake --build "$build" >"$build/build.log" 2>&1 || {
  cat "$build/build.log"
  die "the build failed"
}

TIDY_BUILD=$build enhanced/tools/tidy.sh

out=$(find enhanced/src enhanced/tests enhanced/tools \( -name '*.cpp' -o -name '*.h' \) -print0 |
  xargs -0 clang-format --dry-run 2>&1)
if [ -n "$out" ]; then
  printf '%s\n' "$out"
  die "clang-format reports differences"
fi
echo "clang-format: clean"

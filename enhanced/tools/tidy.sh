#!/bin/sh
# Run clang-tidy (enhanced/.clang-tidy) over enhanced/'s own sources, or over
# the files given. Run from the repository root in the dev shell
# (`nix develop path:./nix#enhanced`), after configuring build/enhanced (or the
# build tree in TIDY_BUILD). TIDY_JOBS clang-tidy processes run at once
# (default: one per core). TIDY_EXTRA: space-separated compiler arguments
# added to every file's (the WebAssembly build's target and sysroot, for check.sh's WEB=1).
# Exits non-zero when clang-tidy reports any warning.
set -e
db=${TIDY_BUILD:-build/enhanced}
if [ ! -f "$db/compile_commands.json" ]; then
  echo "configure $db first" >&2
  exit 1
fi
if [ $# -eq 0 ]; then
  set -- $(grep -o '"file": "[^"]*/enhanced/\(src\|tests\|tools\)/[^"]*"' "$db/compile_commands.json" | cut -d'"' -f4)
fi
extra=
for arg in ${TIDY_EXTRA:-}; do extra="$extra --extra-arg=$arg"; done
out=$(mktemp)
trap 'rm -f "$out"' EXIT
printf '%s\n' "$@" | xargs -P "${TIDY_JOBS:-$(nproc)}" -n 1 clang-tidy --quiet -p "$db" $extra >"$out" 2>/dev/null || true
cat "$out"
n=$(grep -c 'warning:' "$out" || true)
echo "$n warnings"
[ "$n" -eq 0 ]

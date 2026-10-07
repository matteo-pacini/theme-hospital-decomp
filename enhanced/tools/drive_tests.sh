#!/bin/sh
# Runs the drive scripts in enhanced/tests/drive (or the ones given) through
# th_enhanced --drive, headless, and reports each; exits non-zero if any
# fails. A "# Level N" line in a script's header picks the level (default 1).
# Run from the repository root after building build/enhanced (or the build
# tree in DRIVE_BUILD). Screenshots and dumps go to build/drive-tests/.
set -u
build=${DRIVE_BUILD:-build/enhanced}
[ $# -eq 0 ] && set -- enhanced/tests/drive/*.txt
failed=0
mkdir -p build/drive-tests
for script in "$@"; do
  name=$(basename "$script" .txt)
  out=build/drive-tests/$name
  rm -rf "$out"
  level=$(sed -n 's/^# Level \([0-9]*\).*/\1/p' "$script" | head -1)
  if env -u LD_LIBRARY_PATH "$build/th_enhanced" --level "${level:-1}" --drive "$script" --out "$out" >"$out.log" 2>&1; then
    echo "pass $name"
  else
    echo "FAIL $name (see $out.log)"
    failed=1
  fi
done
exit "$failed"

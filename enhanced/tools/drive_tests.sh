#!/bin/sh
# Runs the drive scripts in enhanced/tests/drive (or the ones given) through
# th_enhanced --drive, headless, and reports each; exits non-zero if any
# fails. A "# Level N" line in a script's header picks the level (default 1);
# a "# NoLevel" line starts at the start-up screens instead (the main menu); a
# "# Size WxH" line picks the window size (default 1280x720; the menu bar is
# only at the top in the game's 640x480 layout); a "# Seed DIR" line copies
# DIR's saves into the run's save folder first. A script's debug reports go
# under its own output folder. Run from the repository root after building
# build/enhanced (or the build tree in DRIVE_BUILD). Screenshots and dumps go
# to build/drive-tests/.
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
  size=$(sed -n 's/^# Size \([0-9]*x[0-9]*\).*/\1/p' "$script" | head -1)
  size_arg=""
  [ -n "$size" ] && size_arg="--size $size"
  # A "# NoLevel" header starts at the start-up screens (no --level).
  nolevel=$(sed -n 's/^# NoLevel.*/1/p' "$script" | head -1)
  level_arg="--level ${level:-1}"
  [ -n "$nolevel" ] && level_arg=""
  # A "# Seed DIR" header copies DIR's saves into the run's save folder first
  # (a script that continues or loads another script's save in a fresh run).
  seed=$(sed -n 's/^# Seed \(.*\)/\1/p' "$script" | head -1)
  if [ -n "$seed" ]; then
    mkdir -p "$out/save"
    cp "$seed"/*.h* "$out/save/" 2>/dev/null
  fi
  # shellcheck disable=SC2086
  if env -u LD_LIBRARY_PATH TH_DEBUG_REPORT_DIR="$out/debug-reports" \
       "$build/th_enhanced" $level_arg $size_arg --drive "$script" \
       --out "$out" >"$out.log" 2>&1; then
    echo "pass $name"
  else
    echo "FAIL $name (see $out.log)"
    failed=1
  fi
done
exit "$failed"

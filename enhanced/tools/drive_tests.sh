#!/bin/sh
# Runs the drive scripts in enhanced/tests/drive (or the ones given) through
# th_enhanced --drive, headless, and reports each; exits non-zero if any
# fails. A "# Level N" line in a script's header picks the level (default 1);
# a "# NoLevel" line starts at the start-up screens instead (the main menu); a
# "# Size WxH" line picks the window size (default 1280x720; the menu bar is
# only at the top in the game's 640x480 layout); a "# SeedScript NAME" line
# first runs enhanced/tests/drive/NAME.txt into a folder of its own and copies
# its saves into the run's save folder; a "# DefaultSpeed" line passes
# --default-speed (the speed a normal start has: Max). A script's debug reports
# go under its own output folder. Run from the repository root after building
# build/enhanced (or the build tree in DRIVE_BUILD). Screenshots and dumps go
# to build/drive-tests/.
set -u
build=${DRIVE_BUILD:-build/enhanced}
dir=enhanced/tests/drive

# run_script SCRIPT OUT [SEEDOUT]: one driven run of SCRIPT into a fresh OUT
# (log in OUT.log), its save folder starting with SEEDOUT's saves if given;
# succeeds if the run does.
run_script() {
  script=$1
  out=$2
  rm -rf "$out"
  level=$(sed -n 's/^# Level \([0-9]*\).*/\1/p' "$script" | head -1)
  size=$(sed -n 's/^# Size \([0-9]*x[0-9]*\).*/\1/p' "$script" | head -1)
  size_arg=""
  [ -n "$size" ] && size_arg="--size $size"
  # A "# NoLevel" header starts at the start-up screens (no --level).
  nolevel=$(sed -n 's/^# NoLevel.*/1/p' "$script" | head -1)
  level_arg="--level ${level:-1}"
  speed_arg=""
  grep -q "^# DefaultSpeed" "$script" && speed_arg="--default-speed"
  [ -n "$nolevel" ] && level_arg=""
  if [ $# -ge 3 ]; then
    mkdir -p "$out/save"
    cp "$3"/save/*.h* "$out/save/" 2>/dev/null
  fi
  # A "# SeedReplay LEVEL SCRIPT TURNS" header plays a replay script to that
  # turn with th_replay and starts the run with its end state in slot 1, so
  # that the test needs no saved game kept in the repository. The save is
  # named for the player of a --level run, the default name (misc text 1,
  # HORZA in English; App::player_name).
  seedreplay=$(sed -n 's/^# SeedReplay \(.*\)/\1/p' "$script" | head -1)
  if [ -n "$seedreplay" ]; then
    mkdir -p "$out/save"
    set -- $seedreplay
    env -u LD_LIBRARY_PATH "$build/th_replay" --level "$1" --script "$2" --turns "$3" \
      --out /dev/null --save "$out/save/HORZA.h1" >/dev/null 2>"$out.seed.log" \
      || { echo "FAIL $name (seed replay, see $out.seed.log)"; failed=1; continue; }
  fi
  # shellcheck disable=SC2086
  env -u LD_LIBRARY_PATH TH_DEBUG_REPORT_DIR="$out/debug-reports" \
    "$build/th_enhanced" $level_arg $size_arg $speed_arg --drive "$script" \
    --out "$out" >"$out.log" 2>&1
}

[ $# -eq 0 ] && set -- $dir/*.txt
failed=0
mkdir -p build/drive-tests
for script in "$@"; do
  name=$(basename "$script" .txt)
  out=build/drive-tests/$name
  # A script that continues or loads another script's save in a fresh run
  # names the producer; the producer runs here, so the result does not depend
  # on the order the scripts run in or on their output folders.
  seed=$(sed -n 's/^# SeedScript \(.*\)/\1/p' "$script" | head -1)
  if [ -z "$seed" ]; then
    run_script "$script" "$out"
  elif run_script "$dir/$seed.txt" "$out-seed"; then
    run_script "$script" "$out" "$out-seed"
  else
    echo "seed script $seed failed (see $out-seed.log)" >&2
    false
  fi
  # shellcheck disable=SC2181
  if [ $? -eq 0 ]; then
    echo "pass $name"
  else
    echo "FAIL $name (see $out.log)"
    failed=1
  fi
done
exit "$failed"

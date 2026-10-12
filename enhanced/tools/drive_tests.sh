#!/bin/sh
# Runs the drive scripts in enhanced/tests/drive (or the ones given) through
# th_enhanced --drive, headless, and reports each; exits non-zero if any
# fails. A "# Level N" line in a script's header picks the level (default 1);
# a "# NoLevel" line starts at the start-up screens instead (the main menu); a
# "# Size WxH" line picks the window size (default 1280x720; the menu bar is
# only at the top in the game's 640x480 layout); a "# SeedScript NAME" line
# first runs enhanced/tests/drive/NAME.txt into a folder of its own and copies
# its saves into the run's save folder; a "# DefaultSpeed" line passes
# --default-speed (the speed a normal start has: Max); a "# Fades" line passes
# --fades (the palette fades between screens, which a driven run skips); a
# "# Upscale SPEC" line passes --upscale SPEC (xbrz:2, nearest:2: the
# install-time upscaling); a "# Compare SPEC FILE..." line also runs the
# script with --upscale SPEC and fails unless each FILE it wrote is byte for
# byte the 1x run's. A script's debug reports go under its own output folder.
# Run from the repository root after building build/enhanced (or the build
# tree in DRIVE_BUILD). Screenshots and dumps go to build/drive-tests/<pid>/, a
# folder per run so that two runs in one tree do not clash; it is removed
# when every script passes and kept, with its logs, when one fails. A
# script whose seed script or seed replay fails fails without its own run.
set -u
build=${DRIVE_BUILD:-build/enhanced}
dir=enhanced/tests/drive
base=build/drive-tests/$$

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
  grep -q "^# Fades" "$script" && speed_arg="$speed_arg --fades"
  [ -n "$nolevel" ] && level_arg=""
  upscale=${UPSCALE_SPEC:-$(sed -n 's/^# Upscale \(.*\)/\1/p' "$script" | head -1)}
  upscale_arg=""
  [ -n "$upscale" ] && upscale_arg="--upscale $upscale"
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
      || { echo "seed replay failed (see $out.seed.log)" >&2; return 1; }
  fi
  # shellcheck disable=SC2086
  env -u LD_LIBRARY_PATH TH_DEBUG_REPORT_DIR="$out/debug-reports" \
    "$build/th_enhanced" $level_arg $size_arg $speed_arg $upscale_arg --drive "$script" \
    --out "$out" >"$out.log" 2>&1
}

# compare_run SCRIPT OUT: for a "# Compare SPEC FILE..." header, runs SCRIPT
# again with --upscale SPEC into OUT-upscaled and succeeds if each FILE is
# byte for byte the one OUT holds; with no such header, succeeds.
compare_run() {
  spec=$(sed -n 's/^# Compare \([^ ]*\) .*/\1/p' "$1" | head -1)
  [ -z "$spec" ] && return 0
  files=$(sed -n 's/^# Compare [^ ]* \(.*\)/\1/p' "$1" | head -1)
  (UPSCALE_SPEC=$spec; run_script "$1" "$2-upscaled") || return 1
  for file in $files; do
    if ! cmp -s "$2/$file" "$2-upscaled/$file"; then
      echo "$file differs at --upscale $spec" >&2
      return 1
    fi
  done
}

[ $# -eq 0 ] && set -- $dir/*.txt
failed=0
mkdir -p "$base"
for script in "$@"; do
  name=$(basename "$script" .txt)
  out=$base/$name
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
  if [ $? -eq 0 ] && compare_run "$script" "$out"; then
    echo "pass $name"
  else
    echo "FAIL $name (see $out.log)"
    failed=1
  fi
done
[ "$failed" = 0 ] && rm -rf "$base"
exit "$failed"

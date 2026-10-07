#!/bin/sh
# Runs the replay scripts against the instrumented original and this engine
# (enhanced/tools/replay/orig_trace.sh and th_replay), checks that each pair
# of traces agrees (compare.py) and that each script's "# Expect" header
# lines hold on both traces (check_expect.py), and reports one line per
# script. Exits non-zero if any script disagrees or fails an expectation.
#
# Usage (from the repository root):
#   enhanced/tools/replay/replay_all.sh [SCRIPT ...]
#     SCRIPT  a script's name (in tools/replay/scripts) or a path; without
#             any, every script in tools/replay/scripts.
#
# Environment:
#   BUILD        the enhanced build directory (default build/enhanced)
#   JOBS         scripts run at once (default 4)
#   TURNS        the last turn for a script without "# Turns" (default 18300)
#   RAND         1 also traces every random draw (default 1)
#   KEEP         1 keeps every trace and replay folder, for debugging
#                (default: a passing script's traces go as soon as they
#                agree and its expectations hold; a failing script's two
#                traces are kept, gzipped, with their paths printed)
#   WATCHDOG     minutes without new original-side trace output before a
#                run is killed as hung and its script marked failed
#                (default 5; 0 disables)
#   REPLAY_BASE  where the per-script replay folders and traces go
#                (default build/replay-all-<pid>, so two runs at once do
#                not clash; set it to share one)
#   SKIP         calls left out of the original's trace, comma-separated
#                (default below: the functions the engine does not port yet)
#
# Needs, on PATH: python3 (with the tools the decomp's dev shell has),
# dosbox-x, wibo and the Watcom tools, and the decompilation built in
# build/base; the game data in orig/. Runs inside the decompilation's dev
# shell (`nix develop path:./nix`) with dosbox-x on PATH.
set -u

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
BUILD=${BUILD:-build/enhanced}
JOBS=${JOBS:-4}
TURNS=${TURNS:-18300}
RAND=${RAND:-1}
KEEP=${KEEP:-0}
WATCHDOG=${WATCHDOG:-5}
REPLAY_BASE=${REPLAY_BASE:-build/replay-all-$$}
# The calls the engine does not port yet, left out on the original's side
# (also the list in AGENTS.md's "Replay check").
SKIP=${SKIP:-computer_action,manage_rats,generate_disaster}

rand_flag=
[ "$RAND" = 1 ] && rand_flag=--rand

for tool in python3 dosbox-x gzip; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "replay_all.sh: $tool is not on PATH (see the header)" >&2
    exit 2
  }
done
[ -x "$root/$BUILD/th_replay" ] || {
  echo "replay_all.sh: $root/$BUILD/th_replay is missing: build it first" >&2
  exit 2
}
[ -d "$root/build/base" ] || {
  echo "replay_all.sh: build/base is missing: build the decompilation first (ninja)" >&2
  exit 2
}

# One line per script, plus a summary. `scripts` is the list of paths.
scripts=
if [ "$#" -eq 0 ]; then
  for f in "$here"/scripts/*.txt; do
    scripts="$scripts $f"
  done
else
  for a in "$@"; do
    if [ -f "$a" ]; then
      f=$a
    elif [ -f "$here/scripts/$a.txt" ]; then
      f=$here/scripts/$a.txt
    elif [ -f "$here/scripts/$a" ]; then
      f=$here/scripts/$a
    else
      echo "replay_all.sh: no such script: $a" >&2
      exit 2
    fi
    scripts="$scripts $f"
  done
fi

cd "$root"
mkdir -p "$REPLAY_BASE"
RESULTS=$(mktemp -d "$REPLAY_BASE/.results.XXXXXX")
trap 'rm -rf "$RESULTS"' EXIT

# Sends `sig` (default TERM) to `pid` and every process below it, so a hung
# original's DOSBox and its slot holder are stopped without touching another
# run's; `ps --ppid` keeps it to this run's tree.
kill_tree() (
  pid=$1 sig=${2:-TERM}
  for c in $(ps -o pid= --ppid "$pid" 2>/dev/null); do
    kill_tree "$c" "$sig"
  done
  kill -"$sig" "$pid" 2>/dev/null
)

# Runs one script on both sides, compares the traces and checks its
# expectations; writes its result line and status to $RESULTS. A passing
# script's traces are deleted (unless KEEP=1); a failing script's are kept,
# gzipped.
run_script() {
  f=$1
  n=$(basename "$f" .txt)
  lvl=$(sed -n 's/^# Level \([0-9][0-9]*\).*/\1/p' "$f" | head -n 1)
  [ -n "$lvl" ] || lvl=1
  t=$(sed -n 's/^# Turns \([0-9][0-9]*\).*/\1/p' "$f" | head -n 1)
  [ -n "$t" ] || t=$TURNS
  dir=$REPLAY_BASE/$n
  mkdir -p "$dir"
  o=$dir/orig.txt
  e=$dir/enh.txt
  rm -f "$o" "$e" "$o.gz" "$e.gz"
  status=0
  line="$n (level $lvl, $t turns): agree"
  # The original side runs in the background so the watchdog can stop it if
  # the trace it writes stops growing (DOSBox hung, or never started).
  REPLAY_DIR="$dir" TRACE_RAND="$RAND" "$here/orig_trace.sh" "$o" "$t" "$f" "$SKIP" "$lvl" \
    >"$dir/orig.log" 2>&1 &
  opid=$!
  hung=0
  if [ "$WATCHDOG" -gt 0 ]; then
    trace=$dir/run/TRACE.TXT
    last_size=-1
    acquired=0
    changed=$(date +%s)
    while kill -0 "$opid" 2>/dev/null; do
      now=$(date +%s)
      # A run may wait a while for a replay slot; only once it holds one
      # (slot.sh names it in the slot file) does a silent trace mean a hang.
      if [ "$acquired" = 0 ] &&
        grep -l -F "$dir " "$root"/build/slots/replay/slot-* >/dev/null 2>&1; then
        acquired=1
        changed=$now
      fi
      if [ "$acquired" = 1 ]; then
        size=0
        [ -f "$trace" ] && size=$(wc -c <"$trace" 2>/dev/null || echo 0)
        if [ "$size" != "$last_size" ]; then
          last_size=$size
          changed=$now
        fi
        if [ "$((now - changed))" -ge "$((WATCHDOG * 60))" ]; then
          hung=1
          break
        fi
      fi
      sleep 15
    done
  fi
  if [ "$hung" = 1 ]; then
    kill_tree "$opid"
    sleep 2
    kill_tree "$opid" KILL
    wait "$opid" 2>/dev/null
    status=1
    line="$n (level $lvl, $t turns): original hung (no trace for ${WATCHDOG}m; killed; see $dir/orig.log)"
  elif ! wait "$opid"; then
    status=1
    line="$n (level $lvl, $t turns): original run failed (see $dir/orig.log)"
  fi
  if [ "$status" = 0 ] && ! "$root/$BUILD/th_replay" --script "$f" --turns "$t" $rand_flag \
    --out "$e" --level "$lvl" >"$dir/enh.log" 2>&1; then
    status=1
    line="$n (level $lvl, $t turns): enhanced run failed (see $dir/enh.log)"
  fi
  if [ "$status" = 0 ]; then
    out=$(python3 "$here/compare.py" "$o" "$e" 2>&1) || {
      status=1
      line="$n (level $lvl, $t turns): $(printf '%s\n' "$out" | head -n 1)"
    }
  fi
  if [ "$status" = 0 ]; then
    out=$(python3 "$here/check_expect.py" "$f" "$o" "$e" 2>&1) || {
      status=1
      line="$n (level $lvl, $t turns): $(printf '%s\n' "$out" | head -n 1)"
    }
  fi
  if [ "$status" = 0 ]; then
    if [ "$KEEP" != 1 ]; then
      rm -f "$o" "$e"
      rm -rf "$dir/run"
    fi
  else
    if [ "$KEEP" = 1 ]; then
      line="$line [traces:$o $e]"
    else
      traces=
      if [ -f "$o" ]; then gzip -f "$o"; traces="$o.gz"; fi
      if [ -f "$e" ]; then gzip -f "$e"; traces="$traces $e.gz"; fi
      [ -n "$traces" ] && line="$line [traces:$traces]"
      rm -rf "$dir/run"
    fi
  fi
  printf '%s\n' "$line" >"$RESULTS/$n.line"
  printf '%s\n' "$status" >"$RESULTS/$n.status"
  return "$status"
}

running=0
for f in $scripts; do
  run_script "$f" &
  running=$((running + 1))
  if [ "$running" -ge "$JOBS" ]; then
    wait
    running=0
  fi
done
wait

passed=0
failed=0
for f in $scripts; do
  n=$(basename "$f" .txt)
  [ -f "$RESULTS/$n.line" ] && printf '%s\n' "$(cat "$RESULTS/$n.line")"
  if [ "$(cat "$RESULTS/$n.status" 2>/dev/null)" = 0 ]; then
    passed=$((passed + 1))
    if [ "$KEEP" != 1 ]; then
      rm -rf "$REPLAY_BASE/$n"
    fi
  else
    failed=$((failed + 1))
  fi
done
rmdir "$REPLAY_BASE" 2>/dev/null
echo "---"
echo "$passed passed, $failed failed"
[ "$failed" = 0 ]

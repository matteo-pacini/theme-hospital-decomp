#!/bin/sh
# Runs the replay scripts against the instrumented original and this engine
# (enhanced/tools/replay/cached_orig_trace.sh, a cached orig_trace.sh, and
# th_replay), checks that each pair of traces agrees (compare.py) and that
# each script's "# Expect" header lines hold on both traces (check_expect.py),
# and reports one line per script. Exits non-zero if any script disagrees or
# fails an expectation.
#
# Usage (from the repository root):
#   enhanced/tools/replay/replay_all.sh [SCRIPT ...]
#     SCRIPT  a script's name (in tools/replay/scripts) or a path; without
#             any, every script in tools/replay/scripts.
#   enhanced/tools/replay/trace_cache.sh status|prune  cache size and pruning
#
# Environment:
#   BUILD        the enhanced build directory (default build/enhanced)
#   JOBS         scripts run at once (default 4)
#   TURNS        the last turn for a script without "# Turns" (default 18300)
#   RAND         1 also traces every random draw (default 1)
#   NO_CACHE     1 runs the original fresh (and refreshes the cache entry)
#                instead of using a cached trace (default 0); the cached
#                original trace is a valid reference unless the script, its
#                options, or the instrumented executable changed
#   KEEP         1 keeps every trace and replay folder, for debugging
#                (default: a passing script's traces go as soon as they
#                agree and its expectations hold; a failing script's two
#                traces are kept, gzipped, with their paths printed)
#   WATCHDOG     minutes without new original-side trace output before a
#                fresh run is killed as hung and its script marked failed
#                (default 5; 0 disables; cache hits never run DOSBox)
#   REPLAY_BASE  where the per-script replay folders and traces go
#                (default build/replay-all-<pid>, so two runs at once do
#                not clash; set it to share one)
#   IGNORE_EVENTS  event kinds compare.py leaves out on both sides,
#                comma-separated (default empty: every event is compared)
#   SKIP         calls left out of the original's trace, comma-separated
#                (default in lib.sh: empty, every call is in the trace)
#
# Needs, on PATH: python3 (with the tools the decomp's dev shell has),
# dosbox-x, wibo and the Watcom tools, and the decompilation built in
# build/base; the game data in orig/. Runs inside the decompilation's dev
# shell (`nix develop path:./nix`) with dosbox-x on PATH.
set -u

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
# Defaults (SKIP, TURNS, RAND, WATCHDOG), script headers, the watchdog: lib.sh,
# shared with trace_cache.sh prefetch.
# shellcheck source=lib.sh
. "$here/lib.sh"
BUILD=${BUILD:-build/enhanced}
JOBS=${JOBS:-4}
KEEP=${KEEP:-0}
REPLAY_BASE=${REPLAY_BASE:-build/replay-all-$$}

rand_flag=
[ "$RAND" = 1 ] && rand_flag=--rand
ignore_flags=
for kind in $(printf '%s' "$IGNORE_EVENTS" | tr ',' ' '); do
  ignore_flags="$ignore_flags --ignore-event $kind"
done

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

cd "$root" || exit 2
mkdir -p "$REPLAY_BASE"
RESULTS=$(mktemp -d "$REPLAY_BASE/.results.XXXXXX")
trap 'rm -rf "$RESULTS"' EXIT

# Runs one script on both sides, compares the traces and checks its
# expectations; writes its result line and status to $RESULTS. A passing
# script's traces are deleted (unless KEEP=1); a failing script's are kept,
# gzipped.
run_script() {
  f=$1
  n=$(basename "$f" .txt)
  lvl=$(script_level "$f")
  t=$(script_turns "$f")
  dir=$REPLAY_BASE/$n
  mkdir -p "$dir"
  o=$dir/orig.txt
  e=$dir/enh.txt
  rm -f "$o" "$e" "$o.gz" "$e.gz"
  status=0
  line="$n (level $lvl, $t turns): agree"
  # The original side is cached (cached_orig_trace.sh) and runs in the
  # background so the watchdog can stop a fresh run if the trace it writes
  # stops growing (DOSBox hung, or never started).
  REPLAY_DIR="$dir" TRACE_RAND="$RAND" "$here/cached_orig_trace.sh" "$o" "$t" "$f" "$SKIP" "$lvl" \
    >"$dir/orig.log" 2>&1 &
  opid=$!
  hung=0
  watch_original "$opid" "$dir" "$root" || hung=1
  if [ "$hung" = 1 ]; then
    status=1
    line="$n (level $lvl, $t turns): original hung (no trace for ${WATCHDOG}m; killed; see $dir/orig.log)"
  elif ! wait "$opid"; then
    status=1
    line="$n (level $lvl, $t turns): original run failed (see $dir/orig.log)"
  fi
  if grep -q 'cache hit' "$dir/orig.log" 2>/dev/null; then src=cached; else src=fresh; fi
  printf '%s\n' "$src" >"$RESULTS/$n.src"
  if [ "$status" = 0 ] && ! "$root/$BUILD/th_replay" --script "$f" --turns "$t" $rand_flag --skip "$SKIP" \
    --out "$e" --level "$lvl" >"$dir/enh.log" 2>&1; then
    status=1
    line="$n (level $lvl, $t turns): enhanced run failed (see $dir/enh.log)"
  fi
  if [ "$status" = 0 ]; then
    out=$(python3 "$here/compare.py" "$o" "$e" $ignore_flags 2>&1) || {
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
  printf '%s\n' "$line [original $src]" >"$RESULTS/$n.line"
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
cached=0
fresh=0
for f in $scripts; do
  n=$(basename "$f" .txt)
  [ -f "$RESULTS/$n.line" ] && printf '%s\n' "$(cat "$RESULTS/$n.line")"
  if [ "$(cat "$RESULTS/$n.src" 2>/dev/null)" = cached ]; then
    cached=$((cached + 1))
  else
    fresh=$((fresh + 1))
  fi
  if [ "$(cat "$RESULTS/$n.status" 2>/dev/null)" = 0 ]; then
    passed=$((passed + 1))
    if [ "$KEEP" != 1 ]; then
      rm -rf "${REPLAY_BASE:?}/$n"
    fi
  else
    failed=$((failed + 1))
  fi
done
rmdir "$REPLAY_BASE" 2>/dev/null
echo "---"
echo "$passed passed, $failed failed"
echo "original trace: $cached from cache, $fresh fresh"
[ "$failed" = 0 ]

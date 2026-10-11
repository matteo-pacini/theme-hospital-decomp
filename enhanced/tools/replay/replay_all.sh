#!/bin/sh
# Runs the replay scripts against the instrumented original and this engine
# (enhanced/tools/replay/cached_orig_trace.sh, a cached orig_trace.sh, and
# th_replay --advisor), checks that each pair of traces agrees (compare.py),
# that the advisor's "# advice" lines agree (original's seed suffix left off)
# and that each script's "# Expect" header lines hold on both traces
# (check_expect.py), and reports one line per script. Exits non-zero if any
# script disagrees or fails an expectation.
#
# A pass is stored as a verdict (port_cache.sh, build/replay-cache/port/),
# keyed by everything that decides it: the port's sources, its build
# configuration, these checking tools and the original's trace key for the
# script. A script whose current key has a stored verdict is not run at all,
# neither side, and prints the stored line; the summary counts them. A failure
# is never stored.
#
# Usage (from any directory):
#   enhanced/tools/replay/replay_all.sh [--orig-only] [SCRIPT ...]
#     SCRIPT  a script's name (in tools/replay/scripts) or a path; without
#             any, every script in tools/replay/scripts.
#     --orig-only  runs only the original's side, filling the cache (what
#             trace_cache.sh prefetch does): no th_replay, no comparison.
#   enhanced/tools/replay/trace_cache.sh status|prune|missing|prefetch
#
# Environment:
#   BUILD        the enhanced build directory (default build/enhanced)
#   JOBS         scripts run at once (default TH_REPLAY_SLOTS, else 10)
#   TURNS        the last turn for a script without "# Turns" (default 18300)
#   RAND         1 also traces every random draw (default 1)
#   TRACE_DRAW   1 also draws the original's view and copies its screen
#                (default 0; passed to the original's side, part of the cache key)
#   CACHE        use (default): a cached original trace is the reference and a
#                script without one runs DOSBox and stores the result; only:
#                never starts DOSBox, a script without a cache entry is
#                listed as "no cache" (neither passed nor failed; not
#                compared) and the run exits 3 (1 if anything failed), so it
#                never reads as a full pass; refresh: runs every original
#                fresh and replaces its entry (only when the cache is
#                suspected stale; it also runs the port again, as PORT_CACHE=off
#                does). After a decompilation or instrument.py change every
#                key is new and nothing is cached, so `only` finds nothing
#                and fresh originals are needed.
#   PORT_CACHE   use (default): a script with a stored port verdict for the
#                current key passes without a run; off: the port side runs
#                again and a pass rewrites the verdict. With CACHE=refresh
#                the verdict is not read either.
#   KEEP         1 keeps every trace and replay folder, for debugging
#                (default: a passing script's traces go as soon as they
#                agree and its expectations hold; a failing script's two
#                traces are kept, zstd-compressed, with their paths printed)
#   WATCHDOG     minutes without new original-side trace output before a
#                fresh run is killed as hung and its script marked failed
#                (default 5; 0 disables; cache hits never run DOSBox)
#   REPLAY_BASE  where the per-script replay folders and traces go
#                (default build/replay-all-<pid>, so two runs at once do
#                not clash; set it to share one)
# DOSBox runs in one of TH_REPLAY_SLOTS (default 10) slots shared by every
# caller (orig_trace.sh).
#
# Exit codes: 0 all passed, 1 a script disagreed or failed, 2 bad usage or a
# missing tool, 3 nothing disagreed but some script had no cached original
# (CACHE=only).
#
# Needs, on PATH: python3, zstd (with the tools the decomp's dev shell has),
# dosbox-x (not for CACHE=only), wibo and the Watcom tools, and the
# decompilation built in build/base; the game data in orig/. Runs inside the
# decompilation's dev shell (`nix develop path:./nix`).
set -u

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
# Defaults (TURNS, RAND, WATCHDOG, CACHE), script lookups, the watchdog:
# lib.sh, shared with cached_orig_trace.sh and trace_cache.sh.
# shellcheck source=lib.sh
. "$here/lib.sh"
BUILD=${BUILD:-build/enhanced}
JOBS=${JOBS:-${TH_REPLAY_SLOTS:-10}}
KEEP=${KEEP:-0}
REPLAY_BASE=${REPLAY_BASE:-build/replay-all-$$}

orig_only=0
if [ "${1:-}" = --orig-only ]; then
  orig_only=1
  shift
fi

rand_flag=
[ "$RAND" = 1 ] && rand_flag=--rand

need="python3 zstd"
[ "$orig_only" = 1 ] && need=zstd
[ "$CACHE" != only ] && need="$need dosbox-x"
for tool in $need; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "replay_all.sh: $tool is not on PATH (see the header)" >&2
    exit 2
  }
done
[ "$orig_only" = 1 ] || [ -x "$root/$BUILD/th_replay" ] || {
  echo "replay_all.sh: $root/$BUILD/th_replay is missing: build it first" >&2
  exit 2
}
[ -d "$root/build/base" ] || {
  echo "replay_all.sh: build/base is missing: build the decompilation first (ninja)" >&2
  exit 2
}

# One line per script, plus a summary. `scripts` is the list of paths.
scripts=$(script_list "$@") || exit 2

cd "$root" || exit 2
# shellcheck source=port_cache.sh
. "$here/port_cache.sh"
use_verdicts=0
if [ "$orig_only" = 0 ]; then
  port_cache_init "$BUILD" || echo "replay_all.sh: no executable key (build/base): port verdicts are not used" >&2
  [ "$PORT_CACHE" = use ] && [ "$CACHE" != refresh ] && use_verdicts=1
  [ "$PORT_BIN_OK" = 1 ] ||
    echo "replay_all.sh: $BUILD/th_replay is older than its sources: port verdicts are read, not stored" >&2
fi
mkdir -p "$REPLAY_BASE"
RESULTS=$(mktemp -d "$REPLAY_BASE/.results.XXXXXX")
trap 'rm -rf "$RESULTS"' EXIT

# advice_diff ORIG ENH: succeeds when the two traces' "# advice" lines
# agree; otherwise prints how they differ. The original appends its random
# seed to each line, the port does not.
advice_diff() {
  grep -a '^# advice ' "$1" | tr -d '\r' | sed 's/ seed=[0-9a-f]*$//' >"$1.adv"
  grep -a '^# advice ' "$2" >"$2.adv"
  cmp -s "$1.adv" "$2.adv" && return 0
  echo "advice differs (original $(wc -l <"$1.adv"), port $(wc -l <"$2.adv"); first: $(diff "$1.adv" "$2.adv" | sed -n '/^[<>]/{p;q;}'))"
  return 1
}

# Runs one script on both sides, compares the traces and checks its
# expectations; writes its result line and status to $RESULTS. A passing
# script's traces are deleted (unless KEEP=1); a failing script's are kept,
# compressed.
run_script() {
  f=$1
  n=$(basename "$f" .txt)
  lvl=$(script_level "$f")
  t=$(script_turns "$f")
  if [ "$use_verdicts" = 1 ] && stored=$(port_get native "$f"); then
    printf '%s\n' "$stored" >"$RESULTS/$n.line"
    printf 'verdict\n' >"$RESULTS/$n.src"
    printf '0\n' >"$RESULTS/$n.status"
    return 0
  fi
  dir=$REPLAY_BASE/$n
  mkdir -p "$dir"
  o=$dir/orig.txt
  e=$dir/enh.txt
  rm -f "$o" "$e" "$o.zst" "$e.zst"
  status=0
  line="$n (level $lvl, $t turns): agree"
  # The original side is cached (cached_orig_trace.sh) and runs in the
  # background so the watchdog can stop a fresh run if the trace it writes
  # stops growing (DOSBox hung, or never started).
  REPLAY_DIR="$dir" TRACE_RAND="$RAND" TRACE_DRAW="${TRACE_DRAW:-0}" "$here/cached_orig_trace.sh" "$o" "$t" "$f" "" "$lvl" \
    >"$dir/orig.log" 2>&1 &
  opid=$!
  hung=0
  watch_original "$opid" "$dir" "$root" || hung=1
  if [ "$hung" = 1 ]; then
    status=1
    line="$n (level $lvl, $t turns): original hung (no trace for ${WATCHDOG}m; killed; see $dir/orig.log)"
  else
    wait "$opid"
    ocode=$?
    if [ "$CACHE" = only ] && [ "$ocode" != 0 ] &&
      grep -q "^orig trace: no cache" "$dir/orig.log" 2>/dev/null; then
      printf '%s\n' "$n (level $lvl, $t turns): no cache (not compared)" >"$RESULTS/$n.line"
      printf 'nocache\n' >"$RESULTS/$n.status"
      return 0
    elif [ "$ocode" != 0 ]; then
      status=1
      line="$n (level $lvl, $t turns): original run failed (see $dir/orig.log)"
    fi
  fi
  if grep -q 'cache hit' "$dir/orig.log" 2>/dev/null; then src=cached; else src=fresh; fi
  printf '%s\n' "$src" >"$RESULTS/$n.src"
  if [ "$orig_only" = 1 ]; then
    [ "$status" = 0 ] && line="$n (level $lvl, $t turns): original trace ready"
  elif [ "$status" = 0 ] && ! "$root/$BUILD/th_replay" --script "$f" --turns "$t" $rand_flag --advisor \
    --out "$e" --level "$lvl" >"$dir/enh.log" 2>&1; then
    status=1
    line="$n (level $lvl, $t turns): enhanced run failed (see $dir/enh.log)"
  fi
  if [ "$orig_only" = 0 ] && [ "$status" = 0 ]; then
    out=$(python3 "$here/compare.py" "$o" "$e" 2>&1) || {
      status=1
      line="$n (level $lvl, $t turns): $(printf '%s\n' "$out" | head -n 1)"
    }
  fi
  if [ "$orig_only" = 0 ] && [ "$status" = 0 ]; then
    out=$(advice_diff "$o" "$e") || {
      status=1
      line="$n (level $lvl, $t turns): $out"
    }
  fi
  if [ "$orig_only" = 0 ] && [ "$status" = 0 ]; then
    out=$(python3 "$here/check_expect.py" "$f" "$o" "$e" 2>&1) || {
      status=1
      line="$n (level $lvl, $t turns): $(printf '%s\n' "$out" | head -n 1)"
    }
  fi
  if [ "$status" = 0 ]; then
    if [ "$orig_only" = 0 ]; then
      expect=$(python3 "$here/check_expect.py" --verbose "$f" "$o" "$e" 2>&1 | sed 's|^[^:]*/\([a-z]*\)\.txt: |\1: |')
      port_put native "$f" "" "$line [original $src]" "$(sha256sum <"$e" | cut -c1-64)" "$expect"
    fi
    if [ "$KEEP" != 1 ]; then
      rm -f "$o" "$e" "$o.adv" "$e.adv"
      rm -rf "$dir/run"
    fi
  else
    if [ "$KEEP" = 1 ]; then
      line="$line [traces:$o $e]"
    else
      traces=
      rm -f "$o.adv" "$e.adv"
      if [ -f "$o" ]; then zstd -q -f --rm "$o"; traces="$o.zst"; fi
      if [ -f "$e" ]; then zstd -q -f --rm "$e"; traces="$traces $e.zst"; fi
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
nocache=0
nocache_list=
cached=0
fresh=0
verdicts=0
for f in $scripts; do
  n=$(basename "$f" .txt)
  [ -f "$RESULTS/$n.line" ] && printf '%s\n' "$(cat "$RESULTS/$n.line")"
  if [ "$(cat "$RESULTS/$n.status" 2>/dev/null)" = nocache ]; then
    nocache=$((nocache + 1))
    nocache_list="$nocache_list $n"
    rm -rf "${REPLAY_BASE:?}/$n"
    continue
  fi
  src=$(cat "$RESULTS/$n.src" 2>/dev/null)
  if [ "$src" = verdict ]; then
    verdicts=$((verdicts + 1))
  elif [ "$src" = cached ]; then
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
echo "$passed passed ($verdicts cached, $((passed - verdicts)) run), $failed failed"
echo "original trace: $cached from cache, $fresh fresh"
[ "$nocache" = 0 ] || echo "$nocache without a cached original, not compared (CACHE=only):$nocache_list"
[ "$failed" = 0 ] || exit 1
# Exit 3: nothing disagreed, but part of the check did not run, so the gate
# must not read as passed.
[ "$nocache" = 0 ] || exit 3

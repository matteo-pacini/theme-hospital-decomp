#!/bin/sh
# The original side of the replay check, with its trace cached: runs
# orig_trace.sh only when no cached trace for the same instrumented
# HOSPITAL.EXE and the same script, turn count, level, left-out calls and
# random-draw and drawing options exists, and otherwise decompresses the
# cached trace.
#
# The original's trace depends only on the instrumented executable, the
# script's bytes, the turn count, the level, the left-out calls, the trace
# options and the DOSBox run (orig_trace.sh, the original's HOSPITAL.CFG;
# trace_key.sh) — never on this engine — so a cached trace is a valid
# reference until one of those changes (a decompilation or instrument.py
# change makes a new executable cache key). Only a run that finished is
# stored: its trace ends with the "# end" line. Entries live in
# build/replay-cache/traces/<exe-key>-<rest>.zst, compressed with zstd. This
# directory is machine-local and git-ignored; never edit or hand-place its
# files. See enhanced/README.md, "Original trace cache", and
# enhanced/tools/replay/trace_cache.sh for its size, status and pruning.
#
# Usage (from the repository root): the same arguments as orig_trace.sh
#   cached_orig_trace.sh OUT TURNS [SCRIPT] [SKIP] [LEVEL]
#
# Environment:
#   CACHE  use (default): a hit is decompressed, a miss runs the original and
#          stores it; only: never runs DOSBox and takes no lock: without a
#          cached trace it prints "orig trace: no cache <key>" and exits 3,
#          OUT unwritten; refresh: runs the original fresh and replaces the
#          entry.
#   REPLAY_DIR, TRACE_RAND, TRACE_DRAW and TH_REPLAY_SLOTS are passed to
#   orig_trace.sh.
# Needs zstd on PATH. Prints one "orig trace: cache hit/miss <key>" line for
# replay_all.sh.
set -eu

out=$1 turns=$2 script=${3:-} skip=${4:-} level=${5:-1}
root=$PWD
here=$(cd "$(dirname "$0")" && pwd)
# shellcheck source=lib.sh
. "$here/lib.sh"
[ -d build/base ] || { echo "build/base is missing: build the decompilation first (ninja)" >&2; exit 1; }

exe=$("$here/exe_cache_key.sh" "$root" "$here/instrument.py")
key=$(orig_key "$exe" "$turns" "$level" "$skip" "$script")
cache=build/replay-cache/traces
entry=$cache/$key.zst
lock=$cache/$key.lock
mkdir -p "$cache"

# One lock per key, like the executable cache, so two runs of the same
# script never run DOSBox twice or write the entry at once. Held across the
# fresh run too: a waiter finds the entry of the run it waited for.
# A cache-only call takes no lock: an entry appears only by an atomic mv, so
# it reads a whole entry or none, and a fresh run holding the lock has stored
# nothing yet.
if [ "$CACHE" != only ]; then
  exec 9>"$lock"
  flock 9
fi

# Looked up under the lock, so a run that waited for another finds the entry
# it stored.
if [ "$CACHE" != refresh ] && [ -f "$entry" ]; then
  # Onto OUT by a rename, so OUT is never a half-written trace.
  tmp=$out.cache.$$
  zstd -d -q -c "$entry" >"$tmp"
  mv "$tmp" "$out"
  echo "orig trace: cache hit $key ($(basename "$entry"))"
  exit 0
fi

if [ "$CACHE" = only ]; then
  echo "orig trace: no cache $key"
  exit 3
fi

if ! "$here/orig_trace.sh" "$out" "$turns" "$script" "$skip" "$level"; then
  echo "orig trace: cache miss $key (run failed; nothing cached)" >&2
  exit 1
fi
# The instrumented game ends a run that finished (the last turn, a win or a
# loss) with a "# end" line. DOSBox exits 0 whether or not the game got that
# far, so a trace without the line is cut short and is not stored.
if [ "$(tail -n 1 "$out" | tr -d '\r')" != "# end" ]; then
  echo "orig trace: cache miss $key (run ended early; nothing cached)" >&2
  exit 1
fi

tmp=$cache/.$key.$$
zstd -19 -T0 -q -f -o "$tmp" "$out"
mv "$tmp" "$entry"
echo "orig trace: cache miss $key (stored)"

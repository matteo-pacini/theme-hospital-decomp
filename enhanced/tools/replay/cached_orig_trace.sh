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
# build/replay-cache/traces/<exe-key>-<rest>.zst, compressed with zstd when
# the dev shell has it (gzip otherwise). This directory is machine-local and
# git-ignored; never edit or hand-place its files. See enhanced/README.md,
# "Original trace cache", and enhanced/tools/replay/trace_cache.sh for its
# size, status and pruning.
#
# Usage (from the repository root): the same arguments as orig_trace.sh
#   cached_orig_trace.sh OUT TURNS [SCRIPT] [SKIP] [LEVEL]
#
# Environment: NO_CACHE=1 runs the original fresh and refreshes the entry;
# CACHED_ONLY=1 never runs DOSBox and ignores NO_CACHE: without a cached
# trace it prints "orig trace: no cache <key>" and exits 3, OUT unwritten; with
# one it behaves as usual;
# REPLAY_DIR, TRACE_RAND, TRACE_DRAW and TH_REPLAY_SLOTS are passed to
# orig_trace.sh.
# Prints one "orig trace: cache hit/miss <key>" line for replay_all.sh.
set -eu

out=$1 turns=$2 script=${3:-} skip=${4:-} level=${5:-1}
root=$PWD
here=$(cd "$(dirname "$0")" && pwd)
[ -d build/base ] || { echo "build/base is missing: build the decompilation first (ninja)" >&2; exit 1; }

exe=$("$here/exe_cache_key.sh" "$root" "$here/instrument.py")
key=$("$here/trace_key.sh" "$exe" "$turns" "$level" "$skip" "${TRACE_RAND:-0}" "${TRACE_DRAW:-0}" "$script")
cache=build/replay-cache/traces
entry=$cache/$key
lock=$cache/$key.lock
mkdir -p "$cache"

have_zstd=0
if command -v zstd >/dev/null 2>&1; then have_zstd=1; fi

# Decompresses an entry (zstd preferred) onto OUT by a rename, so OUT is
# never a half-written trace.
decompress()
{
  src=$1 dst=$2 tmp=$2.cache.$$
  case $src in
    *.zst) zstd -d -q -c "$src" >"$tmp" ;;
    *.gz) gzip -dc "$src" >"$tmp" ;;
    *) return 1 ;;
  esac
  mv "$tmp" "$dst"
}

# One lock per key, like the executable cache, so two runs of the same
# script never run DOSBox twice or write the entry at once. Held across the
# fresh run too: a waiter finds the entry of the run it waited for.
# A cache-only call takes no lock: an entry appears only by an atomic mv, so
# it reads a whole entry or none, and a fresh run holding the lock has stored
# nothing yet.
if [ "${CACHED_ONLY:-0}" != 1 ]; then
  exec 9>"$lock"
  flock 9
fi

# The cache entry to read, or empty: zstd when it is present and available,
# gzip otherwise (an entry written where zstd was missing). Looked up under
# the lock, so a run that waited for another finds the entry it stored.
lookup=
if [ "$have_zstd" = 1 ] && [ -f "$entry.zst" ]; then
  lookup=$entry.zst
elif [ -f "$entry.gz" ]; then
  lookup=$entry.gz
fi

if [ "${CACHED_ONLY:-0}" = 1 ] && [ -z "$lookup" ]; then
  echo "orig trace: no cache $key"
  exit 3
fi

if { [ "${NO_CACHE:-0}" != 1 ] || [ "${CACHED_ONLY:-0}" = 1 ]; } && [ -n "$lookup" ]; then
  decompress "$lookup" "$out"
  echo "orig trace: cache hit $key ($(basename "$lookup"))"
  exit 0
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
if [ "$have_zstd" = 1 ]; then
  zstd -19 -T0 -q -f -o "$tmp" "$out"
  mv "$tmp" "$entry.zst"
else
  gzip -c "$out" >"$tmp"
  mv "$tmp" "$entry.gz"
fi
echo "orig trace: cache miss $key (stored)"

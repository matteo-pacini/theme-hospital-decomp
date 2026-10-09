#!/bin/sh
# Prints the cache key of the instrumented HOSPITAL.EXE for a tree: a hash of
# everything the executable is built from (the decompilation's sources,
# objects and tools, configure.py and instrument.py). orig_trace.sh names its
# executable cache directory with it and the trace cache names its entries
# with it, so both share one definition.
#
# Usage: exe_cache_key.sh ROOT INSTRUMENT_PY
#   ROOT           the repository root to hash (run from anywhere)
#   INSTRUMENT_PY  path to tools/replay/instrument.py
set -eu

root=$1 instrument=$2
cd "$root"
# Only paths relative to ROOT and contents go into the hash, so a worktree
# (whose build/base is a link to the main tree's) gets the main tree's key and
# shares its caches: -H follows the build/base link, and instrument.py is
# hashed from stdin, without its absolute path.
{
  find -H src include tools config build/base -type f ! -path '*/__pycache__/*' -print0 | sort -z | xargs -0 sha256sum
  sha256sum configure.py
  sha256sum <"$instrument"
} | sha256sum | cut -c1-16

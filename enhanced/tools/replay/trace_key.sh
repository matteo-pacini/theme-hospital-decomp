#!/bin/sh
# Prints the trace cache key for one original-side replay: the instrumented
# HOSPITAL.EXE's cache key (exe_cache_key.sh) and a hash of everything else
# that changes the trace it writes — the script file's bytes, the last turn,
# the level, the left-out calls, the random-draw option, orig_trace.sh (the
# DOSBox configuration and the run folder it builds), the original's
# HOSPITAL.CFG, MODEM.INI and DOS4GW.EXE, and which dosbox-x runs it.
# cached_orig_trace.sh names its entries with it. The game data folders
# (orig/DATA, LEVELS, ...) are not hashed: they are the installed game.
#
# Usage (from the repository root): trace_key.sh EXE_KEY TURNS LEVEL SKIP RAND [SCRIPT]
set -eu

exe=$1 turns=$2 level=$3 skip=$4 rand=$5 script=${6:-}
here=$(cd "$(dirname "$0")" && pwd)

# Hashes a file's bytes, not its name, so the key does not depend on the path.
content()
{
  if [ -f "$1" ]; then sha256sum <"$1" | cut -c1-64; else echo missing; fi
}

dosbox=$(command -v dosbox-x || true)
[ -z "$dosbox" ] || dosbox=$(readlink -f "$dosbox")

rest=$(
  printf 'turns=%s\nlevel=%s\nskip=%s\nrand=%s\n' "$turns" "$level" "$skip" "$rand"
  if [ -n "$script" ]; then
    printf 'script=%s\n' "$(content "$script")"
  else
    printf 'script=none\n'
  fi
  printf 'orig_trace=%s\n' "$(content "$here/orig_trace.sh")"
  for f in HOSPITAL.CFG MODEM.INI DOS4GW.EXE; do
    printf '%s=%s\n' "$f" "$(content "orig/$f")"
  done
  printf 'dosbox=%s\n' "$dosbox"
)
rest=$(printf '%s' "$rest" | sha256sum | cut -c1-16)
printf '%s-%s\n' "$exe" "$rest"

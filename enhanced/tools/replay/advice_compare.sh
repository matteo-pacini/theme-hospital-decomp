#!/bin/sh
# Compares the advisor's advice sequence of the cached original trace (its
# "# advice <situation>" comment lines) with th_replay --advisor's, for every
# script (or the ones named). Reads cached originals only, never starts
# DOSBox: a script without a cache entry is listed as "no cache".
# Same defaults as replay_all.sh (lib.sh: TURNS, RAND, SKIP; BUILD, REPLAY_BASE).
#
# Usage (from any directory, in the decompilation's dev shell, which has zstd;
# no python3 needed):
#   advice_compare.sh [SCRIPT ...]
# Exit 0 all identical, 1 any difference, 3 any without cache (and none differ).
set -u

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
# shellcheck source=lib.sh
. "$here/lib.sh"
BUILD=${BUILD:-build/enhanced}
REPLAY_BASE=${REPLAY_BASE:-build/advice-compare-$$}
rand_flag=
[ "$RAND" = 1 ] && rand_flag=--rand

cd "$root" || exit 2
[ -x "$BUILD/th_replay" ] || { echo "advice_compare.sh: $BUILD/th_replay is missing: build it first" >&2; exit 2; }
mkdir -p "$REPLAY_BASE"
trap 'rm -rf "$REPLAY_BASE"' EXIT

if [ "$#" -eq 0 ]; then
  set -- "$here"/scripts/*.txt
else
  for a in "$@"; do
    [ -f "$a" ] || [ -f "$here/scripts/$a.txt" ] || [ -f "$here/scripts/$a" ] || { echo "advice_compare.sh: no such script: $a" >&2; exit 2; }
  done
fi

same=0 differ=0 nocache=0
for a in "$@"; do
  if [ -f "$a" ]; then f=$a; elif [ -f "$here/scripts/$a.txt" ]; then f=$here/scripts/$a.txt; else f=$here/scripts/$a; fi
  n=$(basename "$f" .txt)
  lvl=$(script_level "$f")
  t=$(script_turns "$f")
  o=$REPLAY_BASE/$n.orig
  e=$REPLAY_BASE/$n.enh
  if ! CACHED_ONLY=1 REPLAY_DIR="$REPLAY_BASE/$n.dir" TRACE_RAND="$RAND" TRACE_DRAW="${TRACE_DRAW:-0}" \
    "$here/cached_orig_trace.sh" "$o" "$t" "$f" "$SKIP" "$lvl" >/dev/null 2>&1; then
    echo "$n: no cache"
    nocache=$((nocache + 1))
    continue
  fi
  "$BUILD/th_replay" --script "$f" --turns "$t" $rand_flag --skip "$SKIP" --advisor --out "$e" --level "$lvl" >/dev/null 2>&1 || {
    echo "$n: th_replay failed"
    differ=$((differ + 1))
    continue
  }
  grep -a '^# advice ' "$o" | tr -d '\r' | sed 's/ seed=[0-9a-f]*$//' >"$o.adv"
  grep -a '^# advice ' "$e" >"$e.adv"
  if diff -q "$o.adv" "$e.adv" >/dev/null; then
    same=$((same + 1))
  else
    differ=$((differ + 1))
    echo "$n: DIFFERS (original $(wc -l <"$o.adv"), port $(wc -l <"$e.adv"))"
    diff "$o.adv" "$e.adv" | head -n 10
  fi
  rm -f "$o" "$e" "$o.adv" "$e.adv"
done
echo "advice sequences: $same identical, $differ differ, $nocache without cache"
[ "$differ" = 0 ] || exit 1
[ "$nocache" = 0 ] || exit 3

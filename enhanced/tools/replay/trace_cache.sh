#!/bin/sh
# Inspects and prunes the original trace cache
# (build/replay-cache/traces; see enhanced/README.md, "Original trace
# cache"). Entries are named <exe-key>-<rest>.zst, so an entry belongs
# to the instrumented executable its key names; entries for any other key are
# stale. This cache is machine-local and git-ignored; never edit or hand-place
# its files.
#
# Usage (from any directory):
#   trace_cache.sh status          entries, size, current key, stale count
#   trace_cache.sh prune [DAYS]    remove entries whose exe key is not the
#                                  current one, and (with DAYS) entries older
#                                  than DAYS days
#   trace_cache.sh missing [SCRIPT ...]
#                                  list (as paths) the scripts, or the ones
#                                  named, that have no cached original at the
#                                  replay_all.sh defaults (turns, level, RAND,
#                                  TRACE_DRAW)
#   trace_cache.sh prefetch [SCRIPT ...]
#                                  run the original for the scripts of
#                                  `missing` through replay_all.sh --orig-only
#                                  (its JOBS, WATCHDOG and CACHE), so a later
#                                  replay_all.sh run is all cache hits
set -eu

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
cache=$root/build/replay-cache/traces
# shellcheck source=lib.sh
. "$here/lib.sh"
mkdir -p "$cache"

current=
if [ -d "$root/build/base" ]; then
  current=$("$here/exe_cache_key.sh" "$root" "$here/instrument.py")
fi

cmd=${1:-status}
case $cmd in
  status)
    entries=0
    stale=0
    for f in "$cache"/*.zst; do
      [ -f "$f" ] || continue
      entries=$((entries + 1))
      base=${f##*/}
      if [ -n "$current" ] && [ "${base%%-*}" != "$current" ]; then stale=$((stale + 1)); fi
    done
    size=$(du -sh "$cache" 2>/dev/null | cut -f 1)
    echo "original trace cache: $cache"
    echo "entries: $entries, size: ${size:-0}"
    echo "current executable key: ${current:-unknown (build/base missing)}"
    echo "stale entries (other executable keys): $stale"
    ;;
  prune)
    days=${2:-}
    removed=0
    freed=0
    for f in "$cache"/*.zst; do
      [ -f "$f" ] || continue
      base=${f##*/}
      key=${base%.*}
      drop=0
      if [ -n "$current" ] && [ "${key%%-*}" != "$current" ]; then drop=1; fi
      if [ -n "$days" ] && [ -z "$(find "$f" -maxdepth 0 -mtime -"$days" 2>/dev/null)" ]; then drop=1; fi
      [ "$drop" = 1 ] || continue
      bytes=$(wc -c <"$f")
      # Under the entry's lock, and only when it is free: never remove an
      # entry a run is writing. Also drop its now-empty lock file.
      # shellcheck disable=SC2094 # the fd only locks; nothing reads the file
      if ( flock -n 9 && rm -f "$f" "$cache/$key.lock" ) 9>>"$cache/$key.lock"; then
        removed=$((removed + 1))
        freed=$((freed + bytes))
      fi
    done
    echo "pruned $removed entr$( [ "$removed" = 1 ] && echo y || echo ies), freed $((freed / 1024)) KiB"
    ;;
  missing)
    shift
    list=$(script_list "$@")
    # shellcheck disable=SC2086
    set -- $list
    [ -n "$current" ] || { echo "missing: build/base is missing: build the decompilation first (ninja)" >&2; exit 1; }
    cd "$root"
    for f in "$@"; do
      key=$(TRACE_RAND=$RAND orig_key "$current" "$(script_turns "$f")" "$(script_level "$f")" "" "$f")
      [ -f "$cache/$key.zst" ] || echo "$f"
    done
    ;;
  prefetch)
    shift
    todo=$("$0" missing "$@")
    if [ -z "$todo" ]; then
      echo "prefetch: every original is cached"
      exit 0
    fi
    # shellcheck disable=SC2086
    "$here/replay_all.sh" --orig-only $todo
    ;;
  *)
    echo "usage: trace_cache.sh status | prune [DAYS] | missing [SCRIPT ...] | prefetch [SCRIPT ...]" >&2
    exit 2
    ;;
esac

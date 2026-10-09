#!/bin/sh
# Inspects and prunes the original trace cache
# (build/replay-cache/traces; see enhanced/README.md, "Original trace
# cache"). Entries are named <exe-key>-<rest>.<zst|gz>, so an entry belongs
# to the instrumented executable its key names; entries for any other key are
# stale. This cache is machine-local and git-ignored; never edit or hand-place
# its files.
#
# Usage (from any directory):
#   trace_cache.sh status          entries, size, current key, stale count
#   trace_cache.sh prune [DAYS]    remove entries whose exe key is not the
#                                  current one, and (with DAYS) entries older
#                                  than DAYS days
#   trace_cache.sh prefetch [SCRIPT ...]
#                                  run the original for every script (or the
#                                  ones named), filling the cache; the same
#                                  turns/level/SKIP/RAND defaults and the
#                                  watchdog as replay_all.sh (lib.sh), so a
#                                  later replay_all.sh run is all cache hits.
#                                  PREFETCH_JOBS scripts at once (default 10)
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
    for f in "$cache"/*.zst "$cache"/*.gz; do
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
    for f in "$cache"/*.zst "$cache"/*.gz; do
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
  prefetch)
    shift
    scripts=
    if [ "$#" -eq 0 ]; then
      for s in "$root"/enhanced/tools/replay/scripts/*.txt; do scripts="$scripts $s"; done
    else
      for a in "$@"; do
        if [ -f "$a" ]; then
          case $a in /*) f=$a ;; *) f=$PWD/$a ;; esac
        elif [ -f "$root/enhanced/tools/replay/scripts/$a.txt" ]; then
          f=$root/enhanced/tools/replay/scripts/$a.txt
        elif [ -f "$root/enhanced/tools/replay/scripts/$a" ]; then
          f=$root/enhanced/tools/replay/scripts/$a
        else
          echo "prefetch: no such script: $a" >&2
          continue
        fi
        scripts="$scripts $f"
      done
    fi
    jobs=${PREFETCH_JOBS:-10}
    tmp=$root/build/replay-prefetch-$$
    mkdir -p "$tmp"
    running=0
    for f in $scripts; do
      [ -f "$f" ] || { echo "prefetch: no such script: $f" >&2; continue; }
      d=$tmp/$(basename "$f" .txt)
      mkdir -p "$d"
      (
        # Like replay_all.sh: the same arguments, and the same watchdog on
        # the original's run.
        cd "$root"
        REPLAY_DIR="$d" TRACE_RAND=$RAND TRACE_DRAW="${TRACE_DRAW:-0}" \
          "$here/cached_orig_trace.sh" "$d/out.txt" "$(script_turns "$f")" "$f" "$SKIP" "$(script_level "$f")" \
          >"$d/orig.log" 2>&1 &
        opid=$!
        hung=0
        watch_original "$opid" "$d" "$root" || hung=1
        if [ "$hung" = 1 ]; then
          echo "prefetch: $(basename "$f") hung (no trace for ${WATCHDOG}m; killed; see $d/orig.log)" >&2
        elif ! wait "$opid"; then
          echo "prefetch: $(basename "$f") failed (see $d/orig.log)" >&2
        fi
      ) &
      running=$((running + 1))
      if [ "$running" -ge "$jobs" ]; then wait; running=0; fi
    done
    wait
    rm -rf "$tmp"
    echo "prefetch done"
    ;;
  *)
    echo "usage: trace_cache.sh status | prune [DAYS] | prefetch [SCRIPT ...]" >&2
    exit 2
    ;;
esac

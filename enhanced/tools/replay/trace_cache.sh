#!/bin/sh
# Inspects and prunes the original trace cache
# (build/replay-cache/traces; see enhanced/README.md, "Original trace
# cache"). Entries are named <exe-key>-<rest>.zst, so an entry belongs
# to the instrumented executable its key names; entries for any other key are
# stale. This cache is machine-local and git-ignored; never edit or hand-place
# its files.
#
# It also covers the port verdict records (build/replay-cache/port, see
# port_cache.sh): a record is stale when it is not the current record of its
# script (a changed key, or a script that is gone). A record whose current
# key cannot be told here (no build/base, or the build folder it needs, which
# is configured by the gate, is missing) is counted as unknown and kept.
#
# Usage (from any directory):
#   trace_cache.sh status          entries, size, current key, stale count,
#                                  and the same for the port verdict records
#   trace_cache.sh prune [DAYS]    remove entries whose exe key is not the
#                                  current one, and (with DAYS) entries older
#                                  than DAYS days; remove the stale port
#                                  verdict records (and with DAYS the old ones)
#   trace_cache.sh missing [SCRIPT ...]
#                                  list (as paths) the scripts, or the ones
#                                  named, that have no cached original at the
#                                  replay_all.sh defaults (turns, level, RAND,
#                                  TRACE_DRAW) and, with PORT_CACHE=use, no
#                                  stored native port verdict: such a script
#                                  needs no original (BUILD names the build
#                                  folder, default build/enhanced)
#   trace_cache.sh native-keys [SCRIPT ...]
#                                  print `NAME KEY` for each script: the
#                                  native port verdict key (the gate's web
#                                  phase, which cannot compute it, reads it)
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
# shellcheck source=port_cache.sh
. "$here/port_cache.sh"
mkdir -p "$cache"
BUILD=${BUILD:-build/enhanced}

current=
if [ -d "$root/build/base" ]; then
  current=$("$here/exe_cache_key.sh" "$root" "$here/instrument.py")
fi

# port_state RECORD: prints current, stale or unknown (see the header).
# Needs port_cache_init to have run, in the root.
port_state()
{
  base=${1##*/}
  base=${base%.rec}
  rec_key=${base##*.}
  rest=${base%.*}
  kind=${rest%%.*}
  rest=${rest#*.}
  name=${rest%%.*}
  tag=${rest#*.}
  [ -f "$here/scripts/$name.txt" ] || { echo stale; return; }
  [ -n "$PORT_EXE_KEY" ] || { echo unknown; return; }
  if [ "$kind" = web ]; then
    [ -f "$WEBBUILD/CMakeCache.txt" ] || { echo unknown; return; }
  else
    [ -f "$BUILD/CMakeCache.txt" ] || { echo unknown; return; }
  fi
  if [ "$(port_key "$kind" "$here/scripts/$name.txt" "$tag")" = "$rec_key" ]; then echo current; else echo stale; fi
}

# port_scan: counts the port records into port_entries, port_stale and
# port_unknown, and lists the stale ones in port_stale_list.
port_scan()
{
  cd "$root"
  WEBBUILD=build/web
  port_cache_init "$BUILD" "$WEBBUILD" || true
  port_entries=0 port_stale=0 port_unknown=0 port_stale_list=
  for r in "$port_dir"/*.rec; do
    [ -f "$r" ] || continue
    port_entries=$((port_entries + 1))
    case $(port_state "$r") in
      stale)
        port_stale=$((port_stale + 1))
        port_stale_list="$port_stale_list $r"
        ;;
      unknown) port_unknown=$((port_unknown + 1)) ;;
    esac
  done
}

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
    port_scan
    size=$(du -sh "$port_dir" 2>/dev/null | cut -f 1)
    echo "port verdict cache: $port_dir"
    echo "entries: $port_entries, size: ${size:-0}"
    echo "stale entries (key not current): $port_stale, unknown: $port_unknown"
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
    port_scan
    removed=0
    for r in $port_stale_list; do
      rm -f "$r" && removed=$((removed + 1))
    done
    if [ -n "$days" ]; then
      for r in "$port_dir"/*.rec; do
        [ -f "$r" ] || continue
        [ -z "$(find "$r" -maxdepth 0 -mtime -"$days" 2>/dev/null)" ] && rm -f "$r" && removed=$((removed + 1))
      done
    fi
    echo "pruned $removed port verdict record(s)"
    ;;
  missing)
    shift
    list=$(script_list "$@")
    # shellcheck disable=SC2086
    set -- $list
    [ -n "$current" ] || { echo "missing: build/base is missing: build the decompilation first (ninja)" >&2; exit 1; }
    cd "$root"
    port_cache_init "$BUILD" || true
    for f in "$@"; do
      key=$(TRACE_RAND=$RAND orig_key "$current" "$(script_turns "$f")" "$(script_level "$f")" "" "$f")
      [ -f "$cache/$key.zst" ] && continue
      [ "$PORT_CACHE" = use ] && port_get native "$f" >/dev/null && continue
      echo "$f"
    done
    ;;
  native-keys)
    shift
    list=$(script_list "$@")
    # shellcheck disable=SC2086
    set -- $list
    cd "$root"
    port_cache_init "$BUILD" || true
    for f in "$@"; do
      key=$(port_key native "$f") || exit 1
      echo "$(basename "$f" .txt) $key"
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
    echo "usage: trace_cache.sh status | prune [DAYS] | missing [SCRIPT ...] | native-keys [SCRIPT ...] | prefetch [SCRIPT ...]" >&2
    exit 2
    ;;
esac

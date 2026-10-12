# shellcheck shell=sh
# Shared by replay_all.sh, cached_orig_trace.sh and trace_cache.sh (sourced,
# not run; the sourcing script sets `here` to this folder first), so that a
# prefetch fills exactly the cache entries replay_all.sh will hit: the one
# place of the defaults (turns, RAND, CACHE, PORT_CACHE), of the script lookups and of the
# watchdog on the original's run.

: "${here:?lib.sh is sourced by a script that sets here}"
TURNS=${TURNS:-18300}
RAND=${RAND:-1}
WATCHDOG=${WATCHDOG:-5}
# use: a cached original trace is the reference and a miss runs DOSBox and
# stores the result; only: never starts DOSBox, a miss is reported; refresh:
# runs the original fresh and replaces the entry.
CACHE=${CACHE:-use}
case $CACHE in
  use | only | refresh) ;;
  *)
    echo "CACHE must be use, only or refresh, not '$CACHE'" >&2
    exit 2
    ;;
esac
# use: a stored port verdict for the same key is a pass without running the
# port; off: the port side runs again. Either way a pass is stored
# (port_cache.sh).
PORT_CACHE=${PORT_CACHE:-use}
case $PORT_CACHE in
  use | off) ;;
  *)
    echo "PORT_CACHE must be use or off, not '$PORT_CACHE'" >&2
    exit 2
    ;;
esac

# script_path NAME|PATH: prints the absolute path of a replay script given by
# its name (in scripts/, with or without .txt) or by a path; fails if there
# is none.
script_path()
{
  if [ -f "$1" ]; then
    case $1 in /*) echo "$1" ;; *) echo "$PWD/$1" ;; esac
  elif [ -f "$here/scripts/$1.txt" ]; then
    echo "$here/scripts/$1.txt"
  elif [ -f "$here/scripts/$1" ]; then
    echo "$here/scripts/$1"
  else
    return 1
  fi
}

# script_list [NAME|PATH ...]: one script path per line, in the order given,
# or every script in scripts/ without arguments. Names no script has are
# reported on stderr and make it fail.
script_list()
{
  if [ "$#" -eq 0 ]; then
    for f in "$here"/scripts/*.txt; do echo "$f"; done
    return 0
  fi
  bad=0
  for a in "$@"; do
    script_path "$a" || {
      echo "no such script: $a" >&2
      bad=1
    }
  done
  return "$bad"
}

# orig_key EXE TURNS LEVEL SKIP SCRIPT: the cache key of one original-side
# run (trace_key.sh, with TRACE_RAND and TRACE_DRAW as the run gets them),
# EXE being exe_cache_key.sh's. Run from the repository root.
orig_key()
{
  "$here/trace_key.sh" "$1" "$2" "$3" "$4" "${TRACE_RAND:-0}" "${TRACE_DRAW:-0}" "$5"
}

# script_level SCRIPT / script_turns SCRIPT: the "# Level N" / "# Turns N"
# header values, else 1 / $TURNS. script_level appends " -bID" when the
# header has "# Balance ID" (the game's command line, which the original side
# passes as it is; th_replay's --level reads it too).
script_level()
{
  l=$(sed -n 's/^# Level \([0-9][0-9]*\).*/\1/p' "$1" | head -n 1)
  b=$(sed -n 's/^# Balance \([A-Za-z0-9_]*\).*/\1/p' "$1" | head -n 1)
  echo "${l:-1}${b:+ -b$b}"
}
script_turns()
{
  t=$(sed -n 's/^# Turns \([0-9][0-9]*\).*/\1/p' "$1" | head -n 1)
  echo "${t:-$TURNS}"
}

# Sends `sig` (default TERM) to `pid` and every process below it, so a hung
# original's DOSBox and its slot holder are stopped without touching another
# run's; `ps --ppid` keeps it to this run's tree.
kill_tree()
(
  pid=$1 sig=${2:-TERM}
  for c in $(ps -o pid= --ppid "$pid" 2>/dev/null); do
    kill_tree "$c" "$sig"
  done
  kill -"$sig" "$pid" 2>/dev/null
)

# watch_original PID DIR ROOT: waits for the background cached_orig_trace.sh
# PID (run with REPLAY_DIR=DIR) and returns 0 when it ended by itself. If it
# holds a replay slot and its trace stops growing for WATCHDOG minutes
# (0 disables), kills it and returns 1. Liveness is polled every 0.2 s, so a
# cache hit costs no wait; the trace is looked at every 15 s.
watch_original()
{
  opid=$1 dir=$2 wroot=$3
  if [ "$WATCHDOG" -gt 0 ]; then
    trace=$dir/run/TRACE.TXT
    last_size=-1
    acquired=0
    changed=$(date +%s)
    checked=$changed
    while kill -0 "$opid" 2>/dev/null; do
      now=$(date +%s)
      if [ "$((now - checked))" -ge 15 ]; then
        checked=$now
        # A run may wait a while for a replay slot; only once it holds one
        # (slot.sh names it in the slot file) does a silent trace mean a hang.
        if [ "$acquired" = 0 ] &&
          grep -l -F "$dir " "$wroot"/build/slots/replay/slot-* >/dev/null 2>&1; then
          acquired=1
          changed=$now
        fi
        if [ "$acquired" = 1 ]; then
          size=0
          [ -f "$trace" ] && size=$(wc -c <"$trace" 2>/dev/null || echo 0)
          if [ "$size" != "$last_size" ]; then
            last_size=$size
            changed=$now
          fi
          if [ "$((now - changed))" -ge "$((WATCHDOG * 60))" ]; then
            kill_tree "$opid"
            sleep 2
            kill_tree "$opid" KILL
            wait "$opid" 2>/dev/null
            return 1
          fi
        fi
      fi
      sleep 0.2
    done
  fi
  return 0
}

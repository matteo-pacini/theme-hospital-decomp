# shellcheck shell=sh
# Shared by replay_all.sh and trace_cache.sh prefetch (sourced, not run), so
# that a prefetch fills exactly the cache entries replay_all.sh will hit: the
# one place of the default SKIP list, turns and RAND, of the script header
# lookups, and of the watchdog on the original's run.

# The calls the engine does not port yet, left out on the original's side
# (also the list in AGENTS.md's "Replay check").
SKIP=${SKIP:-computer_action}
TURNS=${TURNS:-18300}
RAND=${RAND:-1}
WATCHDOG=${WATCHDOG:-5}

# script_level SCRIPT / script_turns SCRIPT: the "# Level N" / "# Turns N"
# header values, else 1 / $TURNS.
script_level()
{
  l=$(sed -n 's/^# Level \([0-9][0-9]*\).*/\1/p' "$1" | head -n 1)
  echo "${l:-1}"
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

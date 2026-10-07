#!/bin/sh
# Runs a command in one of MAX shared slots of POOL, waiting for a free one,
# so that agents running heavy work at the same time (DOSBox replays) stay
# within a limit. The slots are files under build/slots/POOL locked with
# flock: the kernel frees a slot when its holder exits, crashes or is killed.
#
#   enhanced/tools/slot.sh POOL MAX command...  run command in a slot
#   enhanced/tools/slot.sh --status [POOL]      list the slots and holders
#
# TH_SLOT_INFO, if set, is recorded beside the holder's pid for --status.
set -u

root=$(cd "$(dirname "$0")/../.." && pwd)

if [ "${1:-}" = --status ]; then
  for f in "$root"/build/slots/${2:-*}/slot-*; do
    [ -e "$f" ] || continue
    if flock -n "$f" true; then
      echo "${f#"$root"/build/slots/}: free"
    else
      echo "${f#"$root"/build/slots/}: $(cat "$f")"
    fi
  done
  exit 0
fi

if [ $# -lt 3 ]; then
  echo "usage: slot.sh POOL MAX command... | slot.sh --status [POOL]" >&2
  exit 2
fi
pool=$1 max=$2
shift 2
dir=$root/build/slots/$pool
mkdir -p "$dir"

waited=0
while :; do
  i=0
  while [ "$i" -lt "$max" ]; do
    slot=$dir/slot-$i
    exec 9>>"$slot"
    if flock -n 9; then
      break 2
    fi
    exec 9>&-
    i=$((i + 1))
  done
  if [ "$waited" = 0 ]; then
    echo "slot.sh: waiting for a free $pool slot ($max in use):" >&2
    "$0" --status "$pool" >&2
    waited=1
  fi
  sleep 2
done

printf 'pid %s since %s %s\n' "$$" "$(date +%H:%M:%S)" "${TH_SLOT_INFO:-$*}" >"$slot"
"$@"
rc=$?
: >"$slot"
exit "$rc"

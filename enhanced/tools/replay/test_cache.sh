#!/bin/sh
# Tests the original-trace cache (cached_orig_trace.sh, trace_key.sh,
# orig_trace.sh) in a scratch repository with a fake dosbox-x, so no real
# DOSBox run is needed: what is stored, what the key covers and that every
# run starts from an empty SAVE.
#
# Usage (from anywhere): [SLOW=1] enhanced/tools/replay/test_cache.sh
# SLOW=1 also waits out the watchdog (about 75 s).
set -eu

here=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/test_cache.XXXXXX")
trap 'rm -rf "$tmp"' EXIT
fails=0

check()
{
  if [ "$2" = "$3" ]; then echo "ok   $1"; else echo "FAIL $1: got '$2', want '$3'"; fails=$((fails + 1)); fi
}

# The scratch repository: the replay tools, a tiny decompilation, an
# original folder, and the instrumented executable already cached.
root=$tmp/repo
mkdir -p "$root/enhanced/tools" "$root/src" "$root/include" "$root/tools" "$root/config" \
  "$root/build/base" "$root/orig" "$tmp/bin"
cp -r "$here" "$root/enhanced/tools/replay"
cp "$here/../slot.sh" "$root/enhanced/tools/"
echo x >"$root/src/a.cpp"
echo x >"$root/include/a.h"
echo x >"$root/tools/t.py"
echo x >"$root/config/c"
echo x >"$root/configure.py"
echo x >"$root/build/base/a.o"
printf 'LANGUAGE=FRE\nSOUND=1\n' >"$root/orig/HOSPITAL.CFG"
echo m >"$root/orig/MODEM.INI"
echo d >"$root/orig/DOS4GW.EXE"
mkdir "$root/orig/DATA"
printf '0 1\n' >"$tmp/script.txt"

rp=$root/enhanced/tools/replay
exekey=$("$rp/exe_cache_key.sh" "$root" "$rp/instrument.py")
mkdir -p "$root/build/replay-cache/$exekey"
echo exe >"$root/build/replay-cache/$exekey/HOSPITAL.EXE"

# A dosbox-x that plays the game: FAKE_MODE=full writes a finished trace,
# early one cut short. It exits 0 either way, as DOSBox does, and logs each
# run and the SAVE it found.
cat >"$tmp/bin/dosbox-x" <<'FAKE'
#!/bin/sh
conf=
while [ $# -gt 0 ]; do
  if [ "$1" = -conf ]; then conf=$2; fi
  shift
done
run=$(sed -n 's/^mount c "\(.*\)"$/\1/p' "$conf")
turns=$(sed -n 's/^set TRACE_TURNS=//p' "$conf")
echo run >>"$FAKE_LOG"
echo "save: $(ls "$run/SAVE" | tr '\n' ' ')" >>"$FAKE_LOG"
echo flags >"$run/SAVE/HORZA.HDT"
echo "cfg: $(grep LANGUAGE "$run/HOSPITAL.CFG")" >>"$FAKE_LOG"
if [ "$FAKE_MODE" = early ]; then
  printf 'T 1 seed=1\nT 2 seed=2\n' >"$run/TRACE.TXT"
else
  printf 'T 1 seed=1\nT %s seed=2\n# end\n' "$turns" >"$run/TRACE.TXT"
fi
FAKE
chmod +x "$tmp/bin/dosbox-x"

FAKE_LOG=$tmp/fake.log
export FAKE_LOG PATH="$tmp/bin:$PATH" REPLAY_DIR="$tmp/work"
cd "$root"
traces=build/replay-cache/traces
runs() { grep -c '^run$' "$FAKE_LOG" || true; }
entries() { find "$traces" -name '*.zst' -o -name '*.gz' 2>/dev/null | wc -l; }
cached() { "$rp/cached_orig_trace.sh" "$@" 2>&1; }

# 1. A finished run is stored and the next call is a hit without DOSBox.
: >"$FAKE_LOG"
FAKE_MODE=full
export FAKE_MODE
cached "$tmp/out1" 100 "$tmp/script.txt" skipme 1 | grep -q 'cache miss .*stored' && r=stored || r=not
check "finished run is stored" "$r" stored
cached "$tmp/out1b" 100 "$tmp/script.txt" skipme 1 | grep -q 'cache hit' && r=hit || r=miss
check "second call hits" "$r" hit
check "second call runs no DOSBox" "$(runs)" 1
cmp -s "$tmp/out1" "$tmp/out1b" && r=same || r=differ
check "hit returns the stored trace" "$r" same

# 2. A run that ended early (DOSBox still exits 0) is not stored.
FAKE_MODE=early
n=$(entries)
cached "$tmp/out2" 200 "$tmp/script.txt" skipme 1 | grep -q 'ended early' && r=refused || r=stored
check "early run is refused" "$r" refused
check "early run leaves no entry" "$(entries)" "$n"
cached "$tmp/out2" 200 "$tmp/script.txt" skipme 1 >/dev/null && r=exit0 || r=exit1
check "early run fails the call" "$r" exit1

# 3. Every run starts from an empty SAVE, though the last one left flags.
FAKE_MODE=full
: >"$FAKE_LOG"
cached "$tmp/out3" 300 "$tmp/script.txt" skipme 1 >/dev/null
cached "$tmp/out3" 301 "$tmp/script.txt" skipme 1 >/dev/null
check "SAVE is empty at each run" "$(grep '^save:' "$FAKE_LOG" | sort -u)" "save: "

# 4. The key covers the script's bytes, not its path.
key() { "$rp/trace_key.sh" "$exekey" 100 1 skipme 0 "$@"; }
cp "$tmp/script.txt" "$tmp/copy.txt"
k=$(key "$tmp/script.txt")
check "key: same bytes, other path" "$(key "$tmp/copy.txt")" "$k"
check "key: same bytes, relative path" "$(key ../script.txt)" "$k"
echo '5 1' >>"$tmp/copy.txt"
[ "$(key "$tmp/copy.txt")" != "$k" ] && r=differs || r=same
check "key: edited script" "$r" differs

# 5. The key covers the run's other inputs.
check "key: unchanged inputs" "$(key "$tmp/script.txt")" "$k"
printf 'LANGUAGE=FRE\nSOUND=0\n' >orig/HOSPITAL.CFG
[ "$(key "$tmp/script.txt")" != "$k" ] && r=differs || r=same
check "key: orig/HOSPITAL.CFG" "$r" differs
printf 'LANGUAGE=FRE\nSOUND=1\n' >orig/HOSPITAL.CFG
check "key: HOSPITAL.CFG restored" "$(key "$tmp/script.txt")" "$k"
echo '# edit' >>"$rp/orig_trace.sh"
[ "$(key "$tmp/script.txt")" != "$k" ] && r=differs || r=same
check "key: orig_trace.sh" "$r" differs

# 6. The language is forced to ENG whatever the original's config says.
check "run uses LANGUAGE=ENG" "$(grep '^cfg:' "$FAKE_LOG" | sort -u)" "cfg: LANGUAGE=ENG"

# 7. The watchdog (lib.sh, used by replay_all.sh and prefetch): a hit costs
# no wait; with SLOW=1 a silent run holding a slot is killed (about 75 s).
. "$rp/lib.sh"
WATCHDOG=1
d=$tmp/wd
mkdir -p "$d" "$root/build/slots/replay"
true &
start=$(date +%s)
watch_original $! "$d" "$root" && r=ok || r=killed
check "watchdog: finished run returns 0" "$r" ok
check "watchdog: finished run costs under 2 s" "$(($(date +%s) - start < 2))" 1
if [ "${SLOW:-0}" = 1 ]; then
  echo "$d is held" >"$root/build/slots/replay/slot-0"
  sleep 300 &
  hung=$!
  watch_original "$hung" "$d" "$root" && r=ok || r=killed
  check "watchdog: silent run is killed" "$r" killed
  kill -0 "$hung" 2>/dev/null && r=alive || r=dead
  check "watchdog: killed run is gone" "$r" dead
fi

if [ "$fails" = 0 ]; then echo "all cache tests passed"; else echo "$fails cache test(s) failed"; exit 1; fi

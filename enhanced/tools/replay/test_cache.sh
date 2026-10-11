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
# What the port verdict key covers (port_cache.sh), with the build folders
# the gate configures.
mkdir -p "$root/enhanced/src/core" "$root/enhanced/src/app" "$root/enhanced/cmake" \
  "$root/enhanced/web" "$root/nix" "$root/build/enhanced" "$root/build/web"
echo core >"$root/enhanced/src/core/x.cpp"
echo app >"$root/enhanced/src/app/y.cpp"
echo cmake >"$root/enhanced/CMakeLists.txt"
echo deps >"$root/enhanced/cmake/deps.cmake"
echo lock >"$root/nix/flake.lock"
echo nix >"$root/nix/flake.nix"
echo shell >"$root/enhanced/web/shell.html"
printf '# the web phase\nstep web_replays\n' >"$root/enhanced/tools/check.sh"
printf 'CMAKE_BUILD_TYPE:STRING=Release\nCMAKE_CXX_FLAGS:STRING=\nTH_DEV_DATA_DIR:PATH=/somewhere\n' >"$root/build/enhanced/CMakeCache.txt"
printf 'CMAKE_BUILD_TYPE:STRING=Release\n' >"$root/build/web/CMakeCache.txt"
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
entries() { find "$traces" -name '*.zst' 2>/dev/null | wc -l; }
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
key() { "$rp/trace_key.sh" "$exekey" 100 1 skipme 0 0 "$@"; }
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
for v in "100 1 skipme 1 0" "100 2 skipme 0 0" "100 1 other 0 0" "101 1 skipme 0 0" "100 1 skipme 0 1"; do
  # shellcheck disable=SC2086
  set -- $v
  [ "$("$rp/trace_key.sh" "$exekey" "$@" "$tmp/script.txt")" != "$k" ] && r=differs || r=same
  check "key: changed turns, level, skip, rand or draw ($v)" "$r" differs
done

# orig_trace.sh: what it feeds DOSBox is in the key, its comment-only and
# blank lines are not.
cp "$rp/orig_trace.sh" "$tmp/orig_trace.sh.bak"
orig_variant()
{
  cp "$tmp/orig_trace.sh.bak" "$rp/orig_trace.sh"
  "$@" "$rp/orig_trace.sh"
}
key_vs_k() { [ "$(key "$tmp/script.txt")" != "$k" ] && echo differs || echo same; }
orig_variant sh -c 'printf "# a comment\n\n   # indented\n" >>"$1"' x
check "key: orig_trace.sh comments and blank lines" "$(key_vs_k)" same
orig_variant sed -i 's/build\/base is missing: build the decompilation first (ninja)/other wording/'
check "key: orig_trace.sh error message wording is in the key" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(\[ -d build\/base \] || { echo \)"/\1"$(rm -rf $run\/SAVE)/'
check "key: orig_trace.sh command substitution in a message" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(\[ -d build\/base \] || { echo "[^"]*"\) >&2/\1 >>"$conf"/'
check "key: orig_trace.sh message redirected elsewhere" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(  dosbox-x -nopromptfolder -fastlaunch\) \\$/\1 \\ /'
check "key: orig_trace.sh blank after a backslash" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(  dosbox-x -nopromptfolder -fastlaunch \\\)$/\1\n  # a comment/'
check "key: orig_trace.sh comment line after a continuation" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(  dosbox-x -nopromptfolder -fastlaunch \\\)$/\1\n/'
check "key: orig_trace.sh blank line after a continuation" "$(key_vs_k)" differs
orig_variant sed -i 's/^\(root=\$PWD\)$/\1 /'
check "key: orig_trace.sh trailing blank on a line" "$(key_vs_k)" differs
# A comment-looking line inside a quoted string that spans lines is data.
orig_variant sh -c 'printf "msg=\"one\n# two\n\"\n" >>"$1"' x
k_str=$(key "$tmp/script.txt")
orig_variant sh -c 'printf "msg=\"one\n# deux\n\"\n" >>"$1"' x
[ "$(key "$tmp/script.txt")" != "$k_str" ] && r=differs || r=same
check "key: orig_trace.sh comment-looking line in a multi-line string" "$r" differs
# Heredoc forms: the body stays whatever the form; a form the key does not
# understand keeps the rest of the file as written.
for form in '\\EOF' "'EOF'" '"EOF"' '-EOF'; do
  orig_variant sed -i "s/<<EOF\$/<<$form/"
  kh=$(key "$tmp/script.txt")
  sed -i 's/^pcspeaker=false$/#pcspeaker=false/' "$rp/orig_trace.sh"
  [ "$(key "$tmp/script.txt")" != "$kh" ] && r=differs || r=same
  check "key: orig_trace.sh heredoc <<$form keeps its body" "$r" differs
done
orig_variant sed -i 's/<<EOF$/<<$HEREDOC_WORD/'
kh=$(key "$tmp/script.txt")
printf '# trailing comment\n' >>"$rp/orig_trace.sh"
[ "$(key "$tmp/script.txt")" != "$kh" ] && r=differs || r=same
check "key: orig_trace.sh unknown heredoc form keeps the rest" "$r" differs
# A heredoc that starts on a continuation line (after a backslash or inside a
# multi-line string) keeps its body too: a comment-looking line in it counts.
heredoc_key()
{
  cp "$tmp/orig_trace.sh.bak" "$rp/orig_trace.sh"
  printf '%s\n%s\nEOF\n' "$1" "$2" >>"$rp/orig_trace.sh"
  key "$tmp/script.txt"
}
for start in 'cat >/dev/null \' 'x="a
b"; cat >/dev/null <<EOF'; do
  case $start in *'\') start="$start
  <<EOF" ;; esac
  [ "$(heredoc_key "$start" '# A=1')" != "$(heredoc_key "$start" '# A=2')" ] && r=differs || r=same
  check "key: orig_trace.sh heredoc after a continuation keeps its body" "$r" differs
done
orig_variant true
orig_variant sed -i 's/^cycles=max$/cycles=auto/'
check "key: orig_trace.sh dosbox.conf setting" "$(key_vs_k)" differs
orig_variant sed -i 's/^set TRACE_SKIP=/set TRACE_SKIPPED=/'
check "key: orig_trace.sh autoexec line" "$(key_vs_k)" differs
orig_variant sed -i 's/^pcspeaker=false$/#pcspeaker=false/'
check "key: orig_trace.sh comment-looking line inside the conf" "$(key_vs_k)" differs
orig_variant sed -i 's/-fastlaunch/-fastlaunch -noautoexec/'
check "key: orig_trace.sh dosbox command line" "$(key_vs_k)" differs
orig_variant sed -i 's/ln -sfn/ln -sf/'
check "key: orig_trace.sh run folder setup" "$(key_vs_k)" differs
orig_variant sed -i 's/=ENG\//=FRE\//'
check "key: orig_trace.sh language forced" "$(key_vs_k)" differs
orig_variant true
check "key: orig_trace.sh restored" "$(key_vs_k)" same

# instrument.py: its code and the EVENTS text are in the key, its comments,
# docstrings, blank lines and layout are not (needs python3; without it the
# raw text is hashed and every edit changes the key).
if command -v python3 >/dev/null 2>&1; then
  cp "$rp/instrument.py" "$tmp/instrument.py.bak"
  exe_key() { "$rp/exe_cache_key.sh" "$root" "$rp/instrument.py"; }
  inst_variant()
  {
    cp "$tmp/instrument.py.bak" "$rp/instrument.py"
    "$@" "$rp/instrument.py"
  }
  ek=$(exe_key)
  check "exe key: unchanged" "$ek" "$exekey"
  inst_variant sh -c 'printf "\n\n# a comment\n\n\n" >>"$1"; sed -i "s/^import sys\$/import sys  # trailing/" "$1"' x
  check "exe key: instrument.py comments and blank lines" "$(exe_key)" "$ek"
  inst_variant sed -i '1,/^"""/s/^"""\(.*\)$/"""Reworded: \1/'
  check "exe key: instrument.py module docstring" "$(exe_key)" "$ek"
  inst_variant sed -i 's/^import sys$/import sys\nimport  os/'
  [ "$(exe_key)" != "$ek" ] && r=differs || r=same
  check "exe key: instrument.py code" "$r" differs
  inst_variant sed -i 's/"tutorial %u"/"tutorial2 %u"/'
  [ "$(exe_key)" != "$ek" ] && r=differs || r=same
  check "exe key: instrument.py EVENTS text" "$r" differs
  inst_variant sed -i 's/^import sys$/import sys\n"""not a docstring: a statement"""\nx = 1/'
  [ "$(exe_key)" != "$ek" ] && r=differs || r=same
  check "exe key: instrument.py string statement after code" "$r" differs
  # A coding comment changes how non-ASCII source is read, so it changes the key.
  inst_variant sh -c 'printf "x = \"\303\251\"\n" >>"$1"' x
  ek_utf=$(exe_key)
  inst_variant sh -c 'printf "x = \"\303\251\"\n" >>"$1"; sed -i "1a # -*- coding: latin-1 -*-" "$1"' x
  [ "$(exe_key)" != "$ek_utf" ] && r=differs || r=same
  check "exe key: instrument.py coding comment" "$r" differs
  cp "$tmp/instrument.py.bak" "$rp/instrument.py"
  check "exe key: instrument.py restored" "$(exe_key)" "$ek"
  # A syntax error stops the key instead of hashing nothing.
  echo 'def (' >>"$rp/instrument.py"
  exe_key >/dev/null 2>&1 && r=key || r=error
  check "exe key: unparsable instrument.py is an error" "$r" error
  cp "$tmp/instrument.py.bak" "$rp/instrument.py"
fi

# The key is the same wherever the tree lives (a worktree has its own path).
cp -a "$root" "$tmp/repo-copy"
check "exe key: same in another folder" "$("$tmp/repo-copy/enhanced/tools/replay/exe_cache_key.sh" "$tmp/repo-copy" "$tmp/repo-copy/enhanced/tools/replay/instrument.py")" "$exekey"
rm -rf "$tmp/repo-copy"

# 6. The language is forced to ENG whatever the original's config says.
check "run uses LANGUAGE=ENG" "$(grep '^cfg:' "$FAKE_LOG" | sort -u)" "cfg: LANGUAGE=ENG"

# 7. The watchdog (lib.sh, used by replay_all.sh): a hit costs
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

# 8. CACHE=only never starts DOSBox: a miss is reported ("no cache", exit
# 3, nothing written or stored), a hit is used.
: >"$FAKE_LOG"
n=$(entries)
CACHE=only cached "$tmp/out8" 400 "$tmp/script.txt" skipme 1 >"$tmp/o8.txt" && r=exit0 || r=$?
check "cache-only miss exits 3" "$r" 3
grep -q '^orig trace: no cache ' "$tmp/o8.txt" && r=reported || r=silent
check "cache-only miss says 'no cache'" "$r" reported
check "cache-only miss starts no DOSBox" "$(runs)" 0
check "cache-only miss stores nothing" "$(entries)" "$n"
[ -e "$tmp/out8" ] && r=written || r=absent
check "cache-only miss writes no trace" "$r" absent
cached "$tmp/out8a" 400 "$tmp/script.txt" skipme 1 >/dev/null
: >"$FAKE_LOG"
CACHE=only cached "$tmp/out8b" 400 "$tmp/script.txt" skipme 1 | grep -q 'cache hit' && r=hit || r=miss
check "cache-only hit" "$r" hit
check "cache-only hit starts no DOSBox" "$(runs)" 0
cmp -s "$tmp/out8a" "$tmp/out8b" && r=same || r=differ
check "cache-only hit returns the stored trace" "$r" same
# A fresh run holding the key's lock has stored nothing yet; a cache-only
# call neither waits for it nor misreads it, and a stored entry is a hit
# for two readers at once, lock held or not.
lockkey=$("$rp/trace_key.sh" "$exekey" 400 1 skipme 0 0 "$tmp/script.txt")
exec 7>"$traces/$lockkey.lock"
flock 7
(CACHE=only cached "$tmp/out8d" 400 "$tmp/script.txt" skipme 1 >"$tmp/o8d.txt") &
(CACHE=only cached "$tmp/out8e" 400 "$tmp/script.txt" skipme 1 >"$tmp/o8e.txt") &
wait
exec 7>&-
cmp -s "$tmp/out8d" "$tmp/out8a" && cmp -s "$tmp/out8e" "$tmp/out8a" && r=hits || r=lost
check "cache-only: two readers at once, lock held, both hit" "$r" hits
unheld=$("$rp/trace_key.sh" "$exekey" 410 1 skipme 0 0 "$tmp/script.txt")
exec 7>"$traces/$unheld.lock"
flock 7
CACHE=only cached "$tmp/out8c" 410 "$tmp/script.txt" skipme 1 | grep -q 'no cache' && r=reported || r=waited
exec 7>&-
check "cache-only does not wait for a key being run" "$r" reported

# 9. replay_all.sh with CACHE=only: a script with a cached original is
# compared, one without is listed as "no cache" and does not fail the run,
# and no DOSBox is started.
cat >"$root/build/enhanced/th_replay" <<'FAKE'
#!/bin/sh
echo portrun >>"$FAKE_LOG"
while [ $# -gt 0 ]; do
  case $1 in
    --turns) turns=$2 ;;
    --out) out=$2 ;;
  esac
  shift
done
printf 'T 1 seed=1\n' >"$out"
[ -z "${FAKE_ADVICE:-}" ] || echo '# advice 3' >>"$out"
printf 'T %s seed=2\n# end\n' "$turns" >>"$out"
FAKE
chmod +x "$root/build/enhanced/th_replay"
printf '# Turns 50\n0 1\n' >"$rp/scripts/zz_hit.txt"
printf '# Turns 51\n0 1\n' >"$rp/scripts/zz_miss.txt"
# Primed with replay_all.sh's defaults (lib.sh: RAND=1, no calls left out), or the
# key differs and zz_hit misses.
TRACE_RAND=1 REPLAY_DIR="$tmp/prime" "$rp/cached_orig_trace.sh" "$tmp/prime.txt" 50 "$rp/scripts/zz_hit.txt" "" 1 >/dev/null
: >"$FAKE_LOG"
CACHE=only REPLAY_BASE="$tmp/ra" "$rp/replay_all.sh" zz_hit zz_miss >"$tmp/ra.out" 2>&1 && r=0 || r=$?
check "replay_all cache-only with a missing cache exits 3, not 0" "$r" 3
grep -q '^zz_hit .*agree .*\[original cached\]' "$tmp/ra.out" && r=compared || r=missing
check "replay_all cache-only compares the cached script" "$r" compared
grep -q '^zz_miss .*no cache' "$tmp/ra.out" && r=listed || r=missing
check "replay_all cache-only lists the script without cache" "$r" listed
grep -q '^1 passed (0 cached, 1 run), 0 failed' "$tmp/ra.out" && r=counted || r=wrong
check "replay_all cache-only counts only the compared script" "$r" counted
grep -q '^1 without a cached original.*zz_miss' "$tmp/ra.out" && r=counted || r=wrong
check "replay_all cache-only counts the missing one" "$r" counted
check "replay_all cache-only starts no DOSBox" "$(runs)" 0
TRACE_DRAW=1 CACHE=only REPLAY_DIR="$tmp/prime" "$rp/cached_orig_trace.sh" "$tmp/draw.txt" 50 "$rp/scripts/zz_hit.txt" "" 1 >/dev/null && r=0 || r=$?
check "cache: a run with TRACE_DRAW=1 never hits the default entry" "$r" 3
TRACE_DRAW=0 CACHE=only REPLAY_DIR="$tmp/prime" TRACE_RAND=1 "$rp/cached_orig_trace.sh" "$tmp/draw.txt" 50 "$rp/scripts/zz_hit.txt" "" 1 >/dev/null && r=0 || r=$?
check "cache: TRACE_DRAW=0 is the default entry" "$r" 0
# Without CACHE=only the same miss runs the original (a fresh run here).
REPLAY_BASE="$tmp/ra2" "$rp/replay_all.sh" zz_miss >"$tmp/ra2.out" 2>&1 && r=exit0 || r=failed
check "replay_all runs a script without cache when not cache-only" "$(runs)" 1
check "replay_all not cache-only passes it" "$r" exit0
printf '# Turns 52\n0 1\n' >"$rp/scripts/zz_miss2.txt"
CACHE=only REPLAY_BASE="$tmp/ra3" "$rp/replay_all.sh" zz_miss2 >"$tmp/ra3.out" 2>&1 && r=0 || r=$?
check "replay_all cache-only with nothing cached exits 3" "$r" 3
CACHE=only REPLAY_BASE="$tmp/ra4" "$rp/replay_all.sh" zz_hit >"$tmp/ra4.out" 2>&1 && r=0 || r=$?
check "replay_all cache-only with everything cached exits 0" "$r" 0
# 10. The advisor's "# advice" lines are compared: the port writes one the
# cached original does not have.
PORT_CACHE=off FAKE_ADVICE=1 CACHE=only REPLAY_BASE="$tmp/ra5" "$rp/replay_all.sh" zz_hit >"$tmp/ra5.out" 2>&1 && r=0 || r=$?
check "replay_all fails on an advice line the original lacks" "$r" 1
grep -q '^zz_hit .*advice differs' "$tmp/ra5.out" && r=named || r=silent
check "replay_all names the advice difference" "$r" named

# 11. CACHE: a bad value is refused, refresh runs the original over an entry.
CACHE=bogus cached "$tmp/out11" 400 "$tmp/script.txt" skipme 1 >/dev/null && r=0 || r=$?
check "bad CACHE value exits 2" "$r" 2
: >"$FAKE_LOG"
CACHE=refresh cached "$tmp/out11" 400 "$tmp/script.txt" skipme 1 | grep -q 'cache miss .*stored' && r=stored || r=not
check "CACHE=refresh runs the original over an entry" "$r" stored
check "CACHE=refresh starts DOSBox" "$(runs)" 1

# 12. trace_cache.sh missing lists the scripts without an entry; prefetch
# fills them through replay_all.sh --orig-only and then finds none.
check "missing lists only the script without cache" "$("$rp/trace_cache.sh" missing zz_hit zz_miss2 | sed 's|.*/||')" "zz_miss2.txt"
: >"$FAKE_LOG"
"$rp/trace_cache.sh" prefetch zz_miss2 >/dev/null 2>&1
check "prefetch runs the original for the missing script" "$(runs)" 1
check "prefetch leaves nothing missing" "$("$rp/trace_cache.sh" missing zz_hit zz_miss2)" ""

# 13. The port verdict cache (port_cache.sh): a pass is stored under a key
# of the port's sources, build configuration and checking tools plus the
# original's trace key, and stands for the run while the key is current.
# Modification times come from a fake clock, so "the binary is newer than
# the sources" is exact: tick advances it, edit stamps a source, built the
# binaries.
port=build/replay-cache/port
rm -rf "$port"
base=$(($(date +%s) - 100000))
clock=$base
tick() { clock=$((clock + 10)); }
stamp() { tick; touch -d "@$clock" "$@"; }
find enhanced nix -type f -exec touch -d "@$base" {} +
touch -d "@$base" "$rp/main.cpp" "$rp/compare.py" "$rp/check_expect.py" "$rp/replay_all.sh" "$rp/port_cache.sh"
built() { stamp build/enhanced/th_replay build/web/th_replay.js; }
edit() { echo "// edit $RANDOM" >>"$1"; stamp "$1"; }
portruns() { grep -c '^portrun$' "$FAKE_LOG" || true; }
records() { find "$port" -name "$1" 2>/dev/null | wc -l; }
pa() { CACHE=only REPLAY_BASE="$tmp/pa" "$rp/replay_all.sh" "$@" >"$tmp/pa.out" 2>&1; }
summary() { grep '^[0-9]* passed' "$tmp/pa.out"; }
# port NAME ARG ...: a port_cache.sh function in a fresh shell, with the
# state the gate sets up.
portfn()
{
  (
    here=$rp root=$root
    . "$rp/lib.sh"
    . "$rp/port_cache.sh"
    port_cache_init build/enhanced build/web >/dev/null 2>&1
    "$@"
  )
}
nkey() { portfn port_key "$@"; }
pget() { portfn port_get "$@"; }
pput() { portfn port_put "$@"; }
ptrace() { portfn port_trace "$@"; }
differs() { [ "$1" != "$2" ] && echo differs || echo same; }
zz=$rp/scripts/zz_hit.txt
rec_of() { printf '%s/%s.zz_hit.%s.%s.rec' "$port" "$1" "$2" "$(nkey "$1" "$zz" "$2")"; }
built
: >"$FAKE_LOG"
pa zz_hit && r=0 || r=$?
check "port cache: first run passes" "$r" 0
check "port cache: first run runs th_replay" "$(portruns)" 1
check "port cache: first run counts it as run" "$(summary)" "1 passed (0 cached, 1 run), 0 failed"
check "port cache: first run stores a native record" "$(records 'native.zz_hit.*')" 1
check "port cache: the record holds the trace hash" "$(grep -c '^trace=[0-9a-f]\{64\}$' "$(rec_of native '')")" 1
first_line=$(grep '^zz_hit ' "$tmp/pa.out")
check "port cache: the record holds the printed line" "$(sed -n 's/^line=//p' "$(rec_of native '')")" "$first_line"
pa zz_hit && r=0 || r=$?
check "port cache: second identical run passes" "$r" 0
check "port cache: second run runs nothing" "$(portruns)" 1
check "port cache: second run counts it as cached" "$(summary)" "1 passed (1 cached, 0 run), 0 failed"
check "port cache: a cached pass prints the stored line" "$(grep '^zz_hit ' "$tmp/pa.out")" "$first_line"

# The key follows the port's sources (src/core), not src/app.
edit enhanced/src/app/y.cpp
pa zz_hit
check "port cache: editing src/app reruns nothing" "$(portruns)" 1
edit enhanced/src/core/x.cpp
pa zz_hit
check "port cache: editing src/core reruns the script" "$(portruns)" 2
check "port cache: a binary older than the source stores nothing" "$(records 'native.zz_hit.*')" 1
built
pa zz_hit
check "port cache: after a rebuild the script runs" "$(portruns)" 3
check "port cache: after a rebuild the pass is stored" "$(records 'native.zz_hit.*')" 2
pa zz_hit
check "port cache: and then it is a hit" "$(portruns)" 3
# A touch changes no bytes, and the key holds bytes.
stamp enhanced/src/core/x.cpp
built
pa zz_hit
check "port cache: touching src/core without a change is a hit" "$(portruns)" 3

# A failing script caches nothing.
edit enhanced/src/core/x.cpp
built
n=$(records 'native.*')
FAKE_ADVICE=1 pa zz_hit && r=0 || r=$?
check "port cache: a failing script fails" "$r" 1
check "port cache: a failing script stores nothing" "$(records 'native.*')" "$n"
pa zz_hit && r=0 || r=$?
check "port cache: the fixed script passes" "$r" 0
check "port cache: and is stored" "$(records 'native.*')" "$((n + 1))"

# PORT_CACHE=off reruns the port and rewrites the record.
rec=$(rec_of native '')
good=$(sed -n 's/^trace=//p' "$rec")
sed -i 's/^trace=.*/trace=corrupt/' "$rec"
before=$(portruns)
PORT_CACHE=off pa zz_hit
check "port cache: PORT_CACHE=off reruns the port" "$(portruns)" "$((before + 1))"
check "port cache: PORT_CACHE=off counts it as run" "$(summary)" "1 passed (0 cached, 1 run), 0 failed"
check "port cache: PORT_CACHE=off rewrites the record" "$(sed -n 's/^trace=//p' "$rec")" "$good"
PORT_CACHE=bogus pa zz_hit && r=0 || r=$?
check "port cache: a bad PORT_CACHE value exits 2" "$r" 2
# CACHE=refresh runs both sides.
: >"$FAKE_LOG"
CACHE=refresh REPLAY_BASE="$tmp/pa" "$rp/replay_all.sh" zz_hit >"$tmp/pa.out" 2>&1
check "port cache: CACHE=refresh runs DOSBox" "$(runs)" 1
check "port cache: CACHE=refresh runs the port" "$(portruns)" 1
check "port cache: CACHE=refresh counts it as run" "$(summary)" "1 passed (0 cached, 1 run), 0 failed"

# What the key covers.
k=$(nkey native "$zz")
kw=$(nkey web "$zz")
check "port key: stable" "$(nkey native "$zz")" "$k"
cp "$zz" "$tmp/zz_copy.txt"
check "port key: same bytes, another path" "$(nkey native "$tmp/zz_copy.txt")" "$k"
printf '# Turns 50\n0 2\n' >"$tmp/zz_other.txt"
check "port key: another script's bytes" "$(differs "$(nkey native "$tmp/zz_other.txt")" "$k")" differs
echo "zz_hit 0123456789abcdef" >"$tmp/native_keys"
check "port key: PORT_NATIVE_KEYS gives the native key" "$(PORT_NATIVE_KEYS=$tmp/native_keys nkey native "$zz")" 0123456789abcdef
check "port key: the web key is built on the given native key" "$(differs "$(PORT_NATIVE_KEYS=$tmp/native_keys nkey web "$zz")" "$kw")" differs
check "port key: a script missing from the file is computed" "$(PORT_NATIVE_KEYS=$tmp/native_keys nkey native "$tmp/zz_other.txt")" "$(nkey native "$tmp/zz_other.txt")"
check "trace_cache native-keys prints the computed key" "$("$rp/trace_cache.sh" native-keys zz_hit)" "zz_hit $k"
# keyvar LABEL FILE NATIVE WEB LINE: appends LINE to FILE, compares the keys
# (NATIVE and WEB are "differs" or "same") and restores the file.
keyvar()
{
  cp "$2" "$tmp/keyvar.bak"
  echo "$5" >>"$2"
  a=$(differs "$(nkey native "$zz")" "$k")
  b=$(differs "$(nkey web "$zz")" "$kw")
  cp "$tmp/keyvar.bak" "$2"
  touch -d "@$base" "$2"
  check "port key: $1 (native)" "$a" "$3"
  check "port key: $1 (web)" "$b" "$4"
}
keyvar "a src/core code line" enhanced/src/core/x.cpp differs differs "int x;"
keyvar "an app file is outside it" enhanced/src/app/y.cpp same same "int y;"
keyvar "main.cpp of th_replay" "$rp/main.cpp" differs differs "int m;"
keyvar "CMakeLists.txt" enhanced/CMakeLists.txt differs differs "x"
keyvar "cmake/deps.cmake" enhanced/cmake/deps.cmake differs differs "x"
keyvar "flake.lock" nix/flake.lock differs differs "x"
keyvar "flake.nix" nix/flake.nix differs differs "x"
keyvar "compare.py" "$rp/compare.py" differs differs "x = 1"
keyvar "check_expect.py" "$rp/check_expect.py" differs differs "x = 1"
keyvar "replay_all.sh code" "$rp/replay_all.sh" differs differs "true"
keyvar "replay_all.sh comment" "$rp/replay_all.sh" same same "# a comment"
keyvar "replay_all.sh blank line" "$rp/replay_all.sh" same same ""
keyvar "port_cache.sh code" "$rp/port_cache.sh" differs differs "true"
keyvar "an enhanced/web file" enhanced/web/shell.html same differs "x"
keyvar "check.sh code" enhanced/tools/check.sh same differs "true"
keyvar "check.sh comment" enhanced/tools/check.sh same same "# comment"
keyvar "the wasm configuration" build/web/CMakeCache.txt same differs "CMAKE_CXX_FLAGS:STRING=-O0"
keyvar "the native configuration" build/enhanced/CMakeCache.txt differs differs "CMAKE_CXX_FLAGS_RELEASE:STRING=-O1"
keyvar "the data folder of the configuration" build/enhanced/CMakeCache.txt same same "TH_DEV_DATA_DIR:PATH=/elsewhere"
keyvar "an unrelated cache entry" build/enhanced/CMakeCache.txt same same "CMAKE_HOME_DIRECTORY:INTERNAL=/x"
# The original's trace key is in it: another decompilation, another key.
echo y >>"$root/src/a.cpp"
check "port key: the decompilation (native)" "$(differs "$(nkey native "$zz")" "$k")" differs
check "port key: the decompilation (web)" "$(differs "$(nkey web "$zz")" "$kw")" differs
echo x >"$root/src/a.cpp"
check "port key: restored" "$(nkey native "$zz")" "$k"
check "port key: RAND is in it" "$(differs "$(RAND=0 nkey native "$zz")" "$k")" differs
check "port key: another tag" "$(differs "$(nkey web "$zz" cross-9000)" "$kw")" differs
check "port key: web differs from native" "$(differs "$kw" "$k")" differs

# Web records count only with the native record for the same key and trace.
built
pa zz_hit
native_trace=$(ptrace native "$zz")
check "web record: no record, no hit" "$(pget web "$zz" || echo miss)" miss
pput web "$zz" "" "zz_hit: identical" "$native_trace"
check "web record: a stored pass is a hit" "$(pget web "$zz")" "zz_hit: identical"
check "web record: another tag is a miss" "$(pget web "$zz" cross-9000 || echo miss)" miss
pput web "$zz" cross-9000 "zz_hit: turn 9000" "$native_trace"
check "web record: the cross-load record is a hit" "$(pget web "$zz" cross-9000)" "zz_hit: turn 9000"
pput web "$zz" "" "zz_hit: identical" "not-the-native-trace"
check "web record: another trace than the native one is a miss" "$(pget web "$zz" || echo miss)" miss
pput web "$zz" "" "zz_hit: identical" "$native_trace"
nrec=$(rec_of native '')
mv "$nrec" "$tmp/native.rec"
check "web record: without the native record it is a miss" "$(pget web "$zz" || echo miss)" miss
mv "$tmp/native.rec" "$nrec"
check "web record: with the native record it is a hit" "$(pget web "$zz")" "zz_hit: identical"
stamp enhanced/web/shell.html
echo "<!-- x -->" >>enhanced/web/shell.html
check "web record: a web shell edit misses it" "$(pget web "$zz" || echo miss)" miss
check "web record: the native record still hits" "$(pget native "$zz" | head -c 8)" "zz_hit ("
touch -d "@$base" enhanced/web/shell.html
sed -i '$d' enhanced/web/shell.html
edit enhanced/src/core/x.cpp
check "web record: a core edit misses it" "$(pget web "$zz" || echo miss)" miss
check "web record: a wasm binary older than the sources stores nothing" "$(pput web "$zz" "" l "$native_trace"; records "$(basename "$(rec_of web '')")")" 0
built

# status and prune: a record is stale when its key is not current.
pa zz_hit
stale_count() { "$rp/trace_cache.sh" status | sed -n 's/^stale entries (key not current): \([0-9]*\),.*/\1/p'; }
[ "$(stale_count)" -ge 1 ] && r=some || r=none
check "trace_cache status counts stale port records" "$r" some
all=$(records '*.rec')
"$rp/trace_cache.sh" prune >/dev/null
check "trace_cache prune keeps the current native record" "$(pget native "$zz" | head -c 8)" "zz_hit ("
[ "$(records '*.rec')" -lt "$all" ] && r=fewer || r=same
check "trace_cache prune removes stale port records" "$r" fewer
check "trace_cache status after prune: none stale" "$(stale_count)" 0

# missing: a script with a native verdict needs no original; a verdict
# stands for the whole run, so CACHE=only passes it without one.
printf '# Turns 53\n0 1\n' >"$rp/scripts/zz_miss3.txt"
zz3=$rp/scripts/zz_miss3.txt
touch -d "@$base" "$zz3"
built
: >"$FAKE_LOG"
check "missing lists a script without original and verdict" "$("$rp/trace_cache.sh" missing zz_miss3 | sed 's|.*/||')" zz_miss3.txt
pput native "$zz3" "" "zz_miss3 (level 1, 53 turns): agree [original fresh]" abc "enh: Expect X >= 1: 3"
check "port record: Expect counts are kept" "$(sed -n 's/^expect=//p' "$(portfn port_rec_path native "$zz3" "" "$(nkey native "$zz3")")")" "enh: Expect X >= 1: 3"
check "missing skips a script with a native verdict" "$("$rp/trace_cache.sh" missing zz_miss3)" ""
check "missing with PORT_CACHE=off lists it again" "$(PORT_CACHE=off "$rp/trace_cache.sh" missing zz_miss3 | sed 's|.*/||')" zz_miss3.txt
CACHE=only REPLAY_BASE="$tmp/pa" "$rp/replay_all.sh" zz_miss3 >"$tmp/pa.out" 2>&1 && r=0 || r=$?
check "a verdict passes without an original (CACHE=only)" "$r" 0
check "a verdict starts no DOSBox and no port run" "$(runs)$(portruns)" "00"
rm -f "$zz3"

# The files whose text the key strips hold no heredoc.
grep -l '<<' "$here/replay_all.sh" "$here/port_cache.sh" "$here/../check.sh" >/dev/null 2>&1 && r=heredoc || r=none
check "key: replay_all.sh, port_cache.sh and check.sh have no heredoc" "$r" none

rm -f "$rp/scripts/zz_hit.txt" "$rp/scripts/zz_miss.txt" "$rp/scripts/zz_miss2.txt"

if [ "$fails" = 0 ]; then echo "all cache tests passed"; else echo "$fails cache test(s) failed"; exit 1; fi

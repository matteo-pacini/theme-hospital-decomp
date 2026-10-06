#!/bin/sh
# The original game's side of the replay check: links an instrumented
# HOSPITAL.EXE from the decompilation's units (instrument.py on a copy of
# src/; the repository's sources are not touched), runs
# `HOSPITAL.EXE -L<level> -Q` in DOSBox-X with no window or sound, and copies
# the trace it writes, with its events, to OUT. TRACE_RAND=1 in the
# environment also traces every draw from the random seed.
#
# Usage (from the repository root):
#   enhanced/tools/replay/orig_trace.sh OUT TURNS [SCRIPT] [SKIP] [LEVEL]
#     OUT     where to write the trace
#     TURNS   the last turn to run
#     SCRIPT  the player's commands, as th_replay --script reads them
#     SKIP    calls of daily_admin/twice_daily_admin to leave out, separated
#             by commas (e.g. computer_action,manage_rats), to compare past
#             functions the engine does not port
#     LEVEL   default 1
#
# Needs the decompilation built (`ninja`: build/base's objects), the
# decompilation's tools (`nix develop path:./nix`: python3 with capstone,
# wibo, WATCOM_10_6, wlink, objconv) and dosbox-x on PATH. Works in
# $REPLAY_DIR (default build/replay): its tree and run folders, so runs in
# different folders can go at the same time.
set -eu

out=$1 turns=$2 script=${3:-} skip=${4:-} level=${5:-1}
root=$PWD
here=$(cd "$(dirname "$0")" && pwd)
work=${REPLAY_DIR:-build/replay}
tree=$work/tree
run=$work/run
[ -d build/base ] || { echo "build/base is missing: build the decompilation first (ninja)" >&2; exit 1; }

# A copy of the sources and objects: tools/link.py links whatever tree it is in.
mkdir -p "$tree/build"
rm -rf "$tree/src" "$tree/include" "$tree/tools" "$tree/config" "$tree/build/base"
cp -a src include tools config configure.py "$tree/"
cp -a build/base "$tree/build/"
units=$(python3 "$here/instrument.py" "$tree")
(
  cd "$tree"
  for unit in $units; do
    python3 tools/cc.py "$unit" "build/base/$(basename "$unit" .cpp).o"
  done
  python3 tools/link.py | grep -E "^(unresolved|duplicate)"
)

mkdir -p "$run/SAVE"
for f in ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM SOUND DOS4GW.EXE MODEM.INI; do
  [ -e "orig/$f" ] && ln -sfn "$root/orig/$f" "$run/$f"
done
sed 's/^LANGUAGE=.*/LANGUAGE=ENG/' orig/HOSPITAL.CFG > "$run/HOSPITAL.CFG"
cp "$tree/build/link/hosp.exe" "$run/HOSPITAL.EXE"
rm -f "$run/TRACE.TXT" "$run/SCRIPT.TXT"
[ -n "$script" ] && cp "$script" "$run/SCRIPT.TXT"

conf=$work/dosbox.conf
cat > "$conf" <<EOF
[dosbox]
machine=svga_s3
memsize=16
[cpu]
core=dynamic
cycles=max
[mixer]
nosound=true
[sblaster]
sbtype=none
[speaker]
pcspeaker=false
[autoexec]
mount c "$root/$run"
c:
set TRACE_TURNS=$turns
set TRACE_SKIP=$skip
set TRACE_RAND=${TRACE_RAND:-0}

HOSPITAL.EXE -L$level -Q
exit
EOF
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dosbox-x -nopromptfolder -fastlaunch \
  -set "dosbox quit warning=false" -conf "$root/$conf" > "$work/dosbox.log" 2>&1
cp "$run/TRACE.TXT" "$out"

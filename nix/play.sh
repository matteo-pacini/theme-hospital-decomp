# Build the decompilation, rebuild HOSPITAL.EXE from the units, and play it
# in DOSBox with the game data in orig/. Run from the repository root
# (`nix run path:./nix`); the build and a run directory go to build/.
# The byte-identical rebuild is checked against orig/HOSPITAL.EXE, which it
# needs; --wlink, linked from the repository's sources alone, needs only the
# game's data files.
#
# Usage: theme-hospital [--staging | --dosbox-x | --dosbox] [--wlink] [--486 | --pentium]
#   --staging   DOSBox Staging (default for the byte-identical rebuild)
#   --dosbox-x  DOSBox-X (default for --wlink)
#   --dosbox    DOSBox 0.74
#   --wlink     play build/link/hosp.exe, the units linked by wlink, instead
#               of the byte-identical rebuild
#   --486       emulate the game's minimum spec, a 486DX2-66 (fixed 23880
#               cycles); default is as fast as the host allows
#   --pentium   emulate the recommended spec, a Pentium 133 (fixed 71240
#               cycles, DOSBox-X's recalibrated figure)
#
# The run directory keeps its own HOSPITAL.CFG (LANGUAGE=ENG on first run;
# edit it to change language) and saves; game data is linked from orig/.

emulator=
cputype=auto
cycles=max
exe=build/hospital.exe
for arg in "$@"; do
  case $arg in
    --staging) emulator=staging ;;
    --dosbox-x) emulator=x ;;
    --dosbox) emulator=dosbox ;;
    --wlink) exe=build/link/hosp.exe ;;
    --486) cputype=486 cycles=23880 ;;
    --pentium) cputype=pentium cycles=71240 ;;
    *) echo "usage: theme-hospital [--staging | --dosbox-x | --dosbox] [--wlink] [--486 | --pentium]" >&2; exit 2 ;;
  esac
done

# The wlink-linked build defaults to DOSBox-X, the emulator it is tested in.
if [ -z "$emulator" ]; then
  if [ "$exe" = build/hospital.exe ]; then emulator=staging; else emulator=x; fi
fi

if [ ! -f configure.py ]; then
  echo "run from the repository root" >&2
  exit 1
fi
unpack="run nix run path:./nix#gog-unpack -- PATH/setup_theme_hospital_v3_(28027).exe"
for f in DATA DOS4GW.EXE HOSPITAL.CFG; do
  if [ ! -e "orig/$f" ]; then
    echo "orig/$f is missing: the game is not in orig/: $unpack" >&2
    exit 1
  fi
done
if [ "$exe" = build/hospital.exe ] && [ ! -f orig/HOSPITAL.EXE ]; then
  echo "orig/HOSPITAL.EXE is missing: the byte-identical rebuild is checked against it ($unpack)," >&2
  echo "or play the build linked without it: --wlink" >&2
  exit 1
fi

python3 configure.py
case $exe in
  build/hospital.exe)
    ninja all link
    python3 tools/lewrite.py --strict build/hospital.exe
    cmp build/hospital.exe orig/HOSPITAL.EXE
    echo "build/hospital.exe is byte-identical to orig/HOSPITAL.EXE"
    ;;
  build/link/hosp.exe) ninja link ;;
esac

run=build/run
# The game saves to C:\SAVE\ (INSTALL_PATH=c:\ in HOSPITAL.CFG), which the GOG
# data does not include.
mkdir -p "$run" "$run/SAVE"
for f in ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM SOUND DOS4GW.EXE MODEM.INI; do
  [ -e "orig/$f" ] && ln -sfn "$PWD/orig/$f" "$run/$f"
done
if [ ! -f "$run/HOSPITAL.CFG" ]; then
  sed 's/^LANGUAGE=.*/LANGUAGE=ENG/' orig/HOSPITAL.CFG > "$run/HOSPITAL.CFG"
fi
cp "$exe" "$run/HOSPITAL.EXE"

# DOSBox Staging 0.82 replaced `cycles` with cpu_cycles (real mode) and
# cpu_cycles_protected (the game runs in protected mode under DOS/4GW).
if [ "$cycles" = max ]; then
  cpu="[cpu]
core=auto
cycles=max"
elif [ "$emulator" = x ]; then
  cpu="[cpu]
core=normal
cputype=$cputype
cycles=fixed $cycles"
else
  cpu="[cpu]
core=normal
cputype=$cputype
cpu_cycles=$cycles
cpu_cycles_protected=$cycles
cpu_throttle=false"
fi

conf=build/dosbox.conf
cat > "$conf" <<EOF
[dosbox]
machine=svga_s3
memsize=16

$cpu

[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
hdma=5

[autoexec]
mount c "$PWD/$run"
c:
HOSPITAL.EXE
exit
EOF

case $emulator in
  staging) exec dosbox-staging -conf "$conf" ;;
  # Without these DOSBox-X asks for a working directory on its first run and
  # confirms before quitting.
  x) exec dosbox-x -nopromptfolder -defaultdir "$PWD/$run" -fastlaunch -set "dosbox quit warning=false" -conf "$PWD/$conf" ;;
  dosbox) exec dosbox -conf "$conf" ;;
esac

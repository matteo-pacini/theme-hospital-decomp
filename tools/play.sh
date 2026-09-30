# Build the decompilation, rebuild HOSPITAL.EXE from the units, and play it
# in DOSBox with the game data in orig/. Run from the repository root
# (`nix run .`); the build and a run directory go to build/.
#
# Usage: theme-hospital [--staging | --dosbox-x | --dosbox] [--wlink]
#   --staging   DOSBox Staging (default)
#   --dosbox-x  DOSBox-X
#   --dosbox    DOSBox 0.74
#   --wlink     play build/link/hosp.exe, the units linked by wlink, instead
#               of the byte-identical rebuild
#
# The run directory keeps its own HOSPITAL.CFG (LANGUAGE=ENG on first run;
# edit it to change language) and saves; game data is linked from orig/.

emulator=staging
exe=build/hospital.exe
for arg in "$@"; do
  case $arg in
    --staging) emulator=staging ;;
    --dosbox-x) emulator=x ;;
    --dosbox) emulator=dosbox ;;
    --wlink) exe=build/link/hosp.exe ;;
    *) echo "usage: theme-hospital [--staging | --dosbox-x | --dosbox] [--wlink]" >&2; exit 2 ;;
  esac
done

if [ ! -f configure.py ]; then
  echo "run from the repository root" >&2
  exit 1
fi
if [ ! -f orig/HOSPITAL.EXE ] || [ ! -d orig/DATA ]; then
  echo "the game is not in orig/: run nix run .#gog-unpack -- PATH/setup_theme_hospital_v3_(28027).exe" >&2
  exit 1
fi

python3 configure.py
ninja all link
python3 tools/lewrite.py --strict build/hospital.exe
cmp build/hospital.exe orig/HOSPITAL.EXE
echo "build/hospital.exe is byte-identical to orig/HOSPITAL.EXE"

run=build/run
mkdir -p "$run"
for f in ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM SOUND DOS4GW.EXE MODEM.INI; do
  [ -e "orig/$f" ] && ln -sfn "$PWD/orig/$f" "$run/$f"
done
if [ ! -f "$run/HOSPITAL.CFG" ]; then
  sed 's/^LANGUAGE=.*/LANGUAGE=ENG/' orig/HOSPITAL.CFG > "$run/HOSPITAL.CFG"
fi
cp "$exe" "$run/HOSPITAL.EXE"

conf=build/dosbox.conf
cat > "$conf" <<EOF
[dosbox]
machine=svga_s3
memsize=16

[cpu]
core=auto
cycles=max

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
  x) exec dosbox-x -conf "$conf" ;;
  dosbox) exec dosbox -conf "$conf" ;;
esac

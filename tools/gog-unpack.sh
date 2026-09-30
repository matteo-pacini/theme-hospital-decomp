# Extract the game from GOG's installer into orig/, which the build and
# `nix run .` need. Run from the repository root.
#
# Usage: gog-unpack PATH/setup_theme_hospital_v3_(28027).exe
#
# Only GOG's v3 (28027) installer is accepted, checked by file name and
# sha256. Files already in orig/ other than the game's are left alone.

setup_name='setup_theme_hospital_v3_(28027).exe'
setup_sha256=3475c2bb4b262156af4d92e6b1e9e20075f1788a8776835b534e2917c7921e93
exe_sha1=d108d34d4e435ce4ad1e9c9b76ab3ad6f000d6ae
game_files=(HOSPITAL.EXE DOS4GW.EXE HOSPITAL.CFG MODEM.INI ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM SOUND)

if [ $# -ne 1 ]; then
  echo "usage: gog-unpack PATH/$setup_name" >&2
  exit 2
fi
setup=$1
if [ ! -f configure.py ]; then
  echo "run from the repository root" >&2
  exit 1
fi
if [ "$(basename "$setup")" != "$setup_name" ]; then
  echo "expected GOG's installer $setup_name, got $(basename "$setup")" >&2
  exit 1
fi
echo "checking $setup"
if [ "$(sha256sum "$setup" | cut -d' ' -f1)" != "$setup_sha256" ]; then
  echo "$setup: sha256 differs from GOG's $setup_name ($setup_sha256)" >&2
  exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
innoextract --silent -d "$tmp" "$setup"
if [ "$(sha1sum "$tmp/HOSPITAL.EXE" | cut -d' ' -f1)" != "$exe_sha1" ]; then
  echo "the installer's HOSPITAL.EXE is not the one this project matches ($exe_sha1)" >&2
  exit 1
fi

mkdir -p orig
for f in "${game_files[@]}"; do
  rm -rf "orig/$f"
  cp -r "$tmp/$f" "orig/$f"
done
echo "extracted ${game_files[*]} into orig/"

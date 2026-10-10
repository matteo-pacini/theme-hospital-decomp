#!/bin/sh
# Tests the setup screen and --install-from through th_enhanced, headless,
# each in a fake HOME under build/install-tests (the user's folders are never
# touched; an install is ~175 MB, removed when the tests are done). Needs
# `orig/` (the game data) and build/enhanced (or INSTALL_BUILD). Run from the
# repository root; prints one line per check and exits non-zero on a failure.
set -u
build=${INSTALL_BUILD:-build/enhanced}
exe=$PWD/$build/th_enhanced
tests=$PWD/enhanced/tests/install
orig=$PWD/orig
base=$PWD/build/install-tests
failed=0

pass() { echo "pass $1"; }
fail() { echo "FAIL $1"; failed=1; }

# run NAME HOME EXTRA_ARGS... : th_enhanced in HOME with the script NAME.txt
# (also the output folder's name); returns its exit status.
run() {
  name=$1 home=$2
  shift 2
  mkdir -p "$home"
  env -u LD_LIBRARY_PATH HOME="$home" XDG_CONFIG_HOME="$home/.config" \
    XDG_DATA_HOME="$home/.local/share" XDG_STATE_HOME="$home/.local/state" \
    TH_DEBUG_REPORT_DIR="$base/reports" \
    "$exe" "$@" --drive "$tests/$name.txt" --out "$base/out-$name-$(basename "$home")" \
    >"$base/log-$name-$(basename "$home")" 2>&1
}
check() { # NAME DESCRIPTION STATUS
  if [ "$3" -eq 0 ]; then pass "$1: $2"; else fail "$1: $2 (see $base/log-*)"; fi
}

rm -rf "$base"
mkdir -p "$base"

# No game data: the setup screen.
run setup_screen "$base/h-empty" --user-data
check setup_screen "no data shows the setup screen" $?

# A folder dropped on the window installs it; a second start finds it.
run title "$base/h-drop" --user-data --drop "$orig"
check drop "dropped folder installs and starts" $?
run title "$base/h-drop" --user-data
check drop "second start finds the install" $?

# --install-from, with a driven run; then a start without it.
run title "$base/h-cli" --install-from "$orig"
check install-from "installs, then drives" $?
grep -q '^Installed\.' "$base/log-title-h-cli"
check install-from "reports on stderr" $?
run title "$base/h-cli" --user-data
check install-from "second start finds the install" $?

# A corrupted source: a folder of links to orig with one file replaced.
rm -rf "$base/src"
mkdir "$base/src"
for d in ANIMS DATA INTRO LEVELS QDATA QDATAM SOUND; do
  cp -rs "$orig/$d" "$base/src/$d"
done
victim=$(cd "$orig" && find DATA -type f -size -64k | sort | head -1)
rm "$base/src/$victim"
cp "$orig/$victim" "$base/src/$victim"
printf '\377' | dd of="$base/src/$victim" bs=1 count=1 conv=notrunc 2>/dev/null
run title "$base/h-bad" --install-from "$base/src"
[ $? -eq 1 ]
check corrupt "exits 1" $?
grep -qi "$victim" "$base/log-title-h-bad"
check corrupt "names $victim" $?
[ ! -e "$base/h-bad/.local/share/ThemeHospitalEnhanced/game/install.ok" ] &&
  [ ! -d "$base/h-bad/.local/share/ThemeHospitalEnhanced/game.partial" ]
check corrupt "leaves no install" $?

# An install without its marker is incomplete; Verify files says so.
rm "$base/h-cli/.local/share/ThemeHospitalEnhanced/game/install.ok"
run incomplete_verify "$base/h-cli" --user-data --size 640x480
check incomplete "setup screen and verify" $?

rm -rf "$base/h-drop" "$base/h-cli" "$base/h-empty" "$base/h-bad" "$base/src"
exit "$failed"

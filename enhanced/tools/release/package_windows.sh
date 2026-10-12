#!/bin/sh
# Packages a Windows release build as OUT_DIR/theme-hospital-enhanced-TAG-
# windows-ARCH.zip: one folder with ThemeHospitalEnhanced.exe and the files
# stage.sh writes. Run in the MSYS2 shell (needs zip).
#
#   package_windows.sh BUILD_DIR ARCH TAG OUT_DIR
#
# ARCH is x86_64 or arm64.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 4 ] || die "usage: package_windows.sh BUILD_DIR ARCH TAG OUT_DIR"
build=$1
arch=$2
tag=$3
out=$4
case $arch in
  x86_64 | arm64) ;;
  *) die "unknown ARCH $arch (x86_64 or arm64)" ;;
esac
cd "$root"
[ -f "$build/ThemeHospitalEnhanced.exe" ] || die "$build/ThemeHospitalEnhanced.exe not found"

name=$(release_name "$tag" "windows-$arch")
work=$build/zip
rm -rf "$work"
mkdir -p "$work/$name" "$out"
cp "$build/ThemeHospitalEnhanced.exe" "$work/$name/"
"$root/enhanced/tools/release/stage.sh" "$work/$name" "$build/_deps"
(cd "$work" && zip -q -r -X "$name.zip" "$name")
mv "$work/$name.zip" "$out/"
rm -rf "$work"
echo "package_windows: $out/$name.zip"

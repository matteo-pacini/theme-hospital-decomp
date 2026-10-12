#!/bin/sh
# Packages a macOS release build as OUT_DIR/theme-hospital-enhanced-TAG-
# macos-arm64.zip: one folder with "Theme Hospital Enhanced.app", ad-hoc
# signed (arm64 programs must be signed to run; there is no Developer ID),
# and the files stage.sh writes. The zip is made with ditto, which keeps the
# bundle's attributes.
#
#   package_macos.sh BUILD_DIR TAG OUT_DIR
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 3 ] || die "usage: package_macos.sh BUILD_DIR TAG OUT_DIR"
build=$1
tag=$2
out=$3
cd "$root"
bundle="Theme Hospital Enhanced.app"
[ -d "$build/$bundle" ] || die "$build/$bundle not found"

name=$(release_name "$tag" macos-arm64)
work=$build/zip
rm -rf "$work"
mkdir -p "$work/$name" "$out"
ditto "$build/$bundle" "$work/$name/$bundle"
codesign --force --deep --sign - "$work/$name/$bundle"
codesign --verify --deep --strict "$work/$name/$bundle"
"$root/enhanced/tools/release/stage.sh" "$work/$name" "$build/_deps"
ditto -c -k --keepParent "$work/$name" "$out/$name.zip"
rm -rf "$work"
echo "package_macos: $out/$name.zip"

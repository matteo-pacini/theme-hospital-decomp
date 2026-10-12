#!/bin/sh
# Regenerates the derived icon files in this folder from the two masters:
#   icon-1024.png      the art cut out on transparency (Windows, Linux, web,
#                      the window icon)
#   icon-mac-1024.png  the same on macOS's rounded tile (the bundle's icon)
# Derived: icon-256.png, icon.ico (16..256) and icon.icns (16..1024 with @2x).
# To change the icon, replace the masters, run this script and commit the
# results. ImageMagick comes from nix when it is not on PATH.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
if ! command -v magick >/dev/null 2>&1; then
  exec env -u LD_LIBRARY_PATH nix shell nixpkgs#imagemagick -c sh "$0" "$@"
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# scale MASTER SIZE OUT: a Lanczos downscale, lightly sharpened up to 32 pixels
# where it would otherwise look soft.
scale() {
  if [ "$2" -le 32 ]; then
    magick "$1" -depth 8 -filter Lanczos -resize "${2}x${2}" -unsharp 0x0.6+0.6+0.02 "$3"
  else
    magick "$1" -depth 8 -filter Lanczos -resize "${2}x${2}" "$3"
  fi
}

for n in 16 24 32 48 64 128 256; do
  scale "$here/icon-1024.png" "$n" "$work/plain-$n.png"
done
cp "$work/plain-256.png" "$here/icon-256.png"
magick "$work/plain-16.png" "$work/plain-24.png" "$work/plain-32.png" "$work/plain-48.png" \
  "$work/plain-64.png" "$work/plain-128.png" "$work/plain-256.png" "$here/icon.ico"

for n in 16 32 64 128 256 512; do
  scale "$here/icon-mac-1024.png" "$n" "$work/mac-$n.png"
done
magick "$here/icon-mac-1024.png" -depth 8 "$work/mac-1024.png"

# icns: "icns", the total length, then per image its type, its length (8 plus
# the PNG) and the PNG, all lengths big-endian. The types are those of an
# iconset: icp4..ic10 are 16..1024 pixels, ic11..ic14 the @2x of 16..256.
be32() {
  printf "\\$(printf %03o $(($1 >> 24 & 255)))\\$(printf %03o $(($1 >> 16 & 255)))\\$(printf %03o $(($1 >> 8 & 255)))\\$(printf %03o $(($1 & 255)))"
}
size_of() { wc -c <"$1" | tr -d ' '; }
images="icp4:16 icp5:32 icp6:64 ic07:128 ic08:256 ic09:512 ic10:1024 ic11:32 ic12:64 ic13:256 ic14:512"
total=8
for t in $images; do total=$((total + 8 + $(size_of "$work/mac-${t#*:}.png"))); done
{
  printf icns
  be32 "$total"
  for t in $images; do
    f=$work/mac-${t#*:}.png
    printf %s "${t%:*}"
    be32 $((8 + $(size_of "$f")))
    cat "$f"
  done
} >"$here/icon.icns"

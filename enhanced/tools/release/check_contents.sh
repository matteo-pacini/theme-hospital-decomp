#!/bin/sh
# Checks a release file's contents against an allowlist: the program, its
# icon and desktop entry or bundle files, and the documents stage.sh writes.
# Anything else (game data, caches, build outputs, debug reports) fails.
#
#   check_contents.sh FILE
#
# FILE is a .zip (Windows, macOS) or an .AppImage (Linux), named as the
# package scripts name them. An AppImage is checked by unpacking it with
# --appimage-extract, which its static runtime handles itself (the program
# does not start). Needs unzip for a zip.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 1 ] || die "usage: check_contents.sh FILE"
file=$1
[ -f "$file" ] || die "$file not found"
base=$(basename "$file")
app_id=me.matteopacini.theme-hospital-enhanced
docs="README.txt LICENSE COPYING CHANGELOG.md THIRD_PARTY_NOTICES.txt"

work=$root/build/check-contents-$$
trap 'rm -rf "$work"' EXIT
rm -rf "$work"
mkdir -p "$work"
list=$work/list

# require ITEM ...: fails unless every ITEM is an entry of `list`.
require()
{
  for item in "$@"; do
    grep -qxF "$item" "$list" || die "$base lacks $item"
  done
}

# rejected FUNCTION: prints the entries of `list` that FUNCTION, called with
# the entry, does not accept.
rejected()
{
  while IFS= read -r entry; do
    "$1" "$entry" || echo "$entry"
  done <"$list"
}

appimage_allowed()
{
  case $1 in
    AppRun | .DirIcon | "$app_id.desktop" | "$app_id.png") ;;
    usr | usr/bin | usr/bin/th_enhanced) ;;
    usr/share | usr/share/applications | "usr/share/applications/$app_id.desktop") ;;
    usr/share/icons | usr/share/icons/hicolor | usr/share/icons/hicolor/256x256) ;;
    usr/share/icons/hicolor/256x256/apps | "usr/share/icons/hicolor/256x256/apps/$app_id.png") ;;
    usr/share/doc | usr/share/doc/theme-hospital-enhanced) ;;
    usr/share/doc/theme-hospital-enhanced/*)
      case " $docs " in *" ${1#usr/share/doc/theme-hospital-enhanced/} "*) ;; *) return 1 ;; esac
      ;;
    *) return 1 ;;
  esac
}

zip_allowed()
{
  # $1 is relative to the zip's one top folder.
  case $1 in
    "") ;;
    ThemeHospitalEnhanced.exe) ;;
    "Theme Hospital Enhanced.app" | "Theme Hospital Enhanced.app"/Contents) ;;
    "Theme Hospital Enhanced.app"/Contents/Info.plist | "Theme Hospital Enhanced.app"/Contents/PkgInfo) ;;
    "Theme Hospital Enhanced.app"/Contents/MacOS | "Theme Hospital Enhanced.app"/Contents/MacOS/*) ;;
    "Theme Hospital Enhanced.app"/Contents/Resources | "Theme Hospital Enhanced.app"/Contents/Resources/*.icns) ;;
    "Theme Hospital Enhanced.app"/Contents/_CodeSignature | "Theme Hospital Enhanced.app"/Contents/_CodeSignature/*) ;;
    *)
      case " $docs " in *" $1 "*) ;; *) return 1 ;; esac
      ;;
  esac
}

case $base in
  *.AppImage)
    cp "$file" "$work/package.AppImage"
    chmod +x "$work/package.AppImage"
    (cd "$work" && ./package.AppImage --appimage-extract >/dev/null)
    (cd "$work/squashfs-root" && find . -mindepth 1 | sed 's|^\./||' | sort) >"$list"
    require AppRun .DirIcon "$app_id.desktop" "$app_id.png" usr/bin/th_enhanced
    for doc in $docs; do require "usr/share/doc/theme-hospital-enhanced/$doc"; done
    bad=$(rejected appimage_allowed)
    ;;
  *.zip)
    unzip -Z1 "$file" | sed 's|/$||' | sort >"$work/raw"
    top=${base%.zip}
    [ "$(sed 's|/.*||' "$work/raw" | sort -u)" = "$top" ] || die "$base: entries are not all under $top/"
    sed -e "s|^$top/\{0,1\}||" "$work/raw" | sort -u >"$list"
    case $base in
      *-windows-*) require ThemeHospitalEnhanced.exe ;;
      *-macos-*) require "Theme Hospital Enhanced.app" "Theme Hospital Enhanced.app/Contents/Info.plist" ;;
      *) die "$base: not a windows or macos zip" ;;
    esac
    for doc in $docs; do require "$doc"; done
    bad=$(rejected zip_allowed)
    ;;
  *) die "$base: neither a .zip nor an .AppImage" ;;
esac

if [ -n "$bad" ]; then
  printf '%s\n' "$bad" >&2
  die "$base holds files outside the allowlist (above)"
fi
echo "check_contents: $base: $(wc -l <"$list") entries, all allowed"

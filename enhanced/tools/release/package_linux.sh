#!/bin/sh
# Packages a Linux release build as an AppImage: OUT_DIR/theme-hospital-
# enhanced-TAG-linux-ARCH.AppImage, a bare file (not zipped). Built with the
# static-runtime appimagetool, which needs no libfuse2, from the pinned
# release below (downloaded once into TH_RELEASE_TOOLS, default build/
# release-tools, and checked against its SHA-256).
#
#   package_linux.sh BUILD_DIR ARCH TAG OUT_DIR
#
# ARCH is x86_64 or arm64. The icon and the .desktop file are read from
# enhanced/packaging, or from TH_PACKAGING_DIR. TH_APPIMAGE_RUN is a command
# prefix to run the downloaded AppImages with (nix's `appimage-run` on NixOS,
# which cannot run them directly); empty elsewhere.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 4 ] || die "usage: package_linux.sh BUILD_DIR ARCH TAG OUT_DIR"
build=$1
arch=$2
tag=$3
out=$4
cd "$root"

APPIMAGETOOL_VERSION=1.9.1
RUNTIME_VERSION=20251108
case $arch in
  x86_64)
    tool_arch=x86_64
    tool_sha=ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
    runtime_sha=2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d
    ;;
  arm64)
    tool_arch=aarch64
    tool_sha=f0837e7448a0c1e4e650a93bb3e85802546e60654ef287576f46c71c126a9158
    runtime_sha=00cbdfcf917cc6c0ff6d3347d59e0ca1f7f45a6df1a428a0d6d8a78664d87444
    ;;
  *) die "unknown ARCH $arch (x86_64 or arm64)" ;;
esac

app_id=me.matteopacini.theme-hospital-enhanced
packaging=${TH_PACKAGING_DIR:-$root/enhanced/packaging}
program=$build/th_enhanced
[ -x "$program" ] || die "$program not found"
[ -f "$packaging/icon/icon-256.png" ] || die "$packaging/icon/icon-256.png not found"
[ -f "$packaging/linux/$app_id.desktop" ] || die "$packaging/linux/$app_id.desktop not found"

tools=${TH_RELEASE_TOOLS:-$root/build/release-tools}
mkdir -p "$tools"
fetch()
{
  [ -f "$tools/$1" ] && sha256sum "$tools/$1" | grep -q "^$3 " && return 0
  curl -fsSL --retry 3 -o "$tools/$1" "$2"
  verify_sha256 "$tools/$1" "$3"
}
fetch "appimagetool-$tool_arch-$APPIMAGETOOL_VERSION.AppImage" \
  "https://github.com/AppImage/appimagetool/releases/download/$APPIMAGETOOL_VERSION/appimagetool-$tool_arch.AppImage" "$tool_sha"
fetch "runtime-$tool_arch-$RUNTIME_VERSION" \
  "https://github.com/AppImage/type2-runtime/releases/download/$RUNTIME_VERSION/runtime-$tool_arch" "$runtime_sha"
chmod +x "$tools/appimagetool-$tool_arch-$APPIMAGETOOL_VERSION.AppImage"

name=$(release_name "$tag" "linux-$arch")
work=$build/appimage
rm -rf "$work"
appdir=$work/AppDir
doc=$appdir/usr/share/doc/theme-hospital-enhanced
mkdir -p "$appdir/usr/bin" "$doc" "$appdir/usr/share/applications" \
  "$appdir/usr/share/icons/hicolor/256x256/apps" "$out"
cp "$program" "$appdir/usr/bin/th_enhanced"
"$root/enhanced/tools/release/stage.sh" "$doc" "$build/_deps"
cp "$packaging/linux/$app_id.desktop" "$appdir/$app_id.desktop"
cp "$packaging/linux/$app_id.desktop" "$appdir/usr/share/applications/"
cp "$packaging/icon/icon-256.png" "$appdir/$app_id.png"
cp "$packaging/icon/icon-256.png" "$appdir/usr/share/icons/hicolor/256x256/apps/$app_id.png"
ln -s "$app_id.png" "$appdir/.DirIcon"
ln -s usr/bin/th_enhanced "$appdir/AppRun"

# The tool is itself an AppImage: extract-and-run keeps it independent of
# FUSE, and its runtime comes from the pinned file, not from a download.
# shellcheck disable=SC2086 # TH_APPIMAGE_RUN is a command prefix
ARCH=$tool_arch APPIMAGE_EXTRACT_AND_RUN=1 ${TH_APPIMAGE_RUN:-} \
  "$tools/appimagetool-$tool_arch-$APPIMAGETOOL_VERSION.AppImage" \
  --runtime-file "$tools/runtime-$tool_arch-$RUNTIME_VERSION" --no-appstream \
  "$appdir" "$out/$name.AppImage"
chmod +x "$out/$name.AppImage"
rm -rf "$work"
echo "package_linux: $out/$name.AppImage"

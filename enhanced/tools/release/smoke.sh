#!/bin/sh
# Starts a release program headless (SDL's dummy video and audio drivers,
# empty user folders, no game data) through --drive and checks that it comes
# up and reports the project's version. Proves the static SDL starts; it
# touches no game data and nothing outside build/.
#
#   smoke.sh PROGRAM
#
# PROGRAM is the executable (inside the .app on macOS). TH_APPIMAGE_RUN is a
# command prefix, as for package_linux.sh.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 1 ] || die "usage: smoke.sh PROGRAM"
program=$1
version=$("$root/enhanced/tools/release/version.sh")

dir=$root/build/smoke-$$
trap 'rm -rf "$dir"' EXIT
rm -rf "$dir"
mkdir -p "$dir/home" "$dir/out"
printf '# NoLevel\nwait 30\nexpect version: %s\n' "$version" >"$dir/script.txt"

# shellcheck disable=SC2086 # TH_APPIMAGE_RUN is a command prefix
env HOME="$dir/home" XDG_CONFIG_HOME="$dir/home/config" XDG_DATA_HOME="$dir/home/data" \
  XDG_STATE_HOME="$dir/home/state" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ${TH_APPIMAGE_RUN:-} "$program" --drive "$dir/script.txt" --out "$dir/out" ||
  die "$program did not start or did not report version $version"
echo "smoke: $program started and reports version $version"

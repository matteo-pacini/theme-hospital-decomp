#!/bin/sh
# Prints the project version: the VERSION of project() in
# enhanced/CMakeLists.txt, the one source of it.
#
#   version.sh          print it
#   version.sh TAG      print it, but fail unless TAG is "v" followed by it
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

version=$(sed -n 's/^project([^)]* VERSION \([0-9][0-9.]*\).*/\1/p' "$root/enhanced/CMakeLists.txt")
[ -n "$version" ] || die "no project VERSION in enhanced/CMakeLists.txt"
if [ $# -ge 1 ]; then
  case $1 in
    v[0-9]*.[0-9]*.[0-9]*) ;;
    *) die "tag $1 is not of the form vX.Y.Z" ;;
  esac
  [ "$1" = "v$version" ] || die "tag $1 is not v$version, the project VERSION in enhanced/CMakeLists.txt"
fi
echo "$version"

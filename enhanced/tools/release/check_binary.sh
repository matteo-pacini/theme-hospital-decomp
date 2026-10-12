#!/bin/sh
# Checks that a release binary is as static as its platform allows and runs
# on the oldest system the release supports.
#
#   check_binary.sh linux PROGRAM [MAX_GLIBC]
#       needed libraries only glibc's (libc, libm, libdl, libpthread, librt
#       and the loader), no GLIBC_ symbol version above MAX_GLIBC (default
#       2.35, Ubuntu 22.04's)
#   check_binary.sh windows PROGRAM
#       imports no SDL3.dll and no lib*.dll (the C++ runtime, winpthreads)
#   check_binary.sh macos PROGRAM
#       links only /usr/lib and /System libraries, is arm64 and needs macOS
#       13.3 or older
#
# Needs objdump (llvm-objdump will do) on Linux and Windows, otool and lipo
# on macOS.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -ge 2 ] || die "usage: check_binary.sh linux|windows|macos PROGRAM [MAX_GLIBC]"
kind=$1
program=$2
[ -f "$program" ] || die "$program not found"

objdump_tool()
{
  for tool in objdump llvm-objdump; do
    command -v "$tool" >/dev/null 2>&1 && {
      echo "$tool"
      return
    }
  done
  die "objdump is not on PATH"
}

check_linux()
{
  max=${1:-2.35}
  objdump=$(objdump_tool)
  needed=$("$objdump" -p "$program" | sed -n 's/^ *NEEDED *//p')
  echo "needed: $(echo "$needed" | tr '\n' ' ')"
  for lib in $needed; do
    case $lib in
      libc.so.6 | libm.so.6 | libdl.so.2 | libpthread.so.0 | librt.so.1 | ld-linux-*.so.* | ld-linux.so.*) ;;
      *) die "$program needs $lib: only glibc's libraries may be dynamic" ;;
    esac
  done
  newest=$("$objdump" -T "$program" | grep -o 'GLIBC_[0-9][0-9.]*' | sed 's/^GLIBC_//' | sort -u -V | tail -n 1)
  [ -n "$newest" ] || die "no GLIBC_ symbol version found: is $program a dynamic Linux program?"
  echo "newest GLIBC_ version: $newest (at most $max)"
  [ "$(printf '%s\n%s\n' "$newest" "$max" | sort -V | tail -n 1)" = "$max" ] ||
    die "$program needs GLIBC_$newest, newer than $max"
}

check_windows()
{
  objdump=$(objdump_tool)
  dlls=$("$objdump" -p "$program" | sed -n 's/^.*DLL Name: *//p')
  echo "imports: $(echo "$dlls" | tr '\n' ' ')"
  for dll in $dlls; do
    case $(echo "$dll" | tr '[:upper:]' '[:lower:]') in
      sdl*.dll | lib*.dll) die "$program imports $dll: it must be linked in statically" ;;
    esac
  done
}

check_macos()
{
  libs=$(otool -L "$program" | sed -n '2,$p' | awk '{print $1}')
  echo "links: $(echo "$libs" | tr '\n' ' ')"
  for lib in $libs; do
    case $lib in
      /usr/lib/* | /System/Library/*) ;;
      *) die "$program links $lib: only system libraries may be dynamic" ;;
    esac
  done
  [ "$(lipo -archs "$program")" = arm64 ] || die "$program is not arm64 only: $(lipo -archs "$program")"
  minos=$(otool -l "$program" | awk '/LC_BUILD_VERSION/ { f = 1 } f && $1 == "minos" { print $2; exit }')
  [ -n "$minos" ] || die "no minimum macOS version in $program"
  echo "minimum macOS: $minos (at most 13.3)"
  [ "$(printf '%s\n%s\n' "$minos" 13.3 | sort -V | tail -n 1)" = 13.3 ] || die "$program needs macOS $minos"
}

case $kind in
  linux) check_linux "${3:-}" ;;
  windows) check_windows ;;
  macos) check_macos ;;
  *) die "unknown kind $kind" ;;
esac
echo "check_binary: $program is as expected"

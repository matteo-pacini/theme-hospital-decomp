#!/bin/sh
# Writes THIRD_PARTY_NOTICES.txt to stdout: the licence text of every
# dependency a release binary contains. The texts come from the dependencies'
# own pinned sources (the FetchContent folders of the build, which
# enhanced/cmake/deps.cmake pins by tag or archive hash) and xBRZ's from
# enhanced/third_party/xbrz, so no copy in the repository goes stale.
#
#   notices.sh DEPS_DIR
#
# DEPS_DIR holds sdl3-src, imgui-src, ymfm-src and miniz-src: the build's
# `_deps` folder.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -eq 1 ] || die "usage: notices.sh DEPS_DIR"
deps=$1

# section NAME SPDX WHAT FILE
section()
{
  [ -f "$4" ] || die "licence file $4 not found (was the dependency fetched?)"
  printf '%s\n%s (%s)\n%s\n\n' "================================================================" "$1" "$2" "$3"
  cat "$4"
  printf '\n\n'
}

cat <<'TEXT'
Theme Hospital Enhanced: third-party notices

Theme Hospital Enhanced is free software under the GNU General Public License
version 3 (COPYING). It contains the libraries below, under their own
licences, which follow in full.

TEXT

section "SDL 3" "zlib" "Window, input, audio and graphics; linked into the program." \
  "$deps/sdl3-src/LICENSE.txt"
section "Dear ImGui" "MIT" "The developer tools (F10, F12)." \
  "$deps/imgui-src/LICENSE.txt"
section "ymfm" "BSD-3-Clause" "The YMF262 (OPL3) emulation the music plays on." \
  "$deps/ymfm-src/LICENSE"
section "miniz" "MIT" "Reading GOG's installer and the upscaling cache." \
  "$deps/miniz-src/LICENSE"
section "xBRZ" "GPL-3.0-only" "The install-time graphics upscaler (the GNU General Public License version 3 is also in COPYING)." \
  "$root/enhanced/third_party/xbrz/License.txt"

cat <<'TEXT'
================================================================
Compiler runtime libraries

Depending on the platform the program also contains its compiler's runtime,
linked in statically: the GNU C++ runtime (libstdc++, libgcc) under the GNU
General Public License version 3 with the GCC Runtime Library Exception, or
LLVM's C++ runtime (libc++, libunwind) under the Apache License 2.0 with LLVM
Exceptions, and on Windows the mingw-w64 runtime and winpthreads (MIT and
public domain). The sources are those of the toolchain used for the build.
TEXT

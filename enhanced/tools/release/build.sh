#!/bin/sh
# Configures and builds the release configuration in BUILD_DIR, fails on any
# warning from the project's own targets (the fetched dependencies, under
# _deps, are exempt, as in check.sh) and runs th_tests without game data.
# Run from anywhere; CC, CXX and the CMAKE_* environment are the caller's.
#
#   build.sh BUILD_DIR [CMAKE_ARG ...]
#
# CMAKE_ARGs (for instance -DCMAKE_OSX_ARCHITECTURES=arm64) go to the
# configure step. The program is then at BUILD_DIR/th_enhanced
# (ThemeHospitalEnhanced.exe on Windows, "Theme Hospital Enhanced.app" on
# macOS).
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -ge 1 ] || die "usage: build.sh BUILD_DIR [CMAKE_ARG ...]"
build=$1
shift
cd "$root"

cmake -S enhanced -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DTH_RELEASE=ON "$@"
log=$build/build.log
cmake --build "$build" >"$log" 2>&1 || {
  cat "$log"
  die "the build failed"
}
warnings=$(grep 'warning:' "$log" | grep -v '/_deps/' || true)
if [ -n "$warnings" ]; then
  printf '%s\n' "$warnings"
  die "the build has warnings"
fi
echo "build: no warnings"

tests=$build/th_tests
[ -x "$tests" ] || tests=$build/th_tests.exe
[ -x "$tests" ] || die "no th_tests in $build"
# No game data: the tests that need it are skipped, and th_tests then exits
# 77 (ctest's skip code) once every other test has passed.
status=0
env -u TH_TEST_DATA_DIR "$tests" || status=$?
[ "$status" -eq 0 ] || [ "$status" -eq 77 ] || die "th_tests failed (exit $status)"

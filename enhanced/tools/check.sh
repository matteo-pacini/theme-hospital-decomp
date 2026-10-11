#!/bin/sh
# The gate for enhanced/: runs, in order and stopping at the first failure,
# one line per step: build (build/enhanced, configured if missing; any
# `warning:` from our own targets fails), th_tests, tidy.sh, clang-format
# --dry-run over enhanced/'s sources, drive_tests.sh, install_tests.sh, and
# replay_all.sh. A failing step prints the end of its log.
#
# Usage (from any directory, a plain terminal; it enters the dev shells
# itself, with LD_LIBRARY_PATH unset, and runs at nice 19 / ionice idle):
#   enhanced/tools/check.sh [SCRIPT ...]
#     SCRIPT  replay scripts to run (names or paths) in the last step;
#             without any, every script in tools/replay/scripts.
#
# Environment:
#   CACHE       use|only|refresh, passed to replay_all.sh (default use)
#   FRESH_MAX   with CACHE=use, the most scripts without a cached original
#               that a run accepts before the replays (default 10): more
#               would run DOSBox for a long time, so it stops and says how
#               to proceed
#   CMAKE_BUILD_PARALLEL_LEVEL, TIDY_JOBS, JOBS
#               job caps of the build, tidy and replays (default 2, 2, 1, for
#               a machine shared with other agents; 4, 4, 4 alone)
#   and replay_all.sh's own settings (BUILD, TURNS, RAND, KEEP, WATCHDOG ...)
# Logs go to build/check-<pid>/, removed when the gate passes.
#
# Exit codes: 0 all steps passed, 1 a step failed, 2 a tool is missing.
set -u

root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root" || exit 2
self=$root/enhanced/tools/check.sh
export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
export TIDY_JOBS="${TIDY_JOBS:-2}"
export JOBS="${JOBS:-1}"
CACHE=${CACHE:-use}
export CACHE

# Outside the dev shells: run each phase inside its shell (the enhanced one
# for build and tests, the decomposition's for the replays, which need
# python3 and dosbox-x), and stop at the first failing phase.
if [ -z "${TH_CHECK_PHASE:-}" ]; then
  command -v nix >/dev/null 2>&1 || { echo "check.sh: nix is not on PATH" >&2; exit 2; }
  nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_CHECK_PHASE=enhanced \
    nix develop path:./nix#enhanced -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$self" "$@" || exit $?
  exec nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_CHECK_PHASE=replay \
    nix develop path:./nix -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$self" "$@"
fi

logs=$root/build/check-$$
mkdir -p "$logs"

# step NAME COMMAND [ARG ...]: runs the command with its output in
# $logs/NAME.log and prints one line; on failure prints the log's end and
# stops the gate.
step()
{
  name=$1
  shift
  start=$(date +%s)
  if "$@" >"$logs/$name.log" 2>&1; then
    passes=$(grep -c "^pass " "$logs/$name.log")
    summary=$(tail -n 1 "$logs/$name.log")
    [ "$passes" = 0 ] || summary="$passes passed"
    echo "pass $name ($(($(date +%s) - start)) s): $summary"
  else
    echo "FAIL $name ($(($(date +%s) - start)) s): see $logs/$name.log"
    tail -n 20 "$logs/$name.log"
    exit 1
  fi
}

# build: configure when needed (Release: the replays run th_replay), then
# build; every warning outside the fetched dependencies (build/enhanced/_deps)
# fails.
build()
{
  [ -f build/enhanced/CMakeCache.txt ] ||
    cmake -S enhanced -B build/enhanced -G Ninja -DCMAKE_BUILD_TYPE=Release || return 1
  cmake --build build/enhanced >"$logs/build.full" 2>&1 || {
    cat "$logs/build.full"
    return 1
  }
  warnings=$(grep 'warning:' "$logs/build.full" | grep -v '/_deps/')
  [ -z "$warnings" ] || {
    printf '%s\n' "$warnings"
    return 1
  }
  echo "no warnings"
}

format()
{
  command -v clang-format >/dev/null 2>&1 || {
    echo "clang-format is not on PATH"
    return 1
  }
  out=$(find enhanced/src enhanced/tests enhanced/tools \( -name '*.cpp' -o -name '*.h' \) -print0 |
    xargs -0 clang-format --dry-run 2>&1)
  [ -z "$out" ] || {
    printf '%s\n' "$out"
    return 1
  }
  echo "clean"
}

replays()
{
  enhanced/tools/replay/replay_all.sh "$@" >"$logs/replay_all.full" 2>&1
  code=$?
  if [ "$code" = 0 ]; then
    echo "$(grep ' passed, ' "$logs/replay_all.full"); $(grep '^original trace:' "$logs/replay_all.full")"
  else
    grep -v ': agree' "$logs/replay_all.full"
  fi
  return "$code"
}

if [ "$TH_CHECK_PHASE" = enhanced ]; then
  step build build
  step th_tests env TH_TEST_DATA_DIR=orig build/enhanced/th_tests
  step tidy enhanced/tools/tidy.sh
  step format format
  step drive_tests enhanced/tools/drive_tests.sh
  step install_tests enhanced/tools/install_tests.sh
else
  # Before the replays: with CACHE=use a script without a cached original
  # runs DOSBox, so many of them stop the gate rather than start a long run
  # unasked (CACHE=only never runs DOSBox; CACHE=refresh asks for it).
  if [ "$CACHE" = use ]; then
    missing=$(enhanced/tools/replay/trace_cache.sh missing "$@") || exit 1
    fresh=$(printf '%s\n' "$missing" | grep -c . || true)
    echo "$fresh fresh originals"
    if [ "$fresh" -gt "${FRESH_MAX:-10}" ]; then
      echo "more than FRESH_MAX=${FRESH_MAX:-10}: fill the cache first with"
      echo "  enhanced/tools/replay/trace_cache.sh prefetch [SCRIPT ...]"
      echo "or run again with FRESH_MAX raised"
      exit 1
    fi
  fi
  step replays replays "$@"
fi
rm -rf "$logs"

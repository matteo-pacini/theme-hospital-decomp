#!/bin/sh
# The gate inside enhanced/tools/remote.sh's container, run from the root of
# the tree it was sent: builds the decompilation (build/base) and
# build/enhanced, runs th_tests, tidy.sh, the clang-format check and the drive
# tests, then replay_all.sh, and prints how long each step took. Failing replay
# pairs, and the drive tests' output when one fails, are left under
# build/remote-out for remote.sh to copy back.
#
# Environment (set by remote.sh):
#   GATE_JOBS     parallel jobs for every step (default: nproc)
#   GATE_SCRIPTS  the replay scripts to run, separated by spaces (default:
#                 all); "none" skips the replays
#   CACHED_ONLY, KEEP, NO_CACHE, TURNS, SKIP, ...: passed to replay_all.sh
#
# Exits 1 if a step before the replays fails, else with replay_all.sh's
# status (0 pass, 1 a disagreement, 3 scripts without a cached original).
set -u

jobs=${GATE_JOBS:-$(nproc)}
export CMAKE_BUILD_PARALLEL_LEVEL=$jobs TIDY_JOBS=$jobs
# Under the container's CPU limit, still yield to the host's other work;
# every step inherits it.
renice -n 19 $$ >/dev/null
ionice -c 3 -p $$ 2>/dev/null || true

# step NAME command...: runs it, prints its time, and stops the gate on
# failure.
step()
{
  name=$1
  shift
  start=$(date +%s)
  echo "=== $name"
  "$@"
  rc=$?
  echo "=== $name: exit $rc, $(($(date +%s) - start))s"
  if [ "$rc" != 0 ]; then
    echo "gate: $name failed" >&2
    exit 1
  fi
}

format_check()
{
  out=$(find enhanced/src enhanced/tests enhanced/tools \
    \( -name '*.cpp' -o -name '*.h' \) -print0 |
    xargs -0 -P "$jobs" -n 20 clang-format --dry-run 2>&1)
  [ -z "$out" ] || {
    printf '%s\n' "$out"
    return 1
  }
}

# drive_tests: drive_tests.sh, one script per call and GATE_JOBS at once (each
# runs its seed script into a folder of its own, so they are independent),
# keeping the output (logs, screenshots, dumps) for remote.sh when one fails.
drive_tests()
{
  printf '%s\0' enhanced/tests/drive/*.txt | TH_DRIVE_SLOTS=$jobs \
    xargs -0 -P "$jobs" -n 1 enhanced/tools/drive_tests.sh || {
    cp -r build/drive-tests build/remote-out/
    return 1
  }
}

# remote.sh mounts the game data at /orig and the shared replay cache at
# /cache; podman creates mount points owned by root, so not inside the tree.
ln -sfn /orig orig
mkdir -p build/remote-out
ln -sfn /cache build/replay-cache
step configure-base python3 configure.py
step build-base ninja -j "$jobs"
step configure-enhanced cmake -S enhanced -B build/enhanced -G Ninja
step build-enhanced cmake --build build/enhanced
step tests env TH_TEST_DATA_DIR=orig build/enhanced/th_tests
step tidy enhanced/tools/tidy.sh
step format format_check
step drive drive_tests

[ "${GATE_SCRIPTS:-}" = none ] && exit 0
start=$(date +%s)
echo "=== replays"
# shellcheck disable=SC2086 # GATE_SCRIPTS is a list of names
JOBS=$jobs TH_REPLAY_SLOTS=$jobs PREFETCH_JOBS=$jobs \
  REPLAY_BASE=build/remote-out/replay \
  enhanced/tools/replay/replay_all.sh ${GATE_SCRIPTS:-}
rc=$?
echo "=== replays: exit $rc, $(($(date +%s) - start))s"
exit "$rc"

#!/bin/sh
# The gate for enhanced/: runs, in order and stopping at the first failure,
# one line per step: build (build/enhanced, configured if missing; any
# `warning:` from our own targets fails), th_tests, tidy.sh, clang-format
# --dry-run over enhanced/'s sources, drive_tests.sh, install_tests.sh, and
# replay_all.sh. A failing step prints the end of its log. With WEB=1 a last
# phase follows, in the `web` dev shell (WebAssembly; see enhanced/README.md,
# "Web build"): the wasm th_tests and th_replay build with no warning, the
# wasm th_tests, tidy over the files with an __EMSCRIPTEN__ region, every
# script's trace from wasm and native th_replay compared byte for byte, and
# the snapshot cross-load test.
#
# The replays of both phases keep a cache of passes (tools/replay/port_cache.sh,
# build/replay-cache/port/): a script whose key (the port's sources, build
# configuration, checking tools and the original's trace key) has a stored
# pass is not run, and its stored line is printed; the summary counts them,
# e.g. "158 passed (150 cached, 8 run)". Failures are never stored. The wasm
# records count only with the native one for the same key.
#
# Usage (from any directory, a plain terminal; it enters the dev shells
# itself, with LD_LIBRARY_PATH unset, and runs at nice 19 / ionice idle):
#   enhanced/tools/check.sh [SCRIPT ...]
#     SCRIPT  replay scripts to run (names or paths) in the last step;
#             without any, every script in tools/replay/scripts.
#
# Environment:
#   STEPS       the steps to run, space or comma separated, of: build th_tests
#               tidy format drive_tests install_tests replays web_build
#               web_th_tests web_tidy web_replays web_cross_load (default: all;
#               the web ones only with WEB=1, or when named here). A phase with
#               no step selected is not entered. Naming a web replay step also
#               runs the replay phase's native verdict keys, which it reads.
#   NO_DATA     1: th_tests and web_th_tests run without a data folder and an
#               exit code 77 (all tests skipped) passes; for a checkout with
#               no game files, where only the data-free steps are meaningful
#   CACHE       use|only|refresh, passed to replay_all.sh (default use);
#               refresh also runs the port side of every replay again
#   PORT_CACHE  use|off (default use): off runs the port side of every replay
#               again (native and wasm) and stores the passes again
#   FRESH_MAX   with CACHE=use, the most scripts without a cached original
#               that a run accepts before the replays (default 10): more
#               would run DOSBox for a long time, so it stops and says how
#               to proceed
#   WEB         1 adds the WebAssembly phase (default 0); the replays of that
#               phase run JOBS at a time
#   CROSS_LOAD  the cross-load test's SCRIPT:TURN items, the turn the snapshot
#               is taken at (default: rival_two_years:18000 win_level1:9000
#               tutorial:9000)
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

all_steps="build th_tests tidy format drive_tests install_tests replays web_build web_th_tests web_tidy web_replays web_cross_load"

# want NAME: whether STEPS (default: every step) selects the step.
want()
{
  [ -z "${STEPS:-}" ] && return 0
  case " $(printf '%s' "$STEPS" | tr ',' ' ') " in *" $1 "*) return 0 ;; esac
  return 1
}

for s in $(printf '%s' "${STEPS:-}" | tr ',' ' '); do
  case " $all_steps " in *" $s "*) ;; *) echo "check.sh: unknown step $s in STEPS (steps: $all_steps)" >&2; exit 2 ;; esac
done

# Outside the dev shells: run each phase inside its shell (the enhanced one
# for build and tests, the decomposition's for the replays, which need
# python3 and dosbox-x, the web one with WEB=1), and stop at the first
# failing phase.
if [ -z "${TH_CHECK_PHASE:-}" ]; then
  command -v nix >/dev/null 2>&1 || { echo "check.sh: nix is not on PATH" >&2; exit 2; }
  # The replay phase leaves the scripts' native port verdict keys here for the
  # web phase: a key holds the original's trace key, which the web shell
  # (no python3, no DOSBox) cannot compute as the replay shell does.
  TH_CHECK_STATE=$root/build/check-state-$$
  export TH_CHECK_STATE
  mkdir -p "$TH_CHECK_STATE"
  trap 'rm -rf "$TH_CHECK_STATE"' EXIT
  for s in build th_tests tidy format drive_tests install_tests; do
    if want $s; then
      nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_CHECK_PHASE=enhanced \
        nix develop path:./nix#enhanced -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$self" "$@" || exit $?
      break
    fi
  done
  if want replays || { [ -n "${STEPS:-}" ] && { want web_replays || want web_cross_load; }; } ||
    { [ -z "${STEPS:-}" ] && [ "${WEB:-0}" = 1 ]; }; then
    nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_CHECK_PHASE=replay \
      nix develop path:./nix -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$self" "$@" || exit $?
  fi
  if [ -z "${STEPS:-}" ]; then
    [ "${WEB:-0}" = 1 ] || exit 0
  fi
  for s in web_build web_th_tests web_tidy web_replays web_cross_load; do
    if want $s; then
      nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_CHECK_PHASE=web \
        nix develop path:./nix#web -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$self" "$@"
      exit $?
    fi
  done
  exit 0
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

# tests_ok CMD ARG ...: runs a test binary; with NO_DATA=1 its exit code 77
# (every test skipped: no data folder) is a pass.
tests_ok()
{
  "$@"
  code=$?
  [ "${NO_DATA:-0}" = 1 ] && [ "$code" = 77 ] && code=0
  return "$code"
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
    echo "$(grep -E '^[0-9]+ passed ' "$logs/replay_all.full"); $(grep '^original trace:' "$logs/replay_all.full")"
  else
    grep -v ': agree' "$logs/replay_all.full"
  fi
  return "$code"
}


# The WebAssembly phase (WEB=1), in the `web` dev shell. node reads the host's
# files (NODERAWFS) but lists no folder given as a link, and `orig` is one in
# a worktree, so the data folder is passed to th_tests and th_replay by its
# real path.
#
# web_build: build/web is configured when missing, as README.md's "Web build"
# says; every target builds, and any `warning:` outside _deps fails.
web_build()
{
  [ -f build/web/CMakeCache.txt ] ||
    emcmake cmake -S enhanced -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release || return 1
  cmake --build build/web >"$logs/web_build.full" 2>&1 || {
    cat "$logs/web_build.full"
    return 1
  }
  warnings=$(grep 'warning:' "$logs/web_build.full" | grep -v '/_deps/')
  [ -z "$warnings" ] || {
    printf '%s\n' "$warnings"
    return 1
  }
  echo "no warnings"
}

# web_tidy: tidy.sh over the .cpp files that have a region under
# __EMSCRIPTEN__, as the WebAssembly build compiles them (its compile
# database; target and sysroot as emcc has them). An `error:` from the
# compiler fails too: tidy.sh counts only warnings.
web_tidy()
{
  files=$(grep -rl '__EMSCRIPTEN__' enhanced/src enhanced/tests enhanced/tools --include='*.cpp' | sort)
  [ -n "$files" ] || {
    echo "no file has an __EMSCRIPTEN__ region"
    return 0
  }
  sysroot=$EM_CACHE/sysroot
  # shellcheck disable=SC2086
  TIDY_BUILD=build/web TIDY_EXTRA="--target=wasm32-unknown-emscripten -nostdlibinc -isystem$sysroot/include/c++/v1 -isystem$sysroot/include/compat -isystem$sysroot/include" \
    enhanced/tools/tidy.sh $files >"$logs/web_tidy.out" 2>&1
  code=$?
  cat "$logs/web_tidy.out"
  if grep -q ' error: ' "$logs/web_tidy.out"; then
    return 1
  fi
  return "$code"
}

# web_cache_init: sources the replay tools and sets up the port verdict cache
# (port_cache.sh) once; use_verdicts is 1 when a stored pass may stand for a
# run (PORT_CACHE=use and not CACHE=refresh).
web_cache_init()
{
  [ -z "${web_cache_ready:-}" ] || return 0
  here=$root/enhanced/tools/replay
  # shellcheck source=replay/lib.sh
  . "$here/lib.sh"
  # shellcheck source=replay/port_cache.sh
  . "$here/port_cache.sh"
  PORT_NATIVE_KEYS=${TH_CHECK_STATE:-/nonexistent}/native_keys
  port_cache_init build/enhanced build/web || true
  use_verdicts=0
  [ "$PORT_CACHE" = use ] && [ "$CACHE" != refresh ] && use_verdicts=1
  web_cache_ready=1
}

# web_replay_one SCRIPT RESULTS: native th_replay and the wasm one run the
# script with replay_all.sh's arguments; the traces must be byte-identical.
# Equal traces are deleted, different ones kept zstd-compressed. A stored
# pass for the script's current web key (with the native pass for the same
# key) stands for the run; with a native pass stored, only the wasm one runs
# and its trace's hash is compared with the native trace's.
web_replay_one()
{
  f=$1
  n=$(basename "$f" .txt)
  lvl=$(script_level "$f")
  t=$(script_turns "$f")
  if [ "$use_verdicts" = 1 ] && stored=$(port_get web "$f"); then
    echo "$stored" >"$2/$n.line"
    echo verdict >"$2/$n.src"
    echo 0 >"$2/$n.status"
    return 0
  fi
  dir=build/web-replay/$n
  mkdir -p "$dir"
  flags="--data $(readlink -f orig) --script $f --turns $t --advisor --level $lvl"
  [ "$RAND" = 1 ] && flags="$flags --rand"
  if [ "$use_verdicts" = 1 ] && native_hash=$(port_trace native "$f"); then
    # shellcheck disable=SC2086
    if node build/web/th_replay.js $flags --out "$dir/wasm.txt" >"$dir/wasm.log" 2>&1 &&
      [ "$(sha256sum <"$dir/wasm.txt" | cut -c1-64)" = "$native_hash" ]; then
      rm -rf "$dir"
      echo "$n: identical" >"$2/$n.line"
      port_put web "$f" "" "$n: identical" "$native_hash"
      echo 0 >"$2/$n.status"
      return 0
    fi
    rm -f "$dir/wasm.txt" "$dir/wasm.log"
  fi
  # shellcheck disable=SC2086
  if ! build/enhanced/th_replay $flags --out "$dir/native.txt" >"$dir/native.log" 2>&1; then
    echo "$n: native run failed (see $dir/native.log)" >"$2/$n.line"
    echo 1 >"$2/$n.status"
    return 1
  fi
  # shellcheck disable=SC2086
  if ! node build/web/th_replay.js $flags --out "$dir/wasm.txt" >"$dir/wasm.log" 2>&1; then
    echo "$n: wasm run failed (see $dir/wasm.log)" >"$2/$n.line"
    echo 1 >"$2/$n.status"
    return 1
  fi
  if cmp -s "$dir/native.txt" "$dir/wasm.txt"; then
    hash=$(sha256sum <"$dir/wasm.txt" | cut -c1-64)
    rm -rf "$dir"
    echo "$n: identical" >"$2/$n.line"
    port_put web "$f" "" "$n: identical" "$hash"
    echo 0 >"$2/$n.status"
    return 0
  fi
  zstd -q -f --rm "$dir/native.txt" "$dir/wasm.txt"
  echo "$n: traces differ (kept $dir/native.txt.zst $dir/wasm.txt.zst; first difference: $(cmp "$dir/native.txt.zst" "$dir/wasm.txt.zst" 2>&1 | head -n 1))" >"$2/$n.line"
  echo 1 >"$2/$n.status"
  return 1
}

# web_replays [SCRIPT ...]: web_replay_one for every script (replay_all.sh's
# list), JOBS at a time.
web_replays()
{
  web_cache_init
  scripts=$(script_list "$@") || return 1
  results=$(mktemp -d "$logs/web-replays.XXXXXX")
  running=0
  for f in $scripts; do
    web_replay_one "$f" "$results" &
    running=$((running + 1))
    if [ "$running" -ge "$JOBS" ]; then
      wait
      running=0
    fi
  done
  wait
  passed=0
  failed=0
  verdicts=0
  for f in $scripts; do
    n=$(basename "$f" .txt)
    [ "$(cat "$results/$n.status" 2>/dev/null)" = 0 ] || {
      cat "$results/$n.line" 2>/dev/null || echo "$n: no result"
      failed=$((failed + 1))
      continue
    }
    passed=$((passed + 1))
    [ "$(cat "$results/$n.src" 2>/dev/null)" = verdict ] && verdicts=$((verdicts + 1))
  done
  rmdir build/web-replay 2>/dev/null
  echo "$passed traces identical ($verdicts cached, $((passed - verdicts)) run), $failed failed"
  [ "$failed" = 0 ]
}

# cross_one NAME N: the script's snapshot, written by one build at turn N,
# carries on in the other build to the script's last turn M with the trace
# the first build's whole run has from turn N + 1, advice lines included;
# both ways round. A stored pass for the script's web key and turn N (and
# its native pass) stands for the run.
cross_one()
{
  script=$1
  f=$(script_path "$script") || return 1
  n=$2
  if [ "$use_verdicts" = 1 ] && stored=$(port_get web "$f" "cross-$n"); then
    echo "$stored"
    return 0
  fi
  lvl=$(script_level "$f")
  m=$(script_turns "$f")
  dir=build/web-cross/$script
  mkdir -p "$dir"
  flags="--data $(readlink -f orig) --advisor --level $lvl"
  [ "$RAND" = 1 ] && flags="$flags --rand"
  # The script's later commands, their turns counted from the snapshot's.
  awk -v n="$n" '/^[0-9]+[ \t]/ { t = $1 + 0; if (t > n) { sub(/^[0-9]+/, t - n); print } }' "$f" >"$dir/rest.txt"
  from=$((n + 1))
  # shellcheck disable=SC2086
  build/enhanced/th_replay $flags --script "$f" --turns "$m" --out "$dir/whole.txt" || return 1
  awk -v t="$from" '$1 == "T" && $2 == t { p = 1 } p' "$dir/whole.txt" >"$dir/whole.tail"
  rm -f "$dir/whole.txt"
  for pair in "native wasm" "wasm native"; do
    from_build=${pair% *}
    to_build=${pair#* }
    snap=$dir/$from_build.bin
    out=$dir/$from_build-$to_build
    # shellcheck disable=SC2086
    run_th_replay "$from_build" $flags --script "$f" --turns "$n" --out /dev/null --snapshot "$snap" || return 1
    # shellcheck disable=SC2086
    run_th_replay "$to_build" $flags --load "$snap" --script "$dir/rest.txt" --turns "$m" --out "$out.txt" || return 1
    awk -v t="$from" '$1 == "T" && $2 == t { p = 1 } p' "$out.txt" >"$out.tail"
    cmp -s "$dir/whole.tail" "$out.tail" || {
      echo "$script: $from_build snapshot at turn $n, $to_build from there: trace differs from the whole run's ($dir)"
      return 1
    }
    rm -f "$out.txt" "$out.tail" "$snap"
  done
  line="$script: turn $n of $m, native and wasm snapshots continue identically (trace from turn $from: $(wc -l <"$dir/whole.tail") lines)"
  echo "$line"
  port_put web "$f" "cross-$n" "$line" "$(port_trace native "$f")"
  rm -rf "$dir"
}

# run_th_replay native|wasm ARG ...
run_th_replay()
{
  which=$1
  shift
  if [ "$which" = native ]; then
    build/enhanced/th_replay "$@"
  else
    node build/web/th_replay.js "$@"
  fi
}

# web_cross_load: cross_one for each of CROSS_LOAD's NAME:TURN items.
web_cross_load()
{
  web_cache_init
  for item in ${CROSS_LOAD:-rival_two_years:18000 win_level1:9000 tutorial:9000}; do
    cross_one "${item%%:*}" "${item##*:}" || return 1
  done
  rmdir build/web-cross 2>/dev/null
  return 0
}
if [ "$TH_CHECK_PHASE" = enhanced ]; then
  want build && step build build
  want th_tests && step th_tests tests_ok env TH_TEST_DATA_DIR=orig build/enhanced/th_tests
  want tidy && step tidy enhanced/tools/tidy.sh
  want format && step format format
  want drive_tests && step drive_tests enhanced/tools/drive_tests.sh
  want install_tests && step install_tests enhanced/tools/install_tests.sh
elif [ "$TH_CHECK_PHASE" = web ]; then
  want web_build && step web_build web_build
  want web_th_tests && step web_th_tests tests_ok env TH_TEST_DATA_DIR="$(readlink -f orig)" node build/web/th_tests.js
  want web_tidy && step web_tidy web_tidy
  want web_replays && step web_replays web_replays "$@"
  want web_cross_load && step web_cross_load web_cross_load
else
  # Before the replays: with CACHE=use a script without a cached original
  # runs DOSBox, so many of them stop the gate rather than start a long run
  # unasked (CACHE=only never runs DOSBox; CACHE=refresh asks for it).
  if want replays && [ "$CACHE" = use ]; then
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
  if want web_replays || want web_cross_load; then
    enhanced/tools/replay/trace_cache.sh native-keys "$@" >"$TH_CHECK_STATE/native_keys" || exit 1
  fi
  want replays && step replays replays "$@"
fi
rm -rf "$logs"

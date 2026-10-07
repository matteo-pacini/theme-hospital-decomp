#!/bin/sh
# Line, function and branch coverage of the engine over every replay script
# (enhanced/tools/replay/scripts): builds th_replay with -DTH_COVERAGE=ON in
# build/coverage, runs each script as the replay check does (the level of its
# "# Level N" line, to its "# Turns N" or turn 18300, with --rand, and with
# --advisor as the game's frame runs it) and writes the report to
# build/coverage/report/: index.html (per file and line), summary.txt
# (uncovered lines per file, then the totals) and coverage.json. The engine
# side only: no DOSBox. Needs GCC (gcov data); gcovr comes from nixpkgs
# through `nix shell`.
#
# Run from the repository root in the dev shell
# (`nix develop path:./nix#enhanced`). COVERAGE_JOBS replays run at once
# (default 8); CMAKE_BUILD_PARALLEL_LEVEL caps the build as usual.
set -eu
build=build/coverage
report=$build/report

# One replay (run by xargs below). Its trace (tens of MB with --rand) is not
# kept; its standard error goes to build/coverage/logs.
if [ "${1:-}" = --replay ]; then
  level=$(sed -n 's/^# Level \([0-9]*\).*/\1/p' "$2" | head -n 1)
  turns=$(sed -n 's/^# Turns \([0-9]*\).*/\1/p' "$2" | head -n 1)
  name=$(basename "$2" .txt)
  if "$build/th_replay" --level "${level:-1}" --script "$2" --turns "${turns:-18300}" \
      --rand --advisor --out /dev/null 2>"$build/logs/$name.err"; then
    echo "ok   $name"
  else
    echo "FAIL $name"
  fi
  exit 0
fi

if [ ! -f enhanced/CMakeLists.txt ]; then
  echo "run from the repository root" >&2
  exit 1
fi
unset LD_LIBRARY_PATH

cmake -S enhanced -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DTH_COVERAGE=ON -DBUILD_TESTING=OFF >/dev/null
if ! grep -qs 'CMAKE_CXX_COMPILER_ID "GNU"' "$build"/CMakeFiles/*/CMakeCXXCompiler.cmake; then
  echo "coverage.sh reads gcov data: configure $build with GCC" >&2
  exit 1
fi
cmake --build "$build" --target th_replay

# The counters add up across runs: start from none. libgcov locks each .gcda
# while it merges into it, so the replays can run at the same time.
find "$build" -name '*.gcda' -delete
rm -rf "$report" "$build/logs"
mkdir -p "$report" "$build/logs"

ls enhanced/tools/replay/scripts/*.txt |
  xargs -P "${COVERAGE_JOBS:-8}" -n 1 "$0" --replay | sort -k 2 | tee "$build/runs.txt"

# No limit on hits: the advisor's loops over every thing, run each turn of
# every replay, pass gcovr's default (2^32), which it takes for corrupt data.
nix shell nixpkgs#gcovr -c gcovr --root enhanced --object-directory "$build" \
  --filter 'enhanced/src/' --filter 'enhanced/tools/replay/' \
  --gcov-executable gcov --exclude-unreachable-branches --exclude-throw-branches \
  --gcov-suspicious-hits-threshold 0 \
  -j "${COVERAGE_JOBS:-8}" --html-title "th_replay coverage" \
  --html-details "$report/index.html" --json "$report/coverage.json" \
  --txt "$report/summary.txt" --print-summary >"$report/totals.txt"
cat "$report/totals.txt" >>"$report/summary.txt"
rm "$report/totals.txt"
tail -n 3 "$report/summary.txt"
! grep -q '^FAIL' "$build/runs.txt"

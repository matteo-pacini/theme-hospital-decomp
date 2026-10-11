# shellcheck shell=sh
# The port-side verdict cache, sourced (not run) after lib.sh by
# replay_all.sh, trace_cache.sh and check.sh's web phase; the sourcing script
# sets `here` and `root` first and runs from the repository root.
#
# A record is the verdict of one script's run of the port against the
# original (native) or of the wasm th_replay against the native one (web, and
# the snapshot cross-load test): the key, the result line as printed, the
# Expect counts and the hash of the port's trace. Only passes are stored.
# Records live in build/replay-cache/port/, one file per script and key
# (<kind>.<script>.<tag>.<key>.rec), shared by every worktree: the key
# holds bytes, never paths.
#
# The key is a hash of everything that decides the verdict, so any change is
# a miss; where unsure the input goes in (a miss, never a false hit):
#   native  the sources th_replay is built from (enhanced/src/core, its
#           main.cpp, CMakeLists.txt, cmake/, nix/flake.lock and flake.nix,
#           which pin the compiler and the dependencies), the build type, flags
#           and compiler the build folder was configured with, the verdict
#           tools (compare.py and check_expect.py; replay_all.sh and this
#           file without comment-only and blank lines), and, per script, the
#           original's trace key (trace_key.sh: the script's bytes, turns,
#           level, RAND, DRAW, the instrumented executable, the DOSBox run).
#   web     the native key, enhanced/web/, check.sh (without comment-only and
#           blank lines: the web phase's commands) and the wasm build
#           folder's configuration. The wasm link options are in
#           CMakeLists.txt; Emscripten and node come from flake.lock.
# The game data folder is not in the key: it is the installed game, as
# trace_key.sh treats it, and its path is baked into the binary (so a
# worktree's differs from the main tree's).
#
# A record is stored only when the binary (th_replay, th_replay.js) is newer
# than every file it is built from (PORT_BIN_OK): the key is of the sources,
# so a stale binary must not write a verdict for them. Reading needs no such
# check. This does not see a file restored with an old modification time.

: "${here:?port_cache.sh is sourced by a script that sets here}"
: "${root:?port_cache.sh is sourced by a script that sets root}"

port_dir=$root/build/replay-cache/port

# port_strip_text FILE: the file without comment-only and blank lines; every
# other line stays as written. Safe only for a file with no multi-line
# string or heredoc whose lines look like comments or are blank; the files
# it is used on have none (test_cache.sh checks that none has a heredoc).
port_strip_text()
{
  if [ -f "$1" ]; then grep -v '^[[:space:]]*#' "$1" | grep -v '^[[:space:]]*$' || true; else echo missing; fi
}

port_file_hash()
{
  if [ -f "$1" ]; then sha256sum <"$1" | cut -c1-64; else echo missing; fi
}

# port_build_config DIR: what the folder was configured with that changes
# the code generated: type, flags, compilers, linker flags, our own options
# and the compiler's id and version. The data folder (TH_DEV_DATA_DIR) is
# left out; the absolute paths of a worktree must not split the key.
port_build_config()
{
  if [ ! -f "$1/CMakeCache.txt" ]; then
    echo none
    return 0
  fi
  grep -E '^(CMAKE_BUILD_TYPE|CMAKE_[A-Z]+_FLAGS[A-Z_]*|CMAKE_[A-Z]+_COMPILER|CMAKE_[A-Z_]*LINKER_FLAGS[A-Z_]*|CMAKE_TOOLCHAIN_FILE|CMAKE_CXX_STANDARD|CMAKE_INTERPROCEDURAL_OPTIMIZATION|CMAKE_GENERATOR|CMAKE_MSVC_[A-Z_]*|TH_[A-Z_]*|EMSCRIPTEN[A-Z_]*):' "$1/CMakeCache.txt" |
    grep -v '^TH_DEV_DATA_DIR:' | sort || true
  for f in "$1"/CMakeFiles/*/CMakeCXXCompiler.cmake; do
    if [ -f "$f" ]; then grep -E '^set\(CMAKE_CXX_COMPILER_(ID|VERSION|ARCHITECTURE_ID) ' "$f" || true; fi
  done
}

# port_native_sources: the files the native key covers, one per line, as
# paths from the root.
port_native_sources()
{
  find enhanced/src/core enhanced/cmake -type f | sort
  printf '%s\n' enhanced/tools/replay/main.cpp enhanced/CMakeLists.txt nix/flake.lock nix/flake.nix
}

# port_web_sources: the files the web key covers beyond the native ones.
port_web_sources()
{
  find enhanced/web -type f | sort
}

# port_common_native BUILD: the part of the native key that is the same for
# every script.
port_common_native()
{
  {
    port_native_sources | while read -r f; do printf '%s %s\n' "$(port_file_hash "$f")" "$f"; done
    printf 'config\n'
    port_build_config "$1"
    printf 'tool %s\n' "$(port_file_hash "$here/compare.py")" "$(port_file_hash "$here/check_expect.py")"
    for f in replay_all.sh port_cache.sh; do
      printf 'tool %s\n' "$f"
      port_strip_text "$here/$f"
    done
  } | sha256sum | cut -c1-16
}

# port_common_web NATIVE WEBBUILD: NATIVE (port_common_native's) and the web
# phase's inputs.
port_common_web()
{
  {
    printf 'native %s\n' "$1"
    port_web_sources | while read -r f; do printf '%s %s\n' "$(port_file_hash "$f")" "$f"; done
    printf 'config\n'
    port_build_config "$2"
    printf 'check.sh\n'
    port_strip_text "$root/enhanced/tools/check.sh"
  } | sha256sum | cut -c1-16
}

# port_newer_than BIN FILE...: succeeds when BIN exists and no FILE is newer
# than it.
port_newer_than()
{
  bin=$1
  shift
  [ -e "$bin" ] || return 1
  [ -z "$(find "$@" -newer "$bin" -print -quit 2>/dev/null)" ]
}

# port_cache_init BUILD [WEBBUILD]: sets, for port_key and the others:
#   PORT_EXE_KEY       the instrumented executable's key (empty without
#                      build/base: every native lookup then misses)
#   PORT_COMMON_NATIVE the native key's common part
#   PORT_COMMON_WEB    the web key's common part (with WEBBUILD)
#   PORT_BIN_OK        1 when BUILD/th_replay is newer than the key's files,
#                      PORT_WEB_BIN_OK likewise for WEBBUILD/th_replay.js
# Run from the repository root; call it once, before any background jobs.
port_cache_init()
{
  PORT_EXE_KEY=
  if [ -d build/base ]; then
    PORT_EXE_KEY=$("$here/exe_cache_key.sh" "$root" "$here/instrument.py") || PORT_EXE_KEY=
  fi
  PORT_COMMON_NATIVE=$(port_common_native "$1")
  PORT_BIN_OK=0
  # shellcheck disable=SC2046
  if port_newer_than "$1/th_replay" $(port_native_sources); then PORT_BIN_OK=1; fi
  PORT_COMMON_WEB=
  PORT_WEB_BIN_OK=0
  if [ -n "${2:-}" ]; then
    PORT_COMMON_WEB=$(port_common_web "$PORT_COMMON_NATIVE" "$2")
    # shellcheck disable=SC2046
    if port_newer_than "$2/th_replay.js" $(port_native_sources); then PORT_WEB_BIN_OK=1; fi
  fi
  [ -n "$PORT_EXE_KEY" ]
}

# The functions below run in a subshell: the scripts that source this file
# keep their own variables (`line`, `key` ...).

# port_key KIND SCRIPT [TAG]: the record key of KIND (native|web) for the
# script at path SCRIPT; TAG tells records of one script apart (the
# cross-load test's turn). Prints nothing and fails without an executable
# key (or a native key from PORT_NATIVE_KEYS) or, for web, a web build.
# The web key is built on the script's native key. The native key holds the
# original's trace key, which depends on the shell it is computed in (the
# DOSBox that is on PATH, python3 for the executable's key), and the web
# shell has neither: with PORT_NATIVE_KEYS naming a file of `NAME KEY`
# lines (trace_cache.sh native-keys, run in the replay shell), the native
# key is taken from it.
port_key()
(
  kind=$1 script=$2 tag=${3:-}
  name=$(basename "$script" .txt)
  nkey=
  if [ -n "${PORT_NATIVE_KEYS:-}" ] && [ -f "$PORT_NATIVE_KEYS" ]; then
    nkey=$(awk -v n="$name" '$1 == n { print $2; exit }' "$PORT_NATIVE_KEYS")
  fi
  if [ -z "$nkey" ]; then
    [ -n "$PORT_EXE_KEY" ] || exit 1
    orig=$(TRACE_RAND=$RAND TRACE_DRAW=${TRACE_DRAW:-0} orig_key "$PORT_EXE_KEY" "$(script_turns "$script")" \
      "$(script_level "$script")" "" "$script") || exit 1
    nkey=$(printf 'kind=native\ncommon=%s\norig=%s\n' "$PORT_COMMON_NATIVE" "$orig" | sha256sum | cut -c1-16)
  fi
  if [ "$kind" = native ]; then
    echo "$nkey"
    exit 0
  fi
  [ -n "$PORT_COMMON_WEB" ] || exit 1
  printf 'kind=web\ncommon=%s\nnative=%s\ntag=%s\n' "$PORT_COMMON_WEB" "$nkey" "$tag" | sha256sum | cut -c1-16
)

# port_rec_path KIND SCRIPT TAG KEY: the record's file.
port_rec_path()
{
  printf '%s/%s.%s.%s.%s.rec' "$port_dir" "$1" "$(basename "$2" .txt)" "$3" "$4"
}

# port_field PATH NAME: the first value of a record field.
port_field()
{
  sed -n "s/^$2=//p" "$1" | head -n 1
}

# port_valid KIND SCRIPT TAG: prints the path of the record with the current
# key, and succeeds when there is one.
port_valid()
(
  key=$(port_key "$1" "$2" "$3") || exit 1
  rec=$(port_rec_path "$1" "$2" "$3" "$key")
  [ -f "$rec" ] && [ "$(port_field "$rec" key)" = "$key" ] || exit 1
  echo "$rec"
)

# port_get KIND SCRIPT [TAG]: prints the stored result line and succeeds
# when a valid record has the current key. A web record also needs the
# native record for the script's current native key, with the same trace
# hash: a web pass counts only with its native pass.
port_get()
(
  rec=$(port_valid "$1" "$2" "${3:-}") || exit 1
  if [ "$1" = web ]; then
    nrec=$(port_valid native "$2" "") || exit 1
    [ "$(port_field "$nrec" trace)" = "$(port_field "$rec" trace)" ] || exit 1
  fi
  port_field "$rec" line
)

# port_trace KIND SCRIPT [TAG]: prints the trace hash of a valid record.
port_trace()
(
  rec=$(port_valid "$1" "$2" "${3:-}") || exit 1
  port_field "$rec" trace
)

# port_put KIND SCRIPT TAG LINE TRACE_HASH [EXPECT_TEXT]: stores a pass
# (an atomic rename, so a reader never sees half a record). Does nothing
# when the binary is older than the files it is built from.
port_put()
(
  kind=$1 script=$2 tag=$3
  case $kind in
    native) [ "${PORT_BIN_OK:-0}" = 1 ] || exit 0 ;;
    *) [ "${PORT_WEB_BIN_OK:-0}" = 1 ] || exit 0 ;;
  esac
  key=$(port_key "$kind" "$script" "$tag") || exit 0
  mkdir -p "$port_dir"
  rec=$(port_rec_path "$kind" "$script" "$tag" "$key")
  tmp=$port_dir/.$$.$(basename "$rec")
  {
    printf 'key=%s\n' "$key"
    printf 'trace=%s\n' "$5"
    printf 'line=%s\n' "$4"
    printf '%s\n' "${6:-}" | sed -n 's/^\(.\)/expect=\1/p'
  } >"$tmp"
  mv "$tmp" "$rec"
)

#!/bin/sh
# Runs the full gate (remote_gate.sh: both builds, th_tests, tidy, the
# clang-format check and replay_all.sh) for this tree on another machine, in
# a rootless podman container, instead of on this one.
#
# Usage (from anywhere in a checkout or worktree):
#   enhanced/tools/remote.sh gate [SCRIPT ...]  the gate; replay SCRIPTs only
#                                               if given ("none": no replays)
#   enhanced/tools/remote.sh clean              remove every image, container
#                                               and the cache volume it made
#
# What is sent: the files git tracks plus untracked ones not ignored, as they
# are on disk (uncommitted changes included), as a tar on the container's
# stdin; nothing of the tree stays on the host after the run. The host keeps:
#   th-gate-env:<tag>   the toolchain image, built there from nix/flake.nix
#                       (packages.gate-image), loaded when the flake changes
#   th-gate-data:<tag>  the game data from orig/ (without the Watcom copies),
#                       rebuilt when its files change; never pushed anywhere
#   th-replay-cache     a volume with build/replay-cache (instrumented
#                       executables and original traces), shared by all runs
#
# The output is remote_gate.sh's: each step's output and time, then
# replay_all.sh's summary, and the exit status is the same (0, 1 or 3). The
# run's build/remote-out (failing replay pairs) is copied back to
# build/remote-<time>/.
#
# Environment:
#   REMOTE_HOST   ssh destination (default nexus)
#   REMOTE_RUNS   gates run at once on the host; a run waits for a free one
#                 (default 2: at 4 x 16 CPUs and 50 % fans nexus's second
#                 package passed its 88 C maximum)
#   REMOTE_CPUS   CPUs per run, and its jobs for every step (default 16)
#   REMOTE_MEM    memory limit per run (default 24g)
#   REMOTE_FANS   fan duty in % held while a run is going, through the host's
#                 with-fans (optional: skipped where it is not installed);
#                 "off" never sets it (default 50)
#   CACHED_ONLY, KEEP, NO_CACHE, TURNS, SKIP, IGNORE_EVENTS, WATCHDOG,
#   TRACE_DRAW, RAND: passed to replay_all.sh in the container
set -eu

host=${REMOTE_HOST:-nexus}
runs=${REMOTE_RUNS:-2}
cpus=${REMOTE_CPUS:-16}
mem=${REMOTE_MEM:-24g}
fans=${REMOTE_FANS:-50}
root=$(cd "$(dirname "$0")/../.." && pwd)

# q ARG: ARG quoted for a POSIX shell.
q()
{
  printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

# remote SCRIPT ARG...: runs the shell text SCRIPT on the host with ARGs as
# $1...; stdin passes through.
remote()
{
  script=$1
  shift
  cmd="sh -c $(q "$script") remote"
  for a in "$@"; do
    cmd="$cmd $(q "$a")"
  done
  ssh "$host" "$cmd"
}

if [ "${1:-}" = clean ]; then
  remote '
    podman ps -a --format "{{.Names}}" | grep "^th-gate-" | xargs -r podman rm -f
    podman images --format "{{.Repository}}:{{.Tag}}" |
      grep -E "^localhost/th-gate-(env|data):" | xargs -r podman rmi
    podman volume rm -f th-replay-cache'
  echo "remote.sh: the image builds stay in $host's nix store until its next garbage collection" >&2
  exit 0
fi
if [ "${1:-}" != gate ]; then
  echo "usage: remote.sh gate [SCRIPT ...] | remote.sh clean" >&2
  exit 2
fi
shift
[ -d "$root/orig" ] || {
  echo "remote.sh: $root/orig is missing (the game data)" >&2
  exit 2
}

# The toolchain image: built on the host from this tree's flake, and loaded
# unless the host has it already. Only nix/ is sent for it.
env_tag=$(tar -C "$root" -c nix | remote '
  d=$(mktemp -d)
  trap "rm -rf \"\$d\"" EXIT
  tar x -C "$d"
  img="path:$d/nix#gate-image"
  tag=$(nix eval --raw "$img.imageTag")
  if ! podman image exists "th-gate-env:$tag"; then
    echo "remote.sh: building th-gate-env:$tag" >&2
    "$(nice -n 19 nix build --no-link --print-out-paths "$img")" | podman load -q >&2
  fi
  echo "$tag"')

# The game data image: keyed by the names, sizes and times of its files.
data_excludes="--exclude=orig/watcom* --exclude=orig/*.zip --exclude=orig/tmp"
data_tag=$(cd "$root" && find -L orig -type f ! -path 'orig/watcom*' \
  ! -name '*.zip' ! -path 'orig/tmp/*' -printf '%p %s %T@\n' | sort |
  sha256sum | cut -c1-16)
if ! remote 'podman image exists "th-gate-data:$1"' "$data_tag"; then
  echo "remote.sh: sending the game data (th-gate-data:$data_tag)" >&2
  ctx=$(mktemp -d)
  printf 'FROM scratch\nCOPY orig/ /\n' >"$ctx/Containerfile"
  # shellcheck disable=SC2086 # data_excludes is a list of options
  tar -c -h -C "$ctx" Containerfile -C "$root" $data_excludes orig |
    remote 'nice -n 19 podman build -q -t "th-gate-data:$1" - >/dev/null' "$data_tag"
  rm -rf "$ctx"
fi

# The run: waits for one of REMOTE_RUNS slots (flock'd files in the host's
# runtime folder, freed when the holder exits), then runs the container
# with the tree on its stdin. It is left behind until its output is copied.
name=th-gate-$(date +%Y%m%d-%H%M%S)-$$
out=$root/build/remote-${name#th-gate-}
trap 'remote "podman rm -f $(q "$name") >/dev/null 2>&1" || true' EXIT INT TERM
envs=
for v in CACHED_ONLY KEEP NO_CACHE TURNS SKIP IGNORE_EVENTS WATCHDOG TRACE_DRAW RAND; do
  eval "set_=\${$v+x}"
  # shellcheck disable=SC2154 # set_ is set by the eval
  [ -z "$set_" ] || envs="$envs -e $v=$(eval "printf '%s' \"\$$v\"")"
done
rc=0
# One fixed time on every file: Watcom writes each source's path and time
# into its .obj, and build/base goes into the instrumented executable's cache
# key, so the same sources must arrive the same way (always at /build/w) for
# the host's cache to hit.
(cd "$root" && git ls-files -z -co --exclude-standard |
  tar -c --null --ignore-failed-read --mtime='2000-01-01 00:00Z' -T - 2>/dev/null) |
  remote '
    name=$1 runs=$2 cpus=$3 mem=$4 env_tag=$5 data_tag=$6 scripts=$7 envs=$8 fans=$9
    slots=${XDG_RUNTIME_DIR:-/tmp}/th-gate-slots
    mkdir -p "$slots"
    waited=0
    while :; do
      i=0
      while [ "$i" -lt "$runs" ]; do
        exec 9>>"$slots/slot-$i"
        flock -n 9 && break 2
        exec 9>&-
        i=$((i + 1))
      done
      [ "$waited" = 1 ] || echo "remote.sh: waiting for one of $runs runs" >&2
      waited=1
      sleep 5
    done
    # Only the run holds the fans up, not the wait for a slot. with-fans
    # execs podman run, which stays attached for the whole gate (a detached
    # container would outlive its hold).
    hold=
    [ "$fans" = off ] || ! command -v with-fans >/dev/null || hold="with-fans $fans --"
    # Without a terminal a dropped connection sends no SIGHUP: podman run
    # dies on its next write, but the container carries on under conmon
    # (holding the fans too). So a watcher stops it once this ssh session is
    # gone; it runs as a user service, since systemd stops everything left in
    # the session when the session ends.
    sess=$$
    while [ "$sess" -gt 1 ]; do
      case $(ps -o comm= -p "$sess") in sshd*) break ;; esac
      sess=$(ps -o ppid= -p "$sess" | tr -d " ")
    done
    watchdog=
    if [ "$sess" -gt 1 ] && command -v systemd-run >/dev/null; then
      watchdog=$name-watch
      systemd-run --user --quiet --collect --unit "$watchdog" --setenv=PATH="$PATH" \
        "$(command -v sh)" -c "while kill -0 \$0 2>/dev/null; do sleep 5; done
          podman rm -f -t 2 \$1" "$sess" "$name" || watchdog=
    fi
    rc=0
    # shellcheck disable=SC2086 # hold and envs are lists of words
    $hold podman run -i --name "$name" --cpus "$cpus" --memory "$mem" \
      --mount "type=image,src=th-gate-data:$data_tag,dst=/orig" \
      -v th-replay-cache:/cache:U \
      -e GATE_JOBS="$cpus" -e GATE_SCRIPTS="$scripts" $envs \
      "th-gate-env:$env_tag" || rc=$?
    # A container that finished has its output still to be copied; one still
    # running lost its podman run to the dropped connection and is left to
    # the watcher.
    if [ -n "$watchdog" ] &&
      [ "$(podman inspect -f "{{.State.Running}}" "$name" 2>/dev/null)" = false ]; then
      systemctl --user stop "$watchdog" 2>/dev/null
    fi
    exit "$rc"' \
    "$name" "$runs" "$cpus" "$mem" "$env_tag" "$data_tag" "$*" "$envs" "$fans" || rc=$?
mkdir -p "$out"
remote 'podman cp "$1:/build/w/build/remote-out/." -' "$name" | tar -x -C "$out" 2>/dev/null || true
find "$out" -depth -type d -empty -delete
[ ! -d "$out" ] || echo "remote.sh: run output in $out" >&2
exit "$rc"

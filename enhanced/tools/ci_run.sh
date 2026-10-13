#!/bin/sh
# Runs the gate (enhanced/tools/check.sh, sharded: .github/workflows/gate.yml)
# on GitHub Actions for the committed HEAD of this checkout, waits for it,
# prints one line per job like check.sh does, and brings the new original
# traces back into build/replay-cache/traces so later local CACHE=only runs
# and CI runs find them. Exit 0: every job passed; 1: one failed; 2: usage or
# a missing tool.
#
# The game data is in the project's private R2 bucket (enhanced/README.md,
# "CI"); the runner fetches it, and it never comes back here: only the
# traces (numbers) and the failed jobs' text logs do.
#
# Usage:
#   enhanced/tools/ci_run.sh [--scripts "NAME ..."] [--shards N] [--cache M]
#                            [--fresh-max N] [--no-web] [--no-wait]
#     --scripts    replay scripts to run (default: all)
#     --shards     replay shards (default 6; more for many fresh originals)
#     --cache      use|only|refresh, as check.sh's CACHE (default use)
#     --fresh-max  most fresh originals one shard accepts (default 10; a cold
#                  cache needs about the scripts' count / shards)
#     --no-web     leave out the WebAssembly build and checks
#     --no-wait    start the run, print its URL and stop
#
# HEAD is pushed (force) to the branch ci/<this folder's name>: only ci/*
# branches are ever touched. Uncommitted changes are not in the run.
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"

scripts= shards=6 cache=use fresh_max=10 web=true wait=1
while [ $# -gt 0 ]; do
  case $1 in
    --scripts) scripts=${2:?--scripts needs a list}; shift 2 ;;
    --shards) shards=${2:?--shards needs a number}; shift 2 ;;
    --cache) cache=${2:?--cache needs use, only or refresh}; shift 2 ;;
    --fresh-max) fresh_max=${2:?--fresh-max needs a number}; shift 2 ;;
    --no-web) web=false; shift ;;
    --no-wait) wait=0; shift ;;
    *) echo "ci_run.sh: unknown argument $1" >&2; exit 2 ;;
  esac
done
for tool in gh git; do
  command -v "$tool" >/dev/null 2>&1 || { echo "ci_run.sh: $tool is not on PATH" >&2; exit 2; }
done

name=$(basename "$root" | tr -c 'A-Za-z0-9._\n-' '-')
branch=ci/$name
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
  echo "note: uncommitted changes are not in the run (it checks HEAD $(git rev-parse --short HEAD))"
fi
sha=$(git rev-parse HEAD)
git push --force --quiet origin "HEAD:refs/heads/$branch"

started=$(date -u +%Y-%m-%dT%H:%M:%SZ)
gh workflow run gate.yml --ref "$branch" -f ref="$sha" -f shards="$shards" -f cache="$cache" \
  -f fresh_max="$fresh_max" -f web="$web" -f scripts="$scripts" -f push_nix=false ||
  { echo "ci_run.sh: gate.yml could not be dispatched (is it on master yet?)" >&2; exit 1; }
id=
for _ in 1 2 3 4 5 6 7 8 9 10; do
  sleep 5
  id=$(gh run list --workflow gate.yml --branch "$branch" --event workflow_dispatch --limit 5 \
    --json databaseId,createdAt -q "[.[] | select(.createdAt >= \"$started\")] | last | .databaseId // empty")
  [ -z "$id" ] || break
done
[ -n "$id" ] || { echo "ci_run.sh: the run did not start" >&2; exit 1; }
url=$(gh run view "$id" --json url -q .url)
echo "run $id: $url"
[ "$wait" = 1 ] || exit 0

gh run watch "$id" --interval 30 >/dev/null 2>&1 || true
gh run view "$id" --json jobs -q '.jobs[] | [.conclusion, .name, .startedAt, .completedAt] | @tsv' |
  while IFS="$(printf '\t')" read -r result job from to; do
    secs=$(($(date -d "$to" +%s) - $(date -d "$from" +%s)))
    case $result in success) word=pass ;; skipped) word=skip ;; *) word=FAIL ;; esac
    echo "$word $job ($secs s)"
  done

out=$root/build/ci-run-$id
mkdir -p "$out" "$root/build/replay-cache/traces"
gh run download "$id" --pattern 'traces-*' --dir "$out/traces" 2>/dev/null || true
count=0
for f in "$out"/traces/*/*.zst; do
  [ -e "$f" ] || continue
  [ -e "$root/build/replay-cache/traces/$(basename "$f")" ] ||
    { cp "$f" "$root/build/replay-cache/traces/" && count=$((count + 1)); }
done
echo "$count new original traces added to build/replay-cache/traces"

conclusion=$(gh run view "$id" --json conclusion -q .conclusion)
if [ "$conclusion" != success ]; then
  gh run download "$id" --pattern '*-logs*' --dir "$out/logs" 2>/dev/null || true
  echo "logs of the failed jobs: $out/logs"
  exit 1
fi
rm -rf "$out"
echo "gate passed on $sha"

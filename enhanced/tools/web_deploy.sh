#!/bin/sh
# Builds the browser build and deploys it to Vercel as static files.
#
#   enhanced/tools/web_deploy.sh [--dry-run]
#
# The build (build/web, as README.md's "Web build" says, configured when
# missing) is brought up to date, then assembled into build/web/deploy:
# index.html, index.<hash>.js and index.<hash>.wasm (hashed names, so they
# cache for good) and web/vercel.json (COOP/COEP headers, caching). The folder
# is what is uploaded; there is no build step on Vercel's side. It holds no
# game data: the player picks their own installer or folder. It is deployed to
# production (`vercel deploy --prod`).
#
# --dry-run: assembles the folder, prints the command, talks to no server.
#
# Needs `vercel login` done once (the CLI is vercel@$VERCEL_VERSION through
# npx: it is not in nixpkgs). TH_VERCEL_PROJECT names the Vercel project
# (default theme-hospital-enhanced).
#
# In CI there is no login: with VERCEL_TOKEN, VERCEL_ORG_ID and
# VERCEL_PROJECT_ID in the environment (the CLI reads all three itself) the
# deploy goes to the project those ids name, which must exist, and
# TH_VERCEL_PROJECT is not used.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"

if [ -z "${TH_WEB_DEPLOY_INNER:-}" ]; then
  exec nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_WEB_DEPLOY_INNER=1 \
    nix develop path:./nix#web -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$0" "$@"
fi

dry_run=0
for arg in "$@"; do
  case $arg in
    --dry-run) dry_run=1 ;;
    *)
      echo "usage: web_deploy.sh [--dry-run]" >&2
      exit 2
      ;;
  esac
done

VERCEL_VERSION=63.1.2
project=${TH_VERCEL_PROJECT:-theme-hospital-enhanced}
vercel="npx --yes vercel@$VERCEL_VERSION"

[ -f build/web/CMakeCache.txt ] ||
  emcmake cmake -S enhanced -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web --target th_enhanced

out=build/web/deploy
node enhanced/tools/web_deploy.js build/web/dist "$out"

# `deploy --project` needs the project to exist.
ensure_project()
{
  $vercel project inspect "$project" >/dev/null 2>&1 || $vercel project create "$project"
}

if [ -n "${VERCEL_ORG_ID:-}" ] && [ -n "${VERCEL_PROJECT_ID:-}" ]; then
  # With both ids the CLI takes the project from them (an explicit
  # --project would have it look for a link or a login instead).
  if [ "$dry_run" = 1 ]; then
    echo "would run: $vercel deploy $out --prod --yes (project $VERCEL_PROJECT_ID of $VERCEL_ORG_ID)"
    exit 0
  fi
  [ -n "${VERCEL_TOKEN:-}" ] || {
    echo "web_deploy: VERCEL_ORG_ID and VERCEL_PROJECT_ID are set but VERCEL_TOKEN is not" >&2
    exit 2
  }
  exec $vercel deploy "$out" --prod --yes
fi
if [ "$dry_run" = 1 ]; then
  echo "would run: $vercel deploy $out --project $project --prod --yes"
  exit 0
fi
ensure_project
exec $vercel deploy "$out" --project "$project" --prod --yes

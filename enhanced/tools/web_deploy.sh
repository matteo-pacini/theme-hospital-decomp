#!/bin/sh
# Builds the browser build and deploys it to Vercel as static files.
#
#   enhanced/tools/web_deploy.sh [--with-data | --preview] [--dry-run]
#
# The build (build/web, as README.md's "Web build" says, configured when
# missing) is brought up to date, then assembled into a folder: index.html,
# index.<hash>.js and index.<hash>.wasm (hashed names, so they cache for good)
# and web/vercel.json (COOP/COEP headers, caching). The folder is what is
# uploaded; there is no build step on Vercel's side.
#
# Default: build/web/deploy, no game data, deployed to production
# (`vercel deploy --prod`). The player picks their own installer or folder.
#
# --with-data (for testing): build/web/deploy-data also holds, in data/, the
# install's files from orig/ (the manifest of core/data/gog_manifest.inc and
# nothing else) and data/files.txt, which the page loads instead of showing
# the picker. This is only ever a PREVIEW deployment, never promoted to a
# domain. Then, without credentials, every URL the deploy printed must answer
# 401 or redirect to Vercel's sign-in (vercel.com/sso-api), for the page and
# for data/files.txt, and https://<project>.vercel.app/data/files.txt must
# answer 404. Otherwise the deployment is removed again and the script fails.
# The copy in build/web/deploy-data is the only copy of the data; it is never
# written into the repository.
#
# --preview: the same preview deployment and checks without the data (the
# picker), to try the checks with nothing at stake.
#
# --dry-run: assembles the folder, prints the commands, talks to no server.
#
# Needs `vercel login` done once (the CLI is vercel@$VERCEL_VERSION through
# npx: it is not in nixpkgs). TH_VERCEL_PROJECT names the Vercel project
# (default theme-hospital-enhanced).
#
# In CI there is no login: with VERCEL_TOKEN, VERCEL_ORG_ID and
# VERCEL_PROJECT_ID in the environment (the CLI reads all three itself) the
# production deploy goes to the project those ids name, which must exist, and
# TH_VERCEL_PROJECT is not used. Without the two ids, or with --preview or
# --with-data, the script behaves as described above.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"

if [ -z "${TH_WEB_DEPLOY_INNER:-}" ]; then
  exec nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH TH_WEB_DEPLOY_INNER=1 \
    nix develop path:./nix#web -c sh -c 'unset LD_LIBRARY_PATH; exec "$0" "$@"' "$0" "$@"
fi

with_data=0
preview=0
dry_run=0
for arg in "$@"; do
  case $arg in
    --with-data) with_data=1 preview=1 ;;
    --preview) preview=1 ;;
    --dry-run) dry_run=1 ;;
    *)
      echo "usage: web_deploy.sh [--with-data | --preview] [--dry-run]" >&2
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

if [ "$with_data" = 1 ]; then
  out=build/web/deploy-data
  [ -d orig ] || {
    echo "web_deploy: orig/ not found" >&2
    exit 2
  }
  node enhanced/tools/web_deploy.js build/web/dist "$out" enhanced/src/core/data/gog_manifest.inc "$root/orig"
else
  out=build/web/deploy
  node enhanced/tools/web_deploy.js build/web/dist "$out" enhanced/src/core/data/gog_manifest.inc
fi

# `deploy --project` needs the project to exist.
ensure_project()
{
  $vercel project inspect "$project" >/dev/null 2>&1 || $vercel project create "$project"
}

if [ "$preview" = 0 ]; then
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
fi

# Vercel made a project's first deployment production, on the public domain
# that Vercel Authentication does not cover, even with --target preview. So a
# production target fails, every address the output names is checked as well
# as the public domain, and anything unexpected removes the deployment, a
# deploy that failed half-way included.
deploy="$vercel deploy $out --project $project --target preview --yes"
public=https://$project.vercel.app
if [ "$dry_run" = 1 ]; then
  echo "would run: $deploy"
  echo "then, without credentials: every printed URL (and its data/files.txt) must"
  echo "answer 401 or redirect to Vercel's sign-in, $public/data/files.txt must"
  echo "answer 404; otherwise: $vercel remove URL --yes"
  exit 0
fi

ensure_project
log=$out.log
deployed=0
$deploy >"$log" 2>&1 || deployed=1
cat "$log" >&2
urls=$(grep -o 'https://[A-Za-z0-9.-]*\.vercel\.app' "$log" | grep -v -x "$public" | sort -u || true)

# A 401, or a redirect to Vercel's sign-in.
signin()
{
  answer=$(curl -s -o /dev/null -w '%{http_code} %{redirect_url}' --max-time 60 "$1") || return 1
  case $answer in
    401\ *) return 0 ;;
    30[1278]\ https://vercel.com/sso-api* | 30[1278]\ https://vercel.com/login*) return 0 ;;
  esac
  echo "web_deploy: $1 answers $answer" >&2
  return 1
}

ok=$((1 - deployed))
[ -n "$urls" ] || ok=0
if grep -q -e '"target": *"production"' -e 'Aliased' "$log"; then
  echo "web_deploy: Vercel made it a production deployment" >&2
  ok=0
fi
for url in $urls; do
  signin "$url/" && signin "$url/data/files.txt" || ok=0
done
answer=$(curl -s -o /dev/null -w '%{http_code}' --max-time 60 "$public/data/files.txt" || true)
[ "$answer" = 404 ] || {
  echo "web_deploy: $public/data/files.txt answers $answer" >&2
  ok=0
}

if [ "$ok" = 1 ]; then
  echo "$urls"
  exit 0
fi
echo "web_deploy: not behind Vercel Authentication: removing the deployment" >&2
for url in $urls; do
  $vercel remove "$url" --yes || echo "web_deploy: REMOVE IT BY HAND: vercel remove $url" >&2
done
[ -n "$urls" ] ||
  echo "web_deploy: no deployment URL found: check \`$vercel ls $project\` and remove it by hand" >&2
exit 1

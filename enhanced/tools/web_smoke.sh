#!/bin/sh
# Smoke test of the browser build: headless Chromium (playwright from
# nixpkgs, in the `web` dev shell) loads build/web/dist from a local static
# server that sends the COOP/COEP headers, installs the game from the local
# orig/ folder through the page's picker (this run only: orig/ is never
# copied into dist/ or the repository), reloads to see the install kept,
# starts level 1, writes a debug report and quits. It fails on any console
# error or uncaught exception, and on a step that does not happen.
#
# Usage, from the repository root:
#   enhanced/tools/web_smoke.sh [DIST]
# DIST is the build's output (default build/web/dist; build it with
# `emcmake cmake -S enhanced -B build/web -DCMAKE_BUILD_TYPE=Release` and
# `cmake --build build/web --target th_enhanced` in the web dev shell).
# Output goes to build/web-smoke/: screenshots (setup.png, level1.png), the
# downloaded report tar and the log. Exit 0 on success.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
dist=${1:-$root/build/web/dist}
if [ ! -f "$dist/index.html" ]; then
  echo "web_smoke: $dist/index.html not found: build the web target first" >&2
  exit 2
fi
out=$root/build/web-smoke
rm -rf "$out"
mkdir -p "$out"
cd "$root"
exec nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH \
  nix develop path:./nix#web -c sh -c \
  'unset LD_LIBRARY_PATH; exec node "$0" "$@"' \
  "$root/enhanced/tools/web_smoke.js" "$dist" "$root/orig" "$out"

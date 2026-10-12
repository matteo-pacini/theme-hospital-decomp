#!/bin/sh
# Remakes the README screenshots in docs/images/ from the *.txt drive scripts
# beside this file, headless, at 1280x720.
#
#   enhanced/tools/readme_shots/shots.sh [NAME ...]
#
# NAME is one of title, hospital, town-map, zoom-original, zoom-upscaled
# (default: all). Run it from a plain terminal after building build/enhanced
# (enhanced/README.md); the game data comes from orig/. The scripts' headers
# are the drive tests' (`# Size`, `# NoLevel`) plus `# SeedReplay LEVEL SCRIPT
# TURNS`, which th_replay plays to that turn into the save slot that Alt+L
# loads. The upscaled shot is the zoom view with `--upscale xbrz:4`, built
# into a scratch user folder so that the player's own is left alone.
#
# The images show the game's art: they are the only game-derived files the
# repository holds (AGENTS.md).
set -eu
root=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$root"
build=${BUILD:-build/enhanced}
here=enhanced/tools/readme_shots
work=${SHOTS_DIR:-build/readme-shots}
[ $# -gt 0 ] || set -- title hospital town-map zoom-original zoom-upscaled

# shoot NAME SCRIPT UPSCALE SHOT: runs drive script SCRIPT with --upscale
# UPSCALE and keeps the screenshot SHOT as docs/images/NAME.png.
shoot() {
  name=$1
  script=$here/$2.txt
  upscale=$3
  shot=$4
  out=$work/$name
  rm -rf "$out"
  mkdir -p "$out/save"
  level_arg="--level 1"
  grep -q '^# NoLevel' "$script" && level_arg=""
  seed=$(sed -n 's/^# SeedReplay \(.*\)/\1/p' "$script" | head -1)
  if [ -n "$seed" ]; then
    # shellcheck disable=SC2086
    set -- $seed
    env -u LD_LIBRARY_PATH nice -n 19 ionice -c 3 "$build/th_replay" \
      --level "$1" --script "$2" --turns "$3" --out /dev/null \
      --save "$out/save/HORZA.h0" >/dev/null 2>&1
    level_arg="--level $1"
  fi
  # shellcheck disable=SC2086
  env -u LD_LIBRARY_PATH XDG_DATA_HOME="$root/$work/xdg" nice -n 19 ionice -c 3 \
    "$build/th_enhanced" $level_arg --size 1280x720 --upscale "$upscale" \
    --drive "$script" --out "$out" >"$out.log" 2>&1
  cp "$out/$shot.png" "docs/images/$name.png"
}

mkdir -p docs/images "$work"
for name in "$@"; do
  case $name in
    title) shoot title title off title ;;
    hospital) shoot hospital hospital off hospital ;;
    town-map) shoot town-map town_map off town_map ;;
    zoom-original) shoot zoom-original zoom off zoom ;;
    zoom-upscaled) shoot zoom-upscaled zoom xbrz:4 zoom ;;
    *) echo "unknown shot: $name" >&2; exit 2 ;;
  esac
done

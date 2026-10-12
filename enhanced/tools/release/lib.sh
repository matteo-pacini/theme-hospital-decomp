# shellcheck shell=sh disable=SC2034
# Sourced by the release scripts: `root` is the repository root, `die` stops
# with a message, `release_name` builds an artifact's base name.

root=$(cd "$(dirname "$0")/../../.." && pwd)

die()
{
  echo "release: $*" >&2
  exit 1
}

# release_name TAG PLATFORM: theme-hospital-enhanced-<tag>-<platform>, the
# artifact's name without its extension; PLATFORM is linux-x86_64,
# linux-arm64, windows-x86_64, windows-arm64 or macos-arm64.
release_name()
{
  echo "theme-hospital-enhanced-$1-$2"
}

# verify_sha256 FILE HASH: fails unless FILE's SHA-256 is HASH.
verify_sha256()
{
  echo "$2  $1" | sha256sum -c - >/dev/null 2>&1 || die "$1: SHA-256 is not $2"
}

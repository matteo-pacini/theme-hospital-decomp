#!/bin/sh
# Downloads the game files the CI gate needs (the archive data_upload.sh made)
# from the project's private R2 bucket and unpacks them into orig/, then
# checks every file against the archive's SHA256SUMS and the install manifest
# (core/data/gog_manifest.inc). Prints counts and the names of files that
# fail, never their contents.
#
# The files must stay out of caches, artifacts and logs: unpack into the
# workspace only.
#
# Usage:
#   enhanced/tools/ci/data_fetch.sh [--from SRC] [DEST]
#     --from SRC  s3://BUCKET/KEY, or a file path (default:
#                 s3://theme-hospital-enhanced/theme-hospital-data.tar.zst)
#     DEST        the folder to unpack into (default orig/)
#
# Environment (for an s3:// source): R2_ACCESS_KEY_ID and
# R2_SECRET_ACCESS_KEY (read-only is enough); R2_ENDPOINT, R2_BUCKET and
# R2_DATA_KEY override the defaults. Needs tar, zstd and, for R2, the AWS CLI.
set -eu
. "$(dirname "$0")/lib.sh"

src=
dest=$root/orig
while [ $# -gt 0 ]; do
  case $1 in
    --from) src=${2:?--from needs a source}; shift 2 ;;
    -*) die "unknown argument $1" ;;
    *) dest=$1; shift ;;
  esac
done
need tar zstd sha256sum
[ -n "$src" ] || src=$(default_object)
case $src in s3://*) need aws ;; *) [ -f "$src" ] || die "$src is not a file" ;; esac

mkdir -p "$dest" "$root/build"
work=$(mktemp -d "$root/build/ci-data.XXXXXX")
trap 'rm -rf "$work"' EXIT

case $src in
  s3://*) r2 s3 cp --only-show-errors "$src" - ;;
  *) cat "$src" ;;
esac | zstd -d -q | tar -x --no-same-owner -C "$dest"
[ -f "$dest/$sums_name" ] || die "the archive has no $sums_name"
mv "$dest/$sums_name" "$work/$sums_name"

(cd "$dest" && sha256sum --quiet -c "$work/$sums_name") || die "files differ from the archive's $sums_name"
for f in $data_files; do
  [ -f "$dest/$f" ] || die "$f is missing from the archive"
done
manifest_sums >"$work/manifest.sums"
count=$(wc -l <"$work/manifest.sums")
[ "$count" -ge 300 ] || die "the manifest was read wrongly"
(cd "$dest" && sha256sum --quiet -c "$work/manifest.sums") || die "files differ from the install manifest"
echo "unpacked into $dest: $(wc -l <"$work/$sums_name") files checked, $count of them against the install manifest"

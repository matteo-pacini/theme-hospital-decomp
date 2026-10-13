#!/bin/sh
# Packs the game files the CI gate needs from orig/ into one zstd-compressed
# tar and uploads it to the project's private R2 bucket. Run by hand on the
# machine that has the game; CI only downloads it (data_fetch.sh).
#
# The archive holds the folders ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM
# SOUND and the files HOSPITAL.CFG, MODEM.INI, DOS4GW.EXE (all that the port's
# tests and the original's DOSBox run read), and a SHA256SUMS file. Files are
# stored byte for byte: the replay trace key hashes the three loose files. It
# is not encrypted: the bucket is private and the read-only token is a secret
# of the `ci` environment. Every file of the install manifest
# (core/data/gog_manifest.inc) is checked before packing.
#
# Usage:
#   enhanced/tools/ci/data_upload.sh [--orig DIR] [--to DEST]
#     --orig DIR  the game folder (default orig/)
#     --to DEST   s3://BUCKET/KEY, or a file path to write instead of
#                 uploading (default: s3://theme-hospital-enhanced/
#                 theme-hospital-data.tar.zst)
#
# Environment (for an s3:// destination): credentials of a token that can
# write the bucket, as AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY (or
# R2_ACCESS_KEY_ID and R2_SECRET_ACCESS_KEY); R2_ENDPOINT, R2_BUCKET and
# R2_DATA_KEY override the defaults. Needs tar, zstd and, for R2, the AWS CLI.
set -eu
. "$(dirname "$0")/lib.sh"

orig=$root/orig
dest=
while [ $# -gt 0 ]; do
  case $1 in
    --orig) orig=${2:?--orig needs a folder}; shift 2 ;;
    --to) dest=${2:?--to needs a destination}; shift 2 ;;
    *) die "unknown argument $1" ;;
  esac
done
[ -d "$orig" ] || die "$orig is not a folder"
need tar zstd sha256sum find
[ -n "$dest" ] || dest=$(default_object)
case $dest in s3://*) need aws ;; esac

mkdir -p "$root/build"
work=$(mktemp -d "$root/build/ci-data.XXXXXX")
trap 'rm -rf "$work"' EXIT

for f in $data_files; do
  [ -f "$orig/$f" ] || die "$orig/$f is missing"
done
for d in $data_dirs; do
  [ -d "$orig/$d" ] || die "$orig/$d is missing"
done
(
  cd "$orig"
  for f in $data_files; do echo "$f"; done
  # shellcheck disable=SC2086
  find $data_dirs -type f
) | LC_ALL=C sort >"$work/list"
# shellcheck disable=SC2086
[ -z "$(cd "$orig" && find $data_dirs -not -type f -not -type d)" ] || die "$orig has links or special files"
(cd "$orig" && xargs -d '\n' sha256sum <"$work/list") >"$work/$sums_name"

manifest_sums >"$work/manifest.sums"
[ "$(wc -l <"$work/manifest.sums")" -ge 300 ] || die "the manifest was read wrongly"
(cd "$orig" && sha256sum --quiet -c "$work/manifest.sums") || die "files differ from the install manifest"

# Fixed owner, order and times, so the same files give the same archive.
tar --owner=0 --group=0 --numeric-owner --mtime=@0 -cf - -C "$work" "$sums_name" -C "$orig" -T "$work/list" |
  zstd -T0 -10 -q -o "$work/data.tar.zst"
bytes=$(wc -c <"$work/data.tar.zst")

case $dest in
  s3://*) r2 s3 cp --only-show-errors "$work/data.tar.zst" "$dest" ;;
  *) cp "$work/data.tar.zst" "$dest" ;;
esac
echo "packed $(wc -l <"$work/list") files, $bytes bytes ($((bytes / 1024 / 1024)) MiB), to $dest"
echo "archive sha256: $(sha256sum "$work/data.tar.zst" | cut -d' ' -f1)"
echo "per-file sha256 (SHA256SUMS):"
cat "$work/$sums_name"

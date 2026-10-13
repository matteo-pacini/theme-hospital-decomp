# shellcheck shell=sh disable=SC2034
# Sourced by data_upload.sh and data_fetch.sh: `root` is the repository root,
# `die` stops with a message, `r2` runs the AWS CLI against the project's R2
# bucket, `manifest_files` lists the install manifest.

root=$(cd "$(dirname "$0")/../../.." && pwd)
manifest=$root/enhanced/src/core/data/gog_manifest.inc

# What the archive holds, relative to orig/: everything the port's tests and
# the original's DOSBox run (orig_trace.sh) read. HOSPITAL.CFG, MODEM.INI and
# DOS4GW.EXE are part of the original's trace key, so they must be byte
# identical to the ones the cached traces were made with.
data_dirs="ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM SOUND"
data_files="HOSPITAL.CFG MODEM.INI DOS4GW.EXE"

# The R2 endpoint is not a secret, the credentials are.
default_endpoint=https://d08c5feaf8c5f2a5bc3447c89c4df52f.r2.cloudflarestorage.com
default_bucket=theme-hospital-enhanced

# The archive's own name in the bucket; R2_DATA_KEY overrides it.
object_key=${R2_DATA_KEY:-theme-hospital-data.tar.zst}
# Inside the archive: SHA-256 of every file, in `sha256sum` format.
sums_name=SHA256SUMS

die()
{
  echo "$(basename "$0"): $*" >&2
  exit 1
}

need()
{
  for tool in "$@"; do
    command -v "$tool" >/dev/null 2>&1 || die "$tool is not on PATH"
  done
}

# manifest_files: prints "PATH SIZE SHA256" for every file of the install
# manifest (core/data/gog_manifest.inc).
manifest_files()
{
  sed -n 's/^[[:space:]]*{"\([^"]*\)", \([0-9][0-9]*\), "\([0-9a-f]\{64\}\)".*/\1 \2 \3/p' "$manifest"
}

# manifest_sums: the manifest in `sha256sum` format.
manifest_sums()
{
  manifest_files | while read -r name _ sha; do
    echo "$sha  $name"
  done
}

# default_object: the s3:// URL of the archive.
default_object()
{
  echo "s3://${R2_BUCKET:-$default_bucket}/$object_key"
}

# r2 ARG ...: the AWS CLI against the R2 endpoint. Credentials: R2_ACCESS_KEY_ID
# and R2_SECRET_ACCESS_KEY (the `ci` environment), else the shell's
# AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY. R2 has one region, "auto"; the
# checksum settings keep newer CLI releases from sending headers R2 does not take.
r2()
{
  key=${R2_ACCESS_KEY_ID:-${AWS_ACCESS_KEY_ID:-}}
  secret=${R2_SECRET_ACCESS_KEY:-${AWS_SECRET_ACCESS_KEY:-}}
  [ -n "$key" ] && [ -n "$secret" ] || die "no credentials: set R2_ACCESS_KEY_ID and R2_SECRET_ACCESS_KEY (or AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY)"
  AWS_ACCESS_KEY_ID=$key AWS_SECRET_ACCESS_KEY=$secret \
    AWS_DEFAULT_REGION=auto AWS_EC2_METADATA_DISABLED=true \
    AWS_REQUEST_CHECKSUM_CALCULATION=when_required AWS_RESPONSE_CHECKSUM_VALIDATION=when_required \
    aws --endpoint-url "${R2_ENDPOINT:-$default_endpoint}" "$@"
}

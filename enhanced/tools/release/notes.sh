#!/bin/sh
# Writes the release notes of TAG to OUTFILE: the lines of its section of
# enhanced/CHANGELOG.md, under the heading `## vX.Y.Z` or
# `## vX.Y.Z - YYYY-MM-DD`, up to the next `## ` heading.
#
#   notes.sh TAG OUTFILE [--require-date]
#
# Fails when the section is missing, repeated or empty. A heading without a
# date fails with --require-date and only warns without it.
set -eu
# shellcheck source=lib.sh
. "$(dirname "$0")/lib.sh"

[ $# -ge 2 ] || die "usage: notes.sh TAG OUTFILE [--require-date]"
tag=$1
out=$2
require_date=0
[ "${3:-}" != --require-date ] || require_date=1
changelog=$root/enhanced/CHANGELOG.md

headings=$(awk -v tag="$tag" '$0 == "## " tag || index($0, "## " tag " ") == 1' "$changelog")
[ -n "$headings" ] || die "enhanced/CHANGELOG.md has no section '## $tag'"
[ "$(printf '%s\n' "$headings" | wc -l)" = 1 ] || die "enhanced/CHANGELOG.md has several sections for $tag"
case $headings in
  "## $tag") dated=0 ;;
  "## $tag - "[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]) dated=1 ;;
  *) die "heading '$headings' is neither '## $tag' nor '## $tag - YYYY-MM-DD'" ;;
esac
if [ "$dated" = 0 ]; then
  [ "$require_date" = 0 ] || die "the heading of $tag has no date: '## $tag - YYYY-MM-DD'"
  echo "::warning::the changelog heading of $tag has no date"
fi

awk -v heading="$headings" '
  /^## / { if (on) exit; on = ($0 == heading); next }
  on { print }
' "$changelog" | sed -e '/./,$!d' | sed -e ':a' -e '/^\n*$/{$d;N;ba' -e '}' >"$out"
[ -s "$out" ] || die "the section of $tag in enhanced/CHANGELOG.md is empty"
echo "notes of $tag: $(wc -l <"$out") lines in $out"

#!/bin/sh
# Prints the trace cache key for one original-side replay: the instrumented
# HOSPITAL.EXE's cache key (exe_cache_key.sh) and a hash of everything else
# that changes the trace it writes — the script file's bytes, the last turn,
# the level, the left-out calls, the random-draw and drawing options,
# orig_trace.sh (the DOSBox configuration and the run folder it builds, read
# without its comment-only and blank lines: `run_text` below), the original's
# HOSPITAL.CFG, MODEM.INI and DOS4GW.EXE, and which dosbox-x runs it.
# cached_orig_trace.sh names its entries with it. The game data folders
# (orig/DATA, LEVELS, ...) are not hashed: they are the installed game.
#
# Usage (from the repository root):
#   trace_key.sh EXE_KEY TURNS LEVEL SKIP RAND DRAW [SCRIPT]
set -eu

exe=$1 turns=$2 level=$3 skip=$4 rand=$5 draw=$6 script=${7:-}
here=$(cd "$(dirname "$0")" && pwd)

# Hashes a file's bytes, not its name, so the key does not depend on the path.
content()
{
  if [ -f "$1" ]; then sha256sum <"$1" | cut -c1-64; else echo missing; fi
}

# Prints a shell script as what it does: without comment-only lines (the
# shebang stays) and blank lines, which cannot change a command. Everything
# that could is kept as written, and so is every line after the first thing
# this does not understand (a miss, never a false hit):
#   - a line that follows a line ending in a backslash (a continuation);
#   - a line inside a quoted string that spans lines;
#   - heredoc bodies, which are the DOSBox configuration, comment-looking
#     lines included. `<<WORD`, `<<-WORD`, `<<\WORD`, `<<'WORD'` and
#     `<<"WORD"` are understood; any other `<<`, or two on a line, is not.
# Trailing blanks and echoed messages are not touched.
run_text()
{
  awk '
    function scan(s,    i, c, n) {
      # Updates sq/dq (inside a quote) over the line; sets cont when it ends
      # in an odd run of backslashes outside single quotes.
      n = length(s)
      cont = 0
      for (i = 1; i <= n; i++) {
        c = substr(s, i, 1)
        if (sq) { if (c == "\047") sq = 0; continue }
        if (c == "\\") {
          if (i == n) cont = 1
          i++
          continue
        }
        if (dq) { if (c == "\"") dq = 0; continue }
        if (c == "\047") { sq = 1; continue }
        if (c == "\"") { dq = 1; continue }
        if (c == "#" && (i == 1 || substr(s, i - 1, 1) ~ /[ \t]/)) break
      }
    }
    function heredoc(s,    rest, d, k) {
      k = index(s, "<<")
      if (!k) return 0
      rest = substr(s, k + 2)
      if (index(rest, "<<") || substr(rest, 1, 1) == "<") { verbatim = 1; return 0 }
      indash = 0
      if (substr(rest, 1, 1) == "-") { indash = 1; rest = substr(rest, 2) }
      sub(/^[ \t]*/, "", rest)
      if (match(rest, /^\\?[A-Za-z_][A-Za-z_0-9]*/)) {
        d = substr(rest, RSTART, RLENGTH); sub(/^\\/, "", d)
      } else if (match(rest, /^\047[A-Za-z_][A-Za-z_0-9]*\047/) || match(rest, /^"[A-Za-z_][A-Za-z_0-9]*"/)) {
        d = substr(rest, RSTART + 1, RLENGTH - 2)
      } else { verbatim = 1; return 0 }
      delim = d
      return 1
    }
    verbatim { print; next }
    inhere {
      print
      line = $0
      if (indash) sub(/^\t+/, "", line)
      if (line == delim) inhere = 0
      next
    }
    prevcont || sq || dq {
      print
      if (index($0, "<<")) verbatim = 1
      scan($0)
      prevcont = cont
      next
    }
    /^[ \t]*$/ { next }
    /^[ \t]*#/ && NR > 1 { next }
    {
      print
      scan($0)
      prevcont = cont
      if (index($0, "<<") && !sq && !dq) inhere = heredoc($0)
    }
  ' "$1"
}

dosbox=$(command -v dosbox-x || true)
[ -z "$dosbox" ] || dosbox=$(readlink -f "$dosbox")

rest=$(
  printf 'turns=%s\nlevel=%s\nskip=%s\nrand=%s\ndraw=%s\n' "$turns" "$level" "$skip" "$rand" "$draw"
  if [ -n "$script" ]; then
    printf 'script=%s\n' "$(content "$script")"
  else
    printf 'script=none\n'
  fi
  printf 'orig_trace=%s\n' "$(run_text "$here/orig_trace.sh" | sha256sum | cut -c1-64)"
  for f in HOSPITAL.CFG MODEM.INI DOS4GW.EXE; do
    printf '%s=%s\n' "$f" "$(content "orig/$f")"
  done
  printf 'dosbox=%s\n' "$dosbox"
)
rest=$(printf '%s' "$rest" | sha256sum | cut -c1-16)
printf '%s-%s\n' "$exe" "$rest"

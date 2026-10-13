#!/bin/sh
# Prints the cache key of the instrumented HOSPITAL.EXE for a tree: a hash of
# everything the executable is built from (the decompilation's sources,
# objects and tools, configure.py and instrument.py). orig_trace.sh names its
# executable cache directory with it and the trace cache names its entries
# with it, so both share one definition.
#
# The decompilation's objects enter as their ELF `.o` files only. The OMF
# `.obj` files carry the source path and the sources' timestamps, so they
# differ between two checkouts that build the same executable; a key over them
# would never match across machines (CI). Paths are sorted in the C locale,
# so the key does not depend on the caller's locale.
#
# instrument.py goes in as its parsed syntax tree without docstrings
# (`normalized` below), so editing its comments, docstrings, blank lines or
# layout keeps the key, while any change to its code, its EVENTS tables or the
# C++ text they carry changes it. Docstring removal is safe only while
# instrument.py reads no `__doc__` but the module's, for its usage message
# (a failed call that writes no trace); a new use of docstrings as data needs
# this removal dropped. Without python3 on PATH the raw text is
# hashed instead: a key that differs from the normal one, never a false match.
#
# Usage: exe_cache_key.sh ROOT INSTRUMENT_PY
#   ROOT           the repository root to hash (run from anywhere)
#   INSTRUMENT_PY  path to tools/replay/instrument.py
set -eu

root=$1 instrument=$2

# Prints the file as `ast.dump` text (no positions, comments or formatting;
# parsed as bytes, so a coding comment decodes the source as it would run)
# with every docstring removed; other strings, such as the C++ it inserts,
# stay. The program text goes into the hash too, so editing it changes the key.
normalized()
{
  prog='import ast, sys
tree = ast.parse(sys.stdin.buffer.read())
for node in ast.walk(tree):
    body = getattr(node, "body", None)
    if (isinstance(node, (ast.Module, ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef))
            and body and isinstance(body[0], ast.Expr)
            and isinstance(body[0].value, ast.Constant) and isinstance(body[0].value.value, str)):
        del body[0]
        if not body:
            body.append(ast.Pass())
print(ascii(ast.dump(tree)))'
  if command -v python3 >/dev/null 2>&1; then
    printf 'ast\n%s\n' "$prog"
    python3 -c "$prog" <"$1"
  else
    printf 'raw\n'
    cat "$1"
  fi
}

# Only paths relative to ROOT and contents go into the hash, so a worktree
# (whose build/base is a link to the main tree's) gets the main tree's key and
# shares its caches: -H follows the build/base link, and instrument.py is
# hashed from stdin, without its absolute path.
cd "$root"
# Computed first: a failing parse stops the script instead of hashing nothing.
norm=$(normalized "$instrument")
{
  find -H src include tools config build/base -type f ! -path '*/__pycache__/*' ! -name '*.obj' -print0 | LC_ALL=C sort -z | xargs -0 sha256sum
  sha256sum configure.py
  printf '%s\n' "$norm" | sha256sum
} | sha256sum | cut -c1-16

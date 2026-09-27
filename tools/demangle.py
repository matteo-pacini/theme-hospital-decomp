"""Demangle Watcom C++ names with owdemangle (from the flake devShell)."""

import subprocess


def demangle(names):
    """{mangled: declaration} for the given names, with `__near` dropped."""
    names = list(dict.fromkeys(names))
    if not names:
        return {}
    out = subprocess.run(
        ["owdemangle"], input="\n".join(names) + "\n", capture_output=True, text=True, check=True
    ).stdout.splitlines()
    return {n: d.replace(" __near", "").replace("__near ", "") for n, d in zip(names, out)}

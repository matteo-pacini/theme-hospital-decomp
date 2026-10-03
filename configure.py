#!/usr/bin/env python3
"""Generate build.ninja and objdiff.json from config/.

Run inside the devShell (`nix develop path:./nix`), then `ninja` to build every object
and `ninja report` for build/report.json.

The matching targets (build/target, `all`, `report`) are split from
orig/HOSPITAL.EXE; without it, or with a different EXE, they fail with a
message when built, and `ninja` builds only the objects compiled from source,
which are all the link target depends on.
"""

import hashlib
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "tools"))
from project import EXE, EXE_SHA1, ROOT, Project  # noqa: E402
import standin  # noqa: E402
from cc import compiler_for, library_path  # noqa: E402

CATEGORIES = [
    {"id": "game", "name": "Game"},
    {"id": "bflib", "name": "Bullfrog libraries"},
    {"id": "runtime", "name": "Runtime and third-party"},
]


def exe_problem():
    """Why orig/HOSPITAL.EXE cannot be split for the matching targets, or None."""
    if not os.path.exists(EXE):
        return f"missing {os.path.relpath(EXE, ROOT)}: extract the GOG release into orig/ (see README)"
    digest = hashlib.sha1(open(EXE, "rb").read()).hexdigest()
    if digest != EXE_SHA1:
        return f"{os.path.relpath(EXE, ROOT)}: sha1 {digest}, expected {EXE_SHA1}"
    return None


def main():
    problem = exe_problem()
    if problem:
        print(f"{problem}\nonly the link target is available; rerun configure.py once the EXE is in place", file=sys.stderr)
    proj = Project()
    tools = ["tools/cc.py", "tools/lefile.py", "tools/project.py", "tools/split.py", "tools/x86scan.py"]
    configs = ["config/functions.txt", "config/symbols.txt", "config/demo_map.txt", "config/demo_data_map.txt", "config/relocs.txt", "config/cflags.txt", "config/sections.txt"]

    n = []
    n.append("ninja_required_version = 1.10\n")
    n.append("rule configure\n  command = python3 configure.py\n  generator = 1\n")
    if problem:
        n.append(f"rule split\n  command = echo '{problem}' >&2; exit 1\n")
    else:
        n.append("rule split\n  command = python3 tools/split.py build/target\n  restat = 1\n")
    n.append("rule as\n  command = as --32 -o $out $in\n")
    n.append("rule cc\n  command = python3 tools/cc.py $in $out\n")
    n.append(
        "rule report\n  command = objdiff-cli report generate -c functionRelocDiffs=name_address -c combineTextSections=false -o $out -f json-pretty"
        " && python3 tools/progress.py $out && python3 tools/verify.py $out\n"
    )
    # Sources are listed so adding a FUNCTION marker regenerates the unit list.
    sources = Project.sources()
    # No dependency files are generated, so every compiled object
    # depends on every header.
    headers = sorted(
        os.path.relpath(os.path.join(d, f), ROOT)
        for top in ("include", "src")
        for d, _, files in os.walk(os.path.join(ROOT, top))
        for f in files
        if f.endswith((".h", ".hpp"))
    )
    n.append(f"build build.ninja objdiff.json: configure configure.py {' '.join(configs + tools + sources)}\n")

    target_s = [f"build/target/{u.name}.s" for u in proj.units]
    exe_dep = [] if problem else [EXE]
    n.append(f"build {' '.join(target_s)}: split | {' '.join(exe_dep + configs + tools + sources)}\n")

    targets = []
    objects = []
    units = []
    for u in proj.units:
        target = f"build/target/{u.name}.o"
        n.append(f"build {target}: as build/target/{u.name}.s\n")
        targets.append(target)
        unit = {
            "name": u.name,
            "target_path": target,
            "metadata": {
                "auto_generated": u.auto,
                "progress_categories": [u.category],
            },
        }
        if not u.auto:
            base = f"build/base/{u.name}.o"
            deps = ["tools/cc.py", "tools/omf2elf.py", "tools/standin.py", "config/cflags.txt", *headers]
            if os.path.exists(os.path.join(ROOT, standin.companion(u.source))):
                # Stand-ins' references into the unit's own data segments.
                deps += [standin.companion(u.source), "config/sections.txt", "config/symbols.txt", "tools/project.py", "tools/split.py"]
            compiler, flags = compiler_for(u.source)
            name, _, version = compiler.partition("@")
            if name == "lib":
                deps.append(library_path(version, flags[0]))
            n.append(f"build {base}: cc {u.source} | {' '.join(deps)}\n")
            objects.append(base)
            unit["base_path"] = base
            unit["metadata"]["source_path"] = u.source
        units.append(unit)
    # No objdiff unit: nothing to compare them with but the EXE's 16-bit
    # object, which tools/relink.py checks.
    for m in proj.code16:
        n.append(f"build build/base/{m.name}.o: as {m.source}\n")
        objects.append(f"build/base/{m.name}.o")

    # base-objects: the objects compiled from source, all that the links need.
    n.append(f"build base-objects: phony {' '.join(objects)}\n")
    n.append(f"build all: phony base-objects {' '.join(targets)}\n")
    n.append("build build/report.json: report | all objdiff.json tools/progress.py tools/verify.py tools/elfrelocs.py\n")
    n.append("build report: phony build/report.json\n")
    n.append("rule link\n  command = python3 tools/link.py\n")
    n.append("build build/link/report.txt: link | base-objects tools/link.py\n")
    n.append("build link: phony build/link/report.txt\n")
    n.append("default base-objects\n" if problem else "default all\n")

    with open(os.path.join(ROOT, "build.ninja"), "w") as f:
        f.write("\n".join(n))

    objdiff = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "ninja",
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.cpp", "*.c", "*.s", "*.h", "*.hpp"],
        "progress_categories": CATEGORIES,
        "units": units,
    }
    with open(os.path.join(ROOT, "objdiff.json"), "w") as f:
        json.dump(objdiff, f, indent=2)
        f.write("\n")

    with open(os.path.join(ROOT, "compile_commands.json"), "w") as f:
        json.dump(compile_commands(proj), f, indent=2)
        f.write("\n")


# Clang flags approximating Watcom's dialect, for editors (clangd, clang-tidy).
CLANG_FLAGS = [
    "--target=i386-unknown-none",
    "-ffreestanding",
    "-funsigned-char",
    "-fshort-enums",
    "-fms-extensions",
    "-Wno-unknown-pragmas",
    "-Wno-pragma-pack",
    "-Wno-incompatible-function-pointer-types",
]


def compile_commands(proj):
    """compile_commands.json for editors: each C/C++ source compiled by clang
    with the Watcom dialect shimmed (tools/clangd/compat.h). The build itself
    uses Watcom (tools/cc.py)."""
    compat = os.path.join(ROOT, "tools", "clangd", "compat.h")
    include = os.path.join(ROOT, "include")
    seen, entries = set(), []
    for u in proj.units:
        if u.auto or u.source in seen or not u.source.endswith((".c", ".cpp")):
            continue
        seen.add(u.source)
        cxx = u.source.endswith(".cpp")
        args = ["clang++" if cxx else "clang", "-std=gnu++98" if cxx else "-std=gnu89", *CLANG_FLAGS]
        args += ["-include", compat, "-I", include, "-c", u.source]
        entries.append({"directory": ROOT, "file": os.path.join(ROOT, u.source), "arguments": args})
    return entries


if __name__ == "__main__":
    main()

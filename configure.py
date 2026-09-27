#!/usr/bin/env python3
"""Generate build.ninja and objdiff.json from config/.

Run inside the devShell (`nix develop`), then `ninja` to build every object
and `ninja report` for build/report.json.
"""

import hashlib
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "tools"))
from project import EXE, EXE_SHA1, ROOT, Project  # noqa: E402

CATEGORIES = [
    {"id": "game", "name": "Game"},
    {"id": "lib", "name": "Libraries"},
]


def check_exe():
    if not os.path.exists(EXE):
        sys.exit(f"missing {os.path.relpath(EXE, ROOT)}: extract the GOG release into orig/ (see README)")
    digest = hashlib.sha1(open(EXE, "rb").read()).hexdigest()
    if digest != EXE_SHA1:
        sys.exit(f"{os.path.relpath(EXE, ROOT)}: sha1 {digest}, expected {EXE_SHA1}")


def main():
    check_exe()
    proj = Project()
    tools = ["tools/lefile.py", "tools/project.py", "tools/split.py", "tools/x86scan.py"]
    configs = ["config/functions.txt", "config/symbols.txt", "config/units.txt"]

    n = []
    n.append("ninja_required_version = 1.10\n")
    n.append("rule configure\n  command = python3 configure.py\n  generator = 1\n")
    n.append("rule split\n  command = python3 tools/split.py build/target\n  restat = 1\n")
    n.append("rule as\n  command = as --32 -o $out $in\n")
    n.append("rule cc\n  command = python3 tools/cc.py $in $out\n")
    n.append(
        "rule report\n  command = objdiff-cli report generate -o $out -f json-pretty"
        " && python3 tools/progress.py $out\n"
    )
    n.append(f"build build.ninja objdiff.json: configure configure.py {' '.join(configs + tools)}\n")

    target_s = [f"build/target/{u.name}.s" for u in proj.units]
    n.append(f"build {' '.join(target_s)}: split | {EXE} {' '.join(configs + tools)}\n")

    objects = []
    units = []
    for u in proj.units:
        target = f"build/target/{u.name}.o"
        n.append(f"build {target}: as build/target/{u.name}.s\n")
        objects.append(target)
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
            n.append(f"build {base}: cc {u.source} | tools/cc.py tools/fixsyms.py\n")
            objects.append(base)
            unit["base_path"] = base
            unit["metadata"]["source_path"] = u.source
        units.append(unit)

    n.append(f"build all: phony {' '.join(objects)}\n")
    n.append("build build/report.json: report | all objdiff.json tools/progress.py\n")
    n.append("build report: phony build/report.json\n")
    n.append("default all\n")

    with open(os.path.join(ROOT, "build.ninja"), "w") as f:
        f.write("\n".join(n))

    objdiff = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "ninja",
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.cpp", "*.h", "*.hpp"],
        "progress_categories": CATEGORIES,
        "units": units,
    }
    with open(os.path.join(ROOT, "objdiff.json"), "w") as f:
        json.dump(objdiff, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()

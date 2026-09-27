"""Symbols from the Watcom debug info of the March 1997 demo build.

The demo HOSPITAL.EXE (orig/demo-1997-03/) was built from nearly the same
source as the retail game, with optimisation, and ships with debug info.
`wdump -Dx` of it is kept at orig/demo-1997-03/HOSPITAL.debug.txt.
"""

import os
import re
from dataclasses import dataclass

from project import ROOT

DEMO_DIR = os.path.join(ROOT, "orig", "demo-1997-03")
DEMO_EXE = os.path.join(DEMO_DIR, "HOSPITAL.EXE")
DEMO_DUMP = os.path.join(DEMO_DIR, "HOSPITAL.debug.txt")

# Debug-info segment number -> linear base of the LE object it names.
SEGMENT_BASE = {1: 0x10000, 2: 0x100000, 3: 0x110000}


@dataclass
class DemoSymbol:
    name: str
    addr: int
    module: str  # source path as recorded, e.g. "C:\\HOSP\\main.cpp", or a bare library object name
    code: bool
    static: bool

    @property
    def category(self):
        return module_category(self.module)

    @property
    def source(self):
        """Base name of the source file, e.g. "main.cpp"."""
        return self.module.replace("\\", "/").rsplit("/", 1)[-1].lower()


def module_category(path):
    """"game" for C:\\HOSP, "bflib" for Bullfrog's own libraries (graphics,
    sound, network), "runtime" for the Watcom runtime and third-party code."""
    upper = path.upper()
    if upper.startswith("C:\\HOSP\\"):
        return "game"
    if "\\" in path:
        return "bflib"
    return "runtime"


def load(path=DEMO_DUMP):
    """(modules, symbols) from a `wdump -Dx` listing."""
    text = open(path, encoding="latin-1").read()
    module_part = text[text.index("Module Info") : text.index("Global Info")]
    modules = [m.group(2) for m in re.finditer(r"^\s*(\d+)\) Name:\s+(\S+)", module_part, re.M)]
    symbols = []
    global_part = text[text.index("Global Info") :]
    for m in re.finditer(
        r"Name:\s+(\S+)\s+address\s+=\s+([0-9A-F]{4}):([0-9A-F]{8})\s+module index\s+=\s+(\d+)\s+kind:\s+(.*)",
        global_part,
    ):
        name, seg, off, mod, kind = m.groups()
        seg = int(seg, 16)
        if seg not in SEGMENT_BASE:
            continue
        symbols.append(
            DemoSymbol(name, SEGMENT_BASE[seg] + int(off, 16), modules[int(mod)], "(code)" in kind, "static" in kind)
        )
    return modules, symbols

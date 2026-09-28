"""Project model shared by configure.py and split.py.

config/functions.txt       every function: address, size, symbol
config/symbols.txt         named data: address, symbol, size
config/demo_map.txt        demo-build name and source module per function
config/demo_data_map.txt   demo-build name per global
src/**/*.cpp, src/**/*.c   `// FUNCTION: 0xADDRESS` above each function a file implements

Functions not claimed by a source file are grouped into auto units by their
source module in the demo build, or by fixed address window when unknown, so
assigning a function never renames unrelated units.
"""

import bisect
import os
import re
from dataclasses import dataclass, field

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = os.path.join(ROOT, "orig", "HOSPITAL.EXE")
EXE_SHA1 = "d108d34d4e435ce4ad1e9c9b76ab3ad6f000d6ae"

# Fallback category boundary for functions the demo build does not name:
# below it is game code, above it libraries and runtime.
GAME_END = 0xBBFF0
AUTO_WINDOW = 0x4000
FUNCTION_MARKER = re.compile(r"^\s*//\s*FUNCTION:\s*(0x[0-9a-fA-F]+)", re.M)


@dataclass
class Function:
    addr: int
    size: int
    name: str
    demo_name: str | None = None  # mangled name in the demo build
    source: str | None = None  # source file in the demo build, e.g. "main.cpp"
    demo_category: str | None = None  # "game", "bflib" or "runtime"

    @property
    def end(self):
        return self.addr + self.size

    @property
    def category(self):
        if self.demo_category:
            return self.demo_category
        return "game" if self.addr < GAME_END else "runtime"


@dataclass
class DataSymbol:
    addr: int
    name: str
    size: int


@dataclass
class Unit:
    name: str
    functions: list = field(default_factory=list)
    source: str | None = None  # None for auto units

    @property
    def auto(self):
        return self.source is None

    @property
    def category(self):
        return self.functions[0].category


def _lines(path):
    if not os.path.exists(path):
        return
    for line in open(path):
        line = line.split("#", 1)[0].strip()
        if line:
            yield line


class Project:
    def __init__(self, root=ROOT):
        cfg = os.path.join(root, "config")
        self.functions = []
        for line in _lines(os.path.join(cfg, "functions.txt")):
            addr, size, name = line.split()
            self.functions.append(Function(int(addr, 16), int(size, 16), name))
        self.functions.sort(key=lambda f: f.addr)
        self._fn_addrs = [f.addr for f in self.functions]
        self.by_name = {f.name: f for f in self.functions}

        by_addr = {f.addr: f for f in self.functions}
        for line in _lines(os.path.join(cfg, "demo_map.txt")):
            retail, _, _, evidence, category, source, name = line.split()
            fn = by_addr.get(int(retail, 16))
            if fn is not None and evidence != "conflict":
                fn.demo_name, fn.source, fn.demo_category = name, source, category

        self.demo_data = {}  # retail address -> demo global name
        for line in _lines(os.path.join(cfg, "demo_data_map.txt")):
            retail, _, _, _, _, name = line.split()
            self.demo_data[int(retail, 16)] = name

        self.data_symbols = []
        for line in _lines(os.path.join(cfg, "symbols.txt")):
            addr, name, size = line.split()
            self.data_symbols.append(DataSymbol(int(addr, 16), name, int(size, 16)))
        self.data_symbols.sort(key=lambda s: s.addr)
        self._data_addrs = [s.addr for s in self.data_symbols]

        self.units = []
        claimed = {}
        for source in self.sources(root):
            fns = []
            for m in FUNCTION_MARKER.finditer(open(os.path.join(root, source)).read()):
                addr = int(m.group(1), 16)
                if addr not in by_addr:
                    raise KeyError(f"{source}: no function at {addr:#x} in config/functions.txt")
                if addr in claimed:
                    raise KeyError(f"{source}: {addr:#x} already implemented in {claimed[addr]}")
                claimed[addr] = source
                fns.append(by_addr[addr])
            if fns:
                name = os.path.splitext(os.path.relpath(source, "src"))[0]
                self.units.append(Unit(name, sorted(fns, key=lambda f: f.addr), source))

        auto = {}
        for f in self.functions:
            if f.addr in claimed:
                continue
            if f.source:
                stem = os.path.splitext(f.source)[0]
                name = f"auto/{f.category}/{stem}"
            else:
                window = f.addr - f.addr % AUTO_WINDOW
                name = f"auto/{f.category}/unknown_{window:08x}"
            auto.setdefault(name, Unit(name)).functions.append(f)
        self.units += [auto[k] for k in sorted(auto)]

    @staticmethod
    def sources(root=ROOT):
        """Source files under src/, relative to root, in a stable order."""
        found = []
        for dirpath, _, files in os.walk(os.path.join(root, "src")):
            found += [os.path.relpath(os.path.join(dirpath, f), root) for f in files if f.endswith((".cpp", ".c"))]
        return sorted(found)

    def function_at(self, addr):
        """Function whose [addr, end) contains addr, else None."""
        i = bisect.bisect_right(self._fn_addrs, addr) - 1
        if i >= 0 and addr < self.functions[i].end:
            return self.functions[i]
        return None

    def data_symbol_at(self, addr):
        i = bisect.bisect_right(self._data_addrs, addr) - 1
        if i >= 0 and addr < self.data_symbols[i].addr + max(self.data_symbols[i].size, 1):
            return self.data_symbols[i]
        return None

    def data_ref(self, addr):
        """Assembler expression naming a data address."""
        sym = self.data_symbol_at(addr)
        if sym is None:
            return f"_D_{addr:08x}"
        off = addr - sym.addr
        name = f'"{sym.name}"'
        return f"{name}+{off:#x}" if off else name

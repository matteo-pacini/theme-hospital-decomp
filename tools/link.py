#!/usr/bin/env python3
"""Link every unit into a DOS/4G executable and report what does not resolve.

The executable is built from the units alone; nothing is read from the
original EXE. It runs, but does not reproduce the original's layout (see
tools/lewrite.py for that). Every unit is read into tools/omf2elf.py's
object model: compiled units from the compiler's own OMF object (or, with
assembly stand-ins, the one tools/cc.py writes after applying them),
library units from their library module, and assembly and auto units from
their ELF object. Each unit's static functions get a global alias,
`unit::name`; references into the middle of a function (`_C_XXXXXXXX`)
become the function's symbol, or its alias, plus an offset, and ones
outside every function are defined in a gap of zeros of their own. The
16-bit code object's assembly modules (config/sections.txt, segment
CODE16) are read from their ELF objects as 16-bit segments, and a name the
code refers to by an address inside one is defined there.

A name no unit defines but config/symbols.txt (or an address name,
`_D_XXXXXXXX`) places is defined where its address falls among the units'
data, placed at their original addresses as tools/relink.py places them:
inside a unit's section, there; between them, in a generated section of
zeros spanning that gap (data the sources declare `extern` only, mostly
BSS). The startup and exit tables' bounds become the edges of the linked
tables, the stack is a STACK segment of the original's size, and `_end` is
the linker's. Everything is written as OMF and linked in address order
with Open Watcom's wlink; unresolved and duplicate symbols are written to
build/link/report.txt.

Watcom 10.6's own wlink does not run under wibo.

Usage: tools/link.py    (after `ninja`)
"""

import bisect
import collections
import os
import re
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402
from project import ROOT, Project  # noqa: E402
from standin import companion  # noqa: E402

OUT = os.path.join(ROOT, "build", "link")
INTERIOR = re.compile(r"_C_([0-9a-f]{8})")
# The original's stack: its data object ends this far past `_end` rounded up
# to 16 bytes.
STACK_SIZE = 0x4000
TABLE_RECORDS = re.compile(r"^//\s*TABLE RECORDS:\s*(\S+)", re.M)


def unit_model(u):
    """The object model of a unit, from the object tools/link.py links."""
    base = os.path.join(ROOT, "build", "target" if u.auto else "base", u.name)
    if not u.auto:
        with_standins = os.path.exists(os.path.join(ROOT, companion(u.source)))
        omf = base + (".link.obj" if with_standins else ".obj")
        if os.path.exists(omf):
            return omf2elf.convert(open(omf, "rb").read())
    model = omf2elf.read_elf(base + ".o")
    if not u.auto and u.source.endswith(".s"):
        table_records(model, open(os.path.join(ROOT, u.source)).read())
    return model


def table_records(model, source):
    """Record order of an assembly unit (GNU as keeps none): its sections'
    relocations in section order, except in a table that a
    `// TABLE RECORDS: SYMBOL` line in the source names: the original
    assembler emitted each entry of such a table right after the routine
    it points at, so the linker processed an entry's fixup after that
    routine's own fixups."""
    tables = TABLE_RECORDS.findall(source)
    if not tables:
        return
    sections, symbols = model[0], model[1]
    rank = {id(sec): 2 * i for i, sec in enumerate(sections)}
    for sec in sections:
        sec.reloc_records = {at: rank[id(sec)] for at, _, _ in sec.relocs}
    for name in tables:
        table = symbols[name][0]
        for at, _, target in table.relocs:
            if target[0] == "sym" and target[1] in symbols:
                table.reloc_records[at] = rank[id(symbols[target[1]][0])] + 1


def alias(unit, name):
    return f"{unit.name}::{name}"


def data_placements(proj, units, models):
    """The units' data sections at their original addresses, [(address, size,
    section)] in address order, as tools/relink.py places them: a
    config/sections.txt entry places the unit's first non-COMDAT section of
    its segment (with the entry's size), a config/symbols.txt name the
    section defines globally places any other (with its own size)."""
    counts = collections.Counter(s.name for s in proj.data_symbols)
    unique = {s.name: s.addr for s in proj.data_symbols if counts[s.name] == 1}
    placed = []
    for u, (sections, symbols, _, _) in zip(units, models):
        entries = {seg: (a, n) for seg, a, n in proj.unit_data.get(u.source, [])}
        for sec in sections:
            if sec.code:
                continue
            if sec.name in entries and not sec.comdat:
                a, n = entries.pop(sec.name)
            else:
                named = [unique[n] - o for n, (s, o, local) in symbols.items() if s is sec and n in unique and not local]
                if not named:
                    continue
                a, n = named[0], max(len(sec.data), sec.length)
            placed.append((a, n, sec))
    return sorted(placed, key=lambda p: p[0])


def zero_data(placed, names, sizes, end):
    """Sections defining `names` ({name: original address}), which lie
    between the units' data sections `placed` (data_placements): the
    original's data that no unit defines, all zero (tools/lewrite.py
    --strict takes no byte or fixup from the original, so none of it is
    nonzero). Each span between two units' sections that holds a name
    becomes one section, so a name keeps its place relative to its
    neighbours and its 16-byte alignment; `end`, the address of the
    original's `_end`, also ends a span. Names past every unit section and
    `end` share one span over their config/symbols.txt extents (`sizes`).
    Spans from the end of the units' last initialised section on are BSS,
    the others zero-filled DATA. Returns the sections and their symbols."""
    starts = sorted({a for a, _, _ in placed} | ({end} if end is not None else set()))
    ends = sorted({a + n for a, n, _ in placed} | ({end} if end is not None else set()))
    bss = max(a + n for a, n, sec in placed if not sec.nobits)
    spans, tail = collections.defaultdict(dict), {}
    for name, a in names.items():
        k = bisect.bisect_right(starts, a)
        if k == len(starts):
            tail[name] = a
            continue
        lo = ends[bisect.bisect_right(ends, a) - 1] if ends[0] <= a else a
        spans[(lo, starts[k])][name] = a
    if tail:
        spans[(min(tail.values()), max(a + max(sizes.get(n, 0), 1) for n, a in tail.items()))] = tail
    sections, symbols = [], {}
    for (lo, hi), defs in sorted(spans.items()):
        sec = omf2elf.Section("_BSS", "BSS", 16) if lo >= bss else omf2elf.Section("_DATA", "DATA", 16)
        lo &= ~15
        sec.length = hi - lo
        if not sec.nobits:
            sec.write(0, bytes(hi - lo))
        sections.append(sec)
        symbols.update((n, (sec, a - lo, False)) for n, a in defs.items())
    return sections, symbols


def code_gaps(proj, gaps):
    """Sections defining `gaps` ({name: address}), places in the code
    outside every function that the code refers to: each gap between two
    functions of config/functions.txt that holds one, as zeros."""
    sections, symbols, by_gap = [], {}, {}
    for name, a in sorted(gaps.items(), key=lambda g: g[1]):
        lo = max((f.end for f in proj.functions if f.end <= a), default=a)
        if lo not in by_gap:
            hi = min((f.addr for f in proj.functions if f.addr > a), default=a + 4)
            by_gap[lo] = omf2elf.Section("_TEXT", "CODE", 4)
            by_gap[lo].write(0, bytes(hi - lo))
            sections.append(by_gap[lo])
        symbols[name] = (by_gap[lo], a - lo, False)
    return sections, symbols


def main():
    proj = Project()
    shutil.rmtree(OUT, ignore_errors=True)
    os.makedirs(os.path.join(OUT, "objs"))

    # Data-only units have no code to order them by; wlink places their data
    # after the others' in each segment.
    units = sorted(proj.units, key=lambda u: u.functions[0].addr if u.functions else 1 << 32)
    models = [unit_model(u) for u in units]
    unit_of = {f.addr: u for u in units for f in u.functions}
    defined = {n for m in models for n, (_, _, local) in m[1].items() if not local}
    local_names = {id(u): {n for n, (s, _, local) in m[1].items() if local and s.code} for u, m in zip(units, models)}

    def code_name(f):
        u = unit_of.get(f.addr)
        if f.name in defined:
            return f.name
        if u is not None and f.name in local_names[id(u)]:
            return alias(u, f.name)
        return None

    gaps = {}
    for u, (sections, symbols, externs, weak) in zip(units, models):
        for n in local_names[id(u)]:
            sec, off, _ = symbols[n]
            symbols[alias(u, n)] = (sec, off, False)
        for sec in sections:
            for i, (at, rtype, target) in enumerate(sec.relocs):
                m = target[0] == "sym" and target[1] not in symbols and INTERIOR.fullmatch(target[1])
                if not m:
                    continue
                addr = int(m.group(1), 16)
                f = proj.function_at(addr)
                name = code_name(f) if f is not None else None
                if name is None:
                    gaps[target[1]] = addr  # outside every function
                    continue
                addend = struct.unpack_from("<i", sec.data, at)[0] + addr - f.addr
                struct.pack_into("<i", sec.data, at, addend)
                sec.relocs[i] = (at, rtype, ("sym", name))

    by_name = {s.name: s.addr for s in proj.data_symbols}
    # __InitRtns and __FiniRtns walk the startup (XI) and exit (YI) tables
    # between bounds the sources name by the symbols at the original's table
    # edges: its first XI entry, first YI entry and __End_YI. wlink orders
    # the entries differently, so references to these names become the edges of
    # the linked XI and YI segments, defined by `edges` (linked last, so at
    # the end of the empty XIB, YIB and YIE). XI must end where YI starts:
    # __InitRtns stops at the YI bound.
    tables = {seg: [(a, a + n) for segs in proj.unit_data.values() for s, a, n in segs if s == seg]
              for seg in ("XI", "YI")}
    assert max(e for _, e in tables["XI"]) == min(a for a, _ in tables["YI"])
    edge_of = {min(a for a, _ in tables["XI"]): "XIB", min(a for a, _ in tables["YI"]): "YIB",
               max(e for _, e in tables["YI"]): "YIE"}
    edges = ([omf2elf.Section(seg, "DATA", 1) for seg in edge_of.values()], {}, [None], {})
    for sec in edges[0]:
        edges[1][f"link::{sec.name}"] = (sec, 0, False)
    for sections, symbols, externs, weak in models:
        for sec in sections:
            if sec.name == "_NULL":
                sec.cls = "BEGDATA"  # cstrt386.s: the first bytes of DGROUP
            for i, (at, rtype, target) in enumerate(sec.relocs):
                if target[0] == "sym" and target[1] not in symbols and by_name.get(target[1]) in edge_of:
                    sec.relocs[i] = (at, rtype, ("sym", f"link::{edge_of[by_name[target[1]]]}"))

    needed = set()
    for sections, symbols, externs, weak in models:
        for sec in sections:
            needed.update(t[1] for _, _, t in sec.relocs if t[0] == "sym" and t[1] not in symbols)
    resolved = {}
    for n in sorted(needed - defined):
        if n in by_name:
            resolved[n] = by_name[n]
        elif re.fullmatch(r"_D_[0-9a-f]{8}", n):
            resolved[n] = int(n[3:], 16)
    # The 16-bit code object's modules. A name the code refers to by its
    # address there (the C knows the handlers only as bounds to copy) is an
    # alias of the place in the module that covers it, or ends there.
    code16 = []
    for m in proj.code16:
        sections, symbols, externs, weak = omf2elf.read_elf(os.path.join(ROOT, "build", "base", m.name + ".o"))
        sec = next(s for s in sections if s.name == "CODE16")  # not the assembler's empty .text
        sec.use16 = True
        code16.append((m, ([sec], symbols, externs, weak)))
        for n, a in list(resolved.items()):
            if m.addr <= a < m.end or a == m.end and not any(o.addr == a for o in proj.code16):
                symbols[n] = (sec, a - m.addr, False)
                del resolved[n]
    # Every other name is a unit's data at its original address, or, between
    # the units' data, data no unit defines. `_end` is the linker's.
    resolved.pop("_end", None)
    placed = data_placements(proj, units, models)
    in_units = 0
    for n, a in list(resolved.items()):
        hit = next(((b, sec) for b, size, sec in placed if b <= a < b + size), None)
        if hit is not None:
            symbols = next(m[1] for m in models if any(s is hit[1] for s in m[0]))
            symbols[n] = (hit[1], a - hit[0], False)
            in_units += 1
            del resolved[n]
    zero, zero_symbols = zero_data(placed, resolved, {s.name: s.size for s in proj.data_symbols}, by_name.get("_end"))
    bss = ([s for s in zero if s.nobits], {n: d for n, d in zero_symbols.items() if d[0].nobits}, [None], {})
    gap_sections, gap_symbols = code_gaps(proj, gaps)
    edges[0].extend([s for s in zero if not s.nobits] + gap_sections)
    edges[1].update({n: d for n, d in zero_symbols.items() if not d[0].nobits})
    edges[1].update(gap_symbols)
    stack = omf2elf.Section("STACK", "STACK", 16)
    stack.length = STACK_SIZE
    edges[0].append(stack)

    files = []
    # The zero BSS first: cstrt386 clears BSS from W?wasteOfTime$nul, the
    # original's first BSS variable, which no unit defines.
    for k, model in enumerate([bss] + models + [model for _, model in code16] + [edges]):
        dst = os.path.join("objs", f"{k:03d}.obj")
        omf2elf.write_omf(*model, os.path.join(OUT, dst))
        files.append(dst)
    with open(os.path.join(OUT, "hosp.lnk"), "w") as f:
        f.write("system dos4g\noption nodefaultlibs\noption undefsok\noption start=_cstart_\n"
                "option map=hosp.map\nname hosp.exe\n"
                # The original's order: the null area first in DGROUP, the
                # 16-bit code object between the code and data objects.
                "order clname CODE clname BEGDATA clname DATA clname BSS clname STACK\n")
        f.writelines(f"file {p}\n" for p in files)
    env = dict(os.environ, WATCOM=os.path.dirname(os.path.dirname(shutil.which("wlink"))))
    r = subprocess.run(["wlink", "@hosp.lnk"], cwd=OUT, env=env, capture_output=True, text=True)
    open(os.path.join(OUT, "link.out"), "w").write(r.stdout + r.stderr)

    unresolved = sorted({m.group(1) for m in re.finditer(r"undefined symbol (.+)", r.stdout)})
    duplicates = sorted({m.group(1) for m in re.finditer(r"redefinition of (\S+)", r.stdout)})
    with open(os.path.join(OUT, "report.txt"), "w") as f:
        f.write(f"objects {len(files)} (auto units {sum(u.auto for u in units)})\n")
        f.write(f"code references outside every function {len(gaps)}\n")
        f.write(f"names placed by address in a unit's data {in_units}\n")
        f.write(f"names in data no unit defines {len(zero_symbols)}"
                f" ({len(zero)} sections, {sum(s.length for s in zero):#x} bytes)\n")
        f.write(f"unresolved {len(unresolved)}\n" + "".join(f"  {n}\n" for n in unresolved))
        f.write(f"duplicate {len(duplicates)}\n" + "".join(f"  {n}\n" for n in duplicates))
    print(open(os.path.join(OUT, "report.txt")).read(), end="")


if __name__ == "__main__":
    main()
